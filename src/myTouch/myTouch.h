#ifndef MY_TOUCH_H
#define MY_TOUCH_H

#include <Arduino.h>

// Header nativo IDF
extern "C"
{
#include "driver/touch_pad.h"
}

/**
 * @class myTouch
 * @brief ESP32 native touch sensor wrapper with hardware filtering and calibration
 * 
 * This class provides a simplified interface to the ESP32's capacitive touch sensing
 * capabilities using the native IDF driver. It features:
 * - Hardware IIR filtering for noise-free readings
 * - Automatic calibration to determine resting baselines
 * - Optimized voltage configuration for maximum signal-to-noise ratio
 * - Reference channel configuration for stable operation
 * 
 * Hardware Requirements:
 * - 10nF capacitor connected between GPIO 4 (TOUCH_PAD_NUM0) and GND for reference
 * - Touch electrode connected to selected GPIO pin
 * 
 * @author Your Name
 * @version 1.0
 */
class myTouch
{
public:
    /**
     * @brief Constructor
     * 
     * Creates a new myTouch instance. Actual initialization is deferred to begin().
     */
    myTouch();

    /**
     * @brief Initializes the touch sensor system for the specified GPIO pin
     * 
     * Configures:
     * - IDF touch driver
     * - Voltage levels: High=2.7V, Low=0.5V, Attenuation=1V
     * - Hardware IIR filter with 10ms sampling period
     * - Reference channel (TOUCH_PAD_NUM0 / GPIO 4)
     * 
     * @param pin The touch pad to initialize (e.g., TOUCH_PAD_NUM4 for GPIO 13)
     * @return true if initialization successful, false if failed
     * 
     * @example
     * @code
     * myTouch touch;
     * if (touch.begin(TOUCH_PAD_NUM4)) {
     *     Serial.println("Touch ready!");
     * }
     * @endcode
     * 
     * @note Call this once during setup(). The system will fail if called multiple times
     *       without proper cleanup.
     * 
     * @see TOUCH_PAD_NUM0, TOUCH_PAD_NUM2, TOUCH_PAD_NUM4, TOUCH_PAD_NUM5
     */
    bool begin(touch_pad_t pin);

    /**
     * @brief Reads the current hardware-filtered touch sensor value
     * 
     * Returns the IIR-filtered ADC reading with a 10ms update rate.
     * Lower values indicate a touched state.
     * 
     * @return uint16_t Filtered ADC reading (typically 0-4095 range)
     *         - Untouched: ~2000-3500 (depends on PCB layout)
     *         - Touched:   ~500-1500
     * 
     * @example
     * @code
     * uint16_t value = touch.leggiFiltrato();
     * if (value < threshold) {
     *     Serial.println("Sensor touched!");
     * }
     * @endcode
     * 
     * @note This function should be called regularly (every 10-100ms) for responsive
     *       behavior. The value updates at 100Hz internally.
     */
    uint16_t leggiFiltrato();

    /**
     * @brief Automatically calibrates the touch sensor by averaging resting readings
     * 
     * This function should be called during setup() without the sensor being touched.
     * It averages multiple readings to determine the baseline (resting) value.
     * 
     * Typical calibration process:
     * 1. Call this function without touching the sensor
     * 2. Use returned value to calculate activation threshold
     * 3. Example: threshold = baseline * 0.8
     * 
     * @param campioni Number of samples to average. Default: 10
     *        - 10:  Fast calibration (100ms) - use for stable environments
     *        - 20:  Medium calibration (200ms) - recommended
     *        - 30+: Slow calibration (300ms+) - for very noisy environments
     * 
     * @return uint16_t Average resting value to use as baseline
     * 
     * @example
     * @code
     * Serial.println("Calibrating... don't touch!");
     * uint16_t baseline = touch.calibra(20);
     * uint16_t threshold = baseline - (baseline * 0.2);  // 20% drop = detection
     * @endcode
     * 
     * @warning The sensor MUST NOT be touched during calibration for accurate results.
     *          Ensure proper timing between begin() and calibra() calls.
     * 
     * @note Each calibration sample takes ~20ms. Adjust campioni parameter accordingly.
     */
    uint16_t calibra(uint8_t campioni = 10);

private:
    touch_pad_t _pin;  ///< The configured touch pad pin
};

#endif