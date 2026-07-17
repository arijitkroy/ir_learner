#include "IRSender.h"

IRSender::IRSender() : _pwm(D3), _currentFrequency(38000), _activePulseWidth(8) {}

void IRSender::begin() {
    _pwm.begin(38000.0f, 33.3f); // Initialize pin muxing and timer channel
    _pwm.pulseWidth_us(0);        // Force idle state (constant LOW)
}

void IRSender::setFrequency(uint16_t frequency) {
    if (_currentFrequency != frequency) {
        _pwm.begin((float)frequency, 33.3f);
        _pwm.pulseWidth_us(0);
        _currentFrequency = frequency;
    }
    // Calculate pulse width for 33.3% duty cycle (Period = 1,000,000 / frequency)
    _activePulseWidth = 1000000 / (uint32_t)frequency / 3;
    if (_activePulseWidth == 0) _activePulseWidth = 1;
}

void IRSender::mark(uint16_t time_us) {
    uint32_t start = micros();
    _pwm.pulseWidth_us(_activePulseWidth); // Enable carrier instantly (low overhead integer call)
    
    int32_t remaining = (int32_t)time_us - (int32_t)(micros() - start);
    if (remaining > 0) {
        delayMicroseconds(remaining);
    }
}

void IRSender::space(uint16_t time_us) {
    uint32_t start = micros();
    _pwm.pulseWidth_us(0); // Disable carrier instantly (forces pin LOW)
    
    int32_t remaining = (int32_t)time_us - (int32_t)(micros() - start);
    if (remaining > 0) {
        delayMicroseconds(remaining);
    }
}

void IRSender::sendRaw(const uint16_t* rawBuf, uint16_t length, uint16_t frequency) {
    setFrequency(frequency);
    for (uint16_t i = 0; i < length; i++) {
        if (i % 2 == 0) {
            mark(rawBuf[i]);
        } else {
            space(rawBuf[i]);
        }
    }
    space(0); // Ensure off
}

void IRSender::sendNEC(uint16_t address, uint32_t command, uint8_t bits) {
    setFrequency(38000);
    uint32_t data;
    
    // If bits is 32, combine address (lower 16) and command (upper 16)
    if (bits == 32) {
        data = ((uint32_t)command << 16) | address;
    } else {
        uint8_t addr = address & 0xFF;
        uint8_t cmd = command & 0xFF;
        // Standard NEC: Addr, ~Addr, Cmd, ~Cmd
        data = ((uint32_t)(~cmd & 0xFF) << 24) | 
               ((uint32_t)cmd << 16) | 
               ((uint32_t)(~addr & 0xFF) << 8) | 
               addr;
        bits = 32;
    }

    // Header
    mark(9000);
    space(4500);

    // Data (LSB first)
    for (int i = 0; i < bits; i++) {
        if (data & (1UL << i)) {
            mark(560);
            space(1690);
        } else {
            mark(560);
            space(560);
        }
    }
    
    // Stop bit
    mark(560);
    space(0);
}

void IRSender::sendSamsung(uint16_t address, uint32_t command, uint8_t bits) {
    setFrequency(38000);
    uint32_t data;

    if (bits == 32) {
        data = ((uint32_t)command << 16) | address;
    } else {
        uint8_t addr = address & 0xFF;
        uint8_t cmd = command & 0xFF;
        // Samsung protocol: Addr, Addr, Cmd, ~Cmd
        data = ((uint32_t)(~cmd & 0xFF) << 24) | 
               ((uint32_t)cmd << 16) | 
               ((uint32_t)addr << 8) | 
               addr;
        bits = 32;
    }

    // Header
    mark(4500);
    space(4500);

    // Data (LSB first)
    for (int i = 0; i < bits; i++) {
        if (data & (1UL << i)) {
            mark(560);
            space(1690);
        } else {
            mark(560);
            space(560);
        }
    }

    // Stop bit
    mark(560);
    space(0);
}

void IRSender::sendSony(uint16_t address, uint32_t command, uint8_t bits) {
    setFrequency(40000);
    
    uint32_t data = 0;
    if (bits == 12) {
        data = ((address & 0x1F) << 7) | (command & 0x7F);
    } else if (bits == 15) {
        data = ((address & 0xFF) << 7) | (command & 0x7F);
    } else if (bits == 20) {
        data = ((address & 0x1FFF) << 7) | (command & 0x7F);
    } else {
        data = command;
    }

    // Sony requires at least 3 repeats
    for (int r = 0; r < 3; r++) {
        // Header
        mark(2400);
        space(600);

        // Data (LSB first)
        for (int i = 0; i < bits; i++) {
            if (data & (1UL << i)) {
                mark(1200);
                space(600);
            } else {
                mark(600);
                space(600);
            }
        }
        
        if (r < 2) {
            space(45000); // 45ms gap between repeats
        }
    }
    space(0);
}

