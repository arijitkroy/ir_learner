#ifndef IRRECEIVER_H
#define IRRECEIVER_H

#include <Arduino.h>

#define RX_BUFFER_SIZE 400

struct DecodeResult {
    bool decoded;
    uint8_t protocol;    // 0=Raw/Unknown, 1=NEC, 2=Sony, 3=Samsung, 4=RC5, 5=RC6, 6=Panasonic, 7=LG, 8=JVC
    uint16_t address;
    uint32_t command;
    uint8_t bits;
    uint8_t quality;     // Signal quality (0 to 100)
};

class IRReceiver {
public:
    enum State {
        STATE_IDLE,
        STATE_LEARNING,
        STATE_CAPTURING,
        STATE_FINISHED,
        STATE_OVERFLOW,
        STATE_TIMEOUT
    };

private:
    volatile State _state;
    volatile uint32_t _lastEdgeTime;
    volatile uint16_t _rawBuffer[RX_BUFFER_SIZE];
    volatile uint16_t _rawCount;
    uint32_t _learnStartTime;
    
    // Decoders
    bool decodeNEC(DecodeResult& result);
    bool decodeSamsung(DecodeResult& result);
    bool decodeSony(DecodeResult& result);
    bool decodeRC5(DecodeResult& result);
    bool decodeRC6(DecodeResult& result);
    bool decodePanasonic(DecodeResult& result);
    bool decodeLG(DecodeResult& result);
    bool decodeJVC(DecodeResult& result);

public:
    IRReceiver();

    /**
     * @brief Initializes the IR receiver pin.
     */
    void begin();

    /**
     * @brief Starts the learning mode, setting state to STATE_LEARNING.
     */
    void startLearning();

    /**
     * @brief Stops the learning mode.
     */
    void stopLearning();

    /**
     * @brief Runs background loop tasks (checks timeouts and handles state transitions).
     */
    void loop();

    /**
     * @brief Resets the capture buffers and returns to idle/learning.
     */
    void reset();

    /**
     * @brief Retrieves the current state of the receiver.
     * @return State enum.
     */
    State getState() const { return _state; }

    /**
     * @brief Gets the captured raw pulse count.
     * @return Number of pulses.
     */
    uint16_t getRawCount() const { return _rawCount; }

    /**
     * @brief Gets the total number of hardware interrupt triggers.
     * @return Count of interrupts.
     */
    uint32_t getInterruptCount() const;

    /**
     * @brief Copies the captured raw timing values.
     * @param dest Buffer to copy into.
     * @param maxLen Maximum number of elements to copy.
     * @return Number of elements copied.
     */
    uint16_t getRawTimings(uint16_t* dest, uint16_t maxLen) const;

    /**
     * @brief Decodes the captured buffer.
     * @param result Reference to store the decoded result.
     * @return True if decoded, false if classified as raw/unknown.
     */
    bool decode(DecodeResult& result);

    /**
     * @brief Internal ISR callback handler.
     */
    void handleInterrupt();
};

#endif // IRRECEIVER_H
