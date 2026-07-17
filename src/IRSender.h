#ifndef IRSENDER_H
#define IRSENDER_H

#include <Arduino.h>
#include <pwm.h>

class IRSender {
private:
    PwmOut _pwm;
    uint16_t _currentFrequency;
    uint16_t _activePulseWidth;

    void setFrequency(uint16_t frequency);

public:
    IRSender();

    /**
     * @brief Initializes the IR transmitter hardware (timer GPT).
     */
    void begin();

    /**
     * @brief Generates a carrier mark (active carrier) for a duration in microseconds.
     * @param time_us Duration in microseconds.
     */
    void mark(uint16_t time_us);

    /**
     * @brief Generates a carrier space (idle, carrier off) for a duration in microseconds.
     * @param time_us Duration in microseconds.
     */
    void space(uint16_t time_us);

    /**
     * @brief Replays raw timing array of marks and spaces.
     * @param rawBuf Pointer to raw timing array in microseconds.
     * @param length Number of pulses.
     * @param frequency Carrier frequency in Hz.
     */
    void sendRaw(const uint16_t* rawBuf, uint16_t length, uint16_t frequency = 38000);

    /**
     * @brief Sends an NEC protocol command.
     * @param address 16-bit address.
     * @param command 32-bit command.
     * @param bits Bit length (default 32).
     */
    void sendNEC(uint16_t address, uint32_t command, uint8_t bits = 32);

    /**
     * @brief Sends a Samsung protocol command.
     * @param address 16-bit address.
     * @param command 32-bit command.
     * @param bits Bit length (default 32).
     */
    void sendSamsung(uint16_t address, uint32_t command, uint8_t bits = 32);

    /**
     * @brief Sends a Sony protocol command.
     * @param address 16-bit address.
     * @param command 32-bit command.
     * @param bits Bit length (default 12/15/20).
     */
    void sendSony(uint16_t address, uint32_t command, uint8_t bits = 12);

    /**
     * @brief Sends an RC5 protocol command.
     * @param address 5-bit address.
     * @param command 6-bit command.
     * @param bits Bit length (default 14).
     */
    void sendRC5(uint16_t address, uint32_t command, uint8_t bits = 14);

    /**
     * @brief Sends an RC6 protocol command.
     * @param address 8-bit address.
     * @param command 8-bit command.
     * @param bits Bit length (default 20).
     */
    void sendRC6(uint16_t address, uint32_t command, uint8_t bits = 20);

    /**
     * @brief Sends a Panasonic protocol command.
     * @param address 16-bit address.
     * @param command 32-bit command.
     * @param bits Bit length (default 48).
     */
    void sendPanasonic(uint16_t address, uint32_t command, uint8_t bits = 48);

    /**
     * @brief Sends an LG protocol command.
     * @param address 8-bit address.
     * @param command 16-bit command.
     * @param bits Bit length (default 28).
     */
    void sendLG(uint16_t address, uint32_t command, uint8_t bits = 28);

    /**
     * @brief Sends a JVC protocol command.
     * @param address 16-bit address.
     * @param command 16-bit command.
     * @param bits Bit length (default 16).
     */
    void sendJVC(uint16_t address, uint32_t command, uint8_t bits = 16);
};

#endif // IRSENDER_H