void IRSender::sendRC5(uint16_t address, uint32_t command, uint8_t bits) {
    setFrequency(36000);

    // Construct 14-bit RC5 word: 2 Start bits ('11'), 1 Toggle ('0'), 5 Address, 6 Command
    static bool toggle = false;
    uint32_t data = (1UL << 13) | (1UL << 12) | ((toggle ? 1UL : 0UL) << 11) | 
                    ((address & 0x1F) << 6) | (command & 0x3F);
    toggle = !toggle; // Flip toggle bit for next time

    // RC5 uses Manchester coding (MSB first)
    // Bit period = 1778 us. Half bit = 889 us.
    // '1': space 889, mark 889
    // '0': mark 889, space 889
    for (int i = bits - 1; i >= 0; i--) {
        bool bit = (data >> i) & 1;
        if (bit) {
            space(889);
            mark(889);
        } else {
            mark(889);
            space(889);
        }
    }
    space(0);
}

void IRSender::sendRC6(uint16_t address, uint32_t command, uint8_t bits) {
    setFrequency(36000);

    // Standard RC6 mode 0: 20 bits (Start '1', Mode '000', Toggle '0', Address '8-bit', Command '8-bit')
    static bool toggle = false;
    uint32_t data = ((uint32_t)(toggle ? 1 : 0) << 16) | ((uint32_t)(address & 0xFF) << 8) | (command & 0xFF);
    toggle = !toggle;

    // Header
    mark(2666);
    space(889);

    // Start bit '1' (Manchester: mark 444, space 444)
    mark(444);
    space(444);

    // Mode bits '000' (Manchester: space 444, mark 444)
    for (int i = 0; i < 3; i++) {
        space(444);
        mark(444);
    }

    // Toggle bit (Double duration: 889us mark, 889us space or vice-versa)
    bool tBit = (data >> 16) & 1;
    if (tBit) {
        mark(889);
        space(889);
    } else {
        space(889);
        mark(889);
    }

    // Data bits (MSB first)
    for (int i = 15; i >= 0; i--) {
        bool bit = (data >> i) & 1;
        if (bit) {
            // '1': mark 444, space 444
            mark(444);
            space(444);
        } else {
            // '0': space 444, mark 444
            space(444);
            mark(444);
        }
    }
    space(0);
}

void IRSender::sendPanasonic(uint16_t address, uint32_t command, uint8_t bits) {
    setFrequency(37000);

    // Panasonic is 48 bits total.
    // LSB first
    // Address (16 bits), Command (32 bits)
    uint64_t data = ((uint64_t)command << 16) | address;

    // Header
    mark(3456);
    space(1728);

    // Data (LSB first)
    for (int i = 0; i < bits; i++) {
        if (data & (1ULL << i)) {
            mark(864);
            space(2592);
        } else {
            mark(864);
            space(864);
        }
    }

    // Stop bit
    mark(864);
    space(0);
}

void IRSender::sendLG(uint16_t address, uint32_t command, uint8_t bits) {
    setFrequency(38000);

    // LG protocol: 28 bits total (MSB first)
    // Combine address (upper 12 bits) and command (lower 16 bits)
    uint32_t data = ((uint32_t)address << 16) | (command & 0xFFFF);
    bits = 28;

    // Header
    mark(9000);
    space(4500);

    // Data (MSB first)
    for (int i = bits - 1; i >= 0; i--) {
        if (data & (1UL << i)) {
            mark(560);
            space(1600);
        } else {
            mark(560);
            space(560);
        }
    }

    // Stop bit
    mark(560);
    space(0);
}

void IRSender::sendJVC(uint16_t address, uint32_t command, uint8_t bits) {
    setFrequency(38000);

    // JVC protocol: 16 bits total.
    // LSB first
    uint32_t data = command;

    // Header (JVC only sends header on first frame, but for simplicity we send it here)
    mark(8400);
    space(4200);

    // Data (LSB first)
    for (int i = 0; i < bits; i++) {
        if (data & (1UL << i)) {
            mark(526);
            space(1574);
        } else {
            mark(526);
            space(526);
        }
    }

    // Stop bit
    mark(526);
    space(0);
}
