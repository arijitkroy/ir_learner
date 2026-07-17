# Arduino Uno R4 WiFi IR Learning Remote Server

An all-in-one, production-grade infrared learning, database, and transmitting platform for the **Arduino Uno R4 WiFi** (Renesas RA4M1 + ESP32-S3). Features a sleek, responsive Single Page Application (SPA) dashboard hosted directly from internal flash, featuring real-time diagnostics, WebSocket streaming logs, and raw signal analysis.

---

## Architecture Overview

```
                      +-----------------------------+
                      |       Web Browser UI        |
                      +--------------+--------------+
                                     |
                             HTTP / WebSockets
                                     |
                      +--------------v--------------+
                      |       ESP32-S3 WiFi         |  (Coprocessor running
                      +--------------+--------------+   network bridge)
                                     |  SPI
                      +--------------v--------------+
                      |        Renesas RA4M1        |  (Main MCU)
                      +--+--------+--------+-----+--+
                         |        |        |     |
                   GPT (D3)   IRQ1 (D2)  EEPROM  USB
                         |        |        |     |
                      +--v--+  +--v--+   +--v-+  v
                      | LED |  | OUT |   | 8K | Serial
                      +-----+  +-----+   +----+
                       (TX)     (RX)
```

### Key Modular Components
* **`WiFiManager`**: Dynamically transitions between Station (STA) mode connecting to home routers and local Access Point (AP) Configuration Portal mode if setup fails or credentials are unset. Integrated mDNS responder (`irremote.local`).
* **`IRReceiver`**: Asynchronous interrupt-driven signal edge-timing capture on pin D2 (IRQ1). Custom parses timings to identify protocols (NEC, Samsung, Sony, Panasonic, LG, JVC, RC5, RC6) or marks as RAW with a custom signal quality metric.
* **`IRSender`**: Direct hardware modulation of the RA4M1 General PWM Timer (GPT) channel on pin D3 to emit an accurate 38 kHz carrier frequency at a ~33% duty cycle, minimizing CPU load and timing jitter.
* **`Storage`**: Adapts to the 8KB Data Flash limit by maintaining a compact metadata index in RAM (**3.6 KB SRAM for 100 codes**) and storing variable-length records dynamically in flash, using run-length value compression on raw timings for a ~50% size reduction.
* **`Web` / `API`**: Custom-written HTTP server and RFC 6455-compliant WebSocket server running on port 80, routing REST API commands and broadcasting diagnostics/real-time pulse streams in JSON format.

---

## Hardware Assembly & Pinout

### Receiver (HX1838)
* **OUT** -> **D2** (External Interrupt IRQ1)
* **VCC** -> **5V**
* **GND** -> **GND**

### Emitter (940 nm IR LED with transistor driver)
Use a transistor driver for the IR LED. Driving an IR LED directly from D3 limits it to approximately 20–30 mA, which may be detected by a nearby IR receiver but is often too weak for an appliance. A physical remote normally uses a higher **pulsed** LED current.

Required parts:

* 1 × 940 nm IR LED
* 1 × NPN transistor: **2N2222**, PN2222, S8050, or BC337
* 1 × 1 kΩ resistor (base resistor)
* 1 × 10 kΩ resistor (base pull-down)
* 1 × 39–47 Ω, ½ W resistor (IR LED current limiting)

```
Arduino D3 ----[ 1 kΩ ]---- Base  NPN transistor
                              |
                           [ 10 kΩ ]
                              |
Arduino GND ------------------+---- Emitter

Arduino 5V ----[ 39–47 Ω ]---->|---- Collector
                             IR LED
```

The D3 PWM output is unchanged: it switches the transistor at the required carrier frequency. The transistor supplies approximately 75–100 mA peak LED current while D3 supplies only a few milliamps to the transistor base.

**Important:** Connect the Arduino GND to the transistor emitter and LED-supply ground. Do not connect the IR LED/module signal pin directly to D3 at the same time as this driver. Do not use a relay; it cannot switch at the 38 kHz carrier frequency.

<!-- Legacy direct-drive guidance retained in source history only.
*Note: A 100Ω resistor limits the current from the 5V/3.3V logic line to a safe level (~20-30mA) while still enabling an effective transmission range of 1.5 to 2 meters.*

-->

---

## Compiling & Uploading

### Prerequisites & Board Installation
1. Download and install the latest **Arduino IDE** (version 2.x or later).
2. Go to **Tools > Board > Boards Manager**, search for `Arduino UNO R4 Boards` and install the board support package. (No external libraries are required, as mDNS is fully self-contained in the sketch files).

### Compilation Steps
1. Open `IR_Learner.ino` in the Arduino IDE.
2. Select your board: **Tools > Board > Arduino UNO R4 Boards > Arduino UNO R4 WiFi**.
3. Select your port: **Tools > Port > [Select your board's COM port]**.
4. Verify compiling by clicking the Checkmark button.
5. Upload by clicking the Right Arrow button.

---

## First-Time WiFi Setup (Configuration AP)

1. When the device boots for the first time (or after a factory reset), it will detect that no WiFi credentials exist.
2. It automatically fires up a local Wi-Fi Access Point named **`IR_Learner_XXXX`** (where `XXXX` are the last 4 characters of its MAC address).
3. Connect your computer or phone to this network (no password required).
4. Open your browser and navigate to **`http://192.168.4.1`**.
5. Go to the **Settings** tab. Input your router's SSID and Password, configure an optional mDNS hostname (e.g. `irremote`), and press **Save & Apply**.
6. The controller will save the settings to the Data Flash, shut down the AP, and reboot.
7. Once connected to your network, you can access the interface on your local router network via **`http://irremote.local`** (or the IP address assigned by DHCP, which will print to the Arduino IDE Serial Monitor at 115200 baud).

---

## REST API Documentation

All requests and responses use the `application/json` content-type.

### 1. Device Diagnostics
* **Endpoint:** `GET /api/status`
* **Response:**
  ```json
  {
    "rssi": -55,
    "ip": "192.168.1.104",
    "freeRam": 18450,
    "flashUsage": 135400,
    "uptime": 240,
    "state": "IDLE",
    "wifiSSID": "MyHomeRouter",
    "wifiAP": false
  }
  ```

### 2. Get Stored Commands List
* **Endpoint:** `GET /api/codes`
* **Response:**
  ```json
  [
    {
      "id": 1,
      "name": "TV Power",
      "protocol": 1,
      "address": 16128,
      "command": 61234,
      "bits": 32,
      "frequency": 38000,
      "rawLength": 0
    }
  ]
  ```

### 3. Start IR Signal Learning
* **Endpoint:** `POST /api/learn/start`
* **Response:** `{"status":"ok","msg":"Learning started"}`

### 4. Stop IR Signal Learning
* **Endpoint:** `POST /api/learn/stop`
* **Response:** `{"status":"ok","msg":"Learning stopped"}`

### 5. Check Capture Status
* **Endpoint:** `GET /api/learn/status`
* **Response:**
  ```json
  {
    "state": "FINISHED",
    "decoded": true,
    "protocol": 1,
    "address": 16128,
    "command": 61234,
    "bits": 32,
    "pulses": 67,
    "quality": 95,
    "raw": []
  }
  ```

### 6. Transmit Signal
* **Endpoint:** `POST /api/send`
* **Payload (Send by ID):** `{"id": 1}`
* **Payload (Direct raw emit):**
  ```json
  {
    "protocol": 0,
    "address": 0,
    "command": 0,
    "bits": 0,
    "frequency": 38000,
    "rawLength": 4,
    "raw": [9000, 4500, 560, 560]
  }
  ```
* **Response:** `{"status":"ok","msg":"Signal transmitted successfully"}`

### 7. Delete Command
* **Endpoint:** `POST /api/delete`
* **Payload:** `{"id": 1}`
* **Response:** `{"status":"ok","msg":"Command deleted successfully"}`

### 8. Rename Command
* **Endpoint:** `POST /api/rename`
* **Payload:** `{"id": 1, "name": "Living Room TV"}`
* **Response:** `{"status":"ok","msg":"Command renamed successfully"}`

### 9. Export Database Backup
* **Endpoint:** `POST /api/export`
* **Response:** `{"status":"ok","database":[...Array of stored codes...]}`

### 10. Import Database Backup
* **Endpoint:** `POST /api/import`
* **Payload:** JSON Array containing code objects.
* **Response:** `{"status":"ok","msg":"Imported 5 commands successfully","count":5}`

---

## Troubleshooting & FAQ

### Device fails to boot or loops after configuration
Verify that you are using a stable 5V power supply. Toggling the WiFi stack and emitting signals simultaneously can cause power dips on standard USB ports. Use an external 9V/12V DC barrel jack adaptor if needed.

### Port is not visible or mDNS fails to resolve
mDNS resolution requires Bonjour support (native on Windows 10/11, macOS, and iOS). If you are on an older OS, make sure you can reach the device via its IP address printed in the Serial Monitor. Ensure that client isolation is disabled in your router settings.

---

## Future Improvements
* Add support for 40 kHz (Sony native) and 36 kHz (RC5 native) subcarrier filters on physical receiver shields.
* Implement custom IR protocol macros (combining multiple TV and Receiver signals into one key click).
* Support dynamic WebSocket terminal inputs to run debugging shells at runtime.
