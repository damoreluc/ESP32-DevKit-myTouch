#include "myTouch.h"
//#include "driver/touch_pad.h" // Header nativo IDF

/**
 * Constructor: Initialize the touch pad to an invalid state.
 * The actual configuration happens in begin().
 */
myTouch::myTouch() : _pin(TOUCH_PAD_MAX) {}

/**
 * Initialize the ESP32 touch sensor system with optimized settings for stable operation.
 * 
 * Configuration steps:
 * 1. Initialize IDF touch driver
 * 2. Set optimized voltage levels for maximum signal clarity
 * 3. Configure the selected pin for touch sensing
 * 4. Configure reference channel (GPIO 4) for noise suppression
 * 5. Enable hardware IIR filter with 10ms update rate
 * 
 * @param pin The touch pad to initialize
 * @return true if successful, false otherwise
 */
bool myTouch::begin(touch_pad_t pin) {
    _pin = pin;

    // 1. Initialize IDF touch driver
    // This must be called first to set up the Touch hardware
    esp_err_t err = touch_pad_init();
    if (err != ESP_OK) return false;

    // 2. Configure voltage levels for optimal performance
    // TOUCH_HVOLT_2V7: High voltage = 2.7V (charges electrode faster, stronger signal)
    // TOUCH_LVOLT_0V5: Low voltage = 0.5V (lower baseline, cleaner signal)
    // TOUCH_HVOLT_ATTEN_1V: Attenuation = 1V (reduces overvoltage stress)
    // This configuration maximizes the signal-to-noise ratio
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    
    // 3. Configure the selected pin for touch sensing
    // Second parameter (0) is the threshold - we handle detection logic in software
    touch_pad_config(_pin, 0);

    // 4. Configure reference channel (GPIO 4 / TOUCH_PAD_NUM0)
    // Even though we don't read from it, configuring it as reference provides:
    // - Stable baseline for hardware comparison
    // - Better noise immunity through differential measurement
    // - Hardware requirement for reliable operation
    // The 10nF capacitor on this pin is essential for stability
    touch_pad_config(TOUCH_PAD_NUM0, 0); 

    // 5. Enable hardware IIR (Infinite Impulse Response) filter
    // Period of 10ms means the filter updates 100 times per second
    // IIR filter benefits:
    // - Smooths out noise and high-frequency jitter
    // - Provides stable readings without software filtering
    // - Reduces false touches from electromagnetic interference
    touch_pad_filter_start(10);
    
    // Allow the filter to stabilize before returning
    // First few readings might be inaccurate after startup
    delay(50);
    return true;
}

/**
 * Read the current hardware-filtered value from the touch sensor.
 * This function accesses the IIR-filtered reading calculated by the hardware.
 * 
 * @return uint16_t The filtered ADC value (typically 0-4095)
 *         Lower values = touched state
 *         Higher values = untouched state
 */
uint16_t myTouch::leggiFiltrato() {
    uint16_t val;
    // Read the IIR-filtered value directly from hardware
    // This function returns immediately with the latest smoothed reading
    touch_pad_read_filtered(_pin, &val);
    return val;
}

/**
 * Perform automatic calibration by averaging multiple untouched readings.
 * Call this once during setup() to establish the baseline value.
 * 
 * The calibration process:
 * 1. Takes 'campioni' readings spaced 20ms apart
 * 2. Averages them to reduce noise in the baseline
 * 3. Returns the average as the resting state value
 * 
 * @param campioni Number of samples to average (default: 10)
 * @return uint16_t The average resting value (baseline)
 */
uint16_t myTouch::calibra(uint8_t campioni) {
    uint32_t somma = 0;
    
    // Collect multiple samples
    for (uint8_t i = 0; i < campioni; i++) {
        somma += leggiFiltrato();
        delay(20); // 20ms between samples ensures independent readings
    }
    
    // Return the average - this is your baseline/resting value
    // For touch detection, typically use: threshold = baseline * 0.8
    return (uint16_t)(somma / campioni);
}