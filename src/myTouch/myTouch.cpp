#include "myTouch.h"

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
bool myTouch::begin(touch_pad_t pin)
{
    _pin = pin;
    esp_err_t err;

    // 1. Initialize IDF touch driver
    // This must be called first to set up the Touch hardware
    err = touch_pad_init();
    if (err != ESP_OK)
        return false;

    // Configure touch measurement timing
    // sleep_cycle: Time the touch sensor sleeps between measurements (0x1000 = 4096 cycles)
    // meas_cycle: Time spent measuring touch (0x8000 = 32768 cycles
    touch_pad_set_meas_time(0x1000, 0xffff);

    // 3. Configure voltage levels for optimal performance
    // TOUCH_HVOLT_2V7: High voltage = 2.7V (charges electrode faster, stronger signal)
    // TOUCH_LVOLT_0V5: Low voltage = 0.5V (lower baseline, cleaner signal)
    // TOUCH_HVOLT_ATTEN_1V: Attenuation = 1V (reduces overvoltage stress)
    // This configuration maximizes the signal-to-noise ratio
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

    // 4. Configure charging speed and tie option
    // TOUCH_PAD_SLOPE_7: Slowest charging speed (more counts per measurement cycle)
    // TOUCH_PAD_TIE_OPT_LOW: Start charging from low level (0.5V) for better sensitivity
    // This allows for a more gradual charge/discharge curve, improving resolution and stability
    touch_pad_set_cnt_mode(_pin, TOUCH_PAD_SLOPE_7, TOUCH_PAD_TIE_OPT_LOW);

    // 5. Configure the selected pin for touch sensing
    // Second parameter (0) is the threshold - we handle detection logic in software
    err = touch_pad_config(_pin, 0);
    if (err != ESP_OK)
    {
        Serial.println("Error configuring touch pad: " + String(esp_err_to_name(err)));
        return false;
    }

    // 6. Configure reference channel (GPIO 4 / TOUCH_PAD_NUM0)
    // Even though we don't read from it, configuring it as reference provides:
    // - Stable baseline for hardware comparison
    // - Better noise immunity through differential measurement
    // - Hardware requirement for reliable operation
    // The 10nF capacitor on this pin is essential for stability
    err = touch_pad_config(TOUCH_PAD_NUM0, 0);
    if (err != ESP_OK)
    {
        Serial.println("Error configuring reference pad: " + String(esp_err_to_name(err)));
        return false;
    }

    // 7. Enable hardware IIR (Infinite Impulse Response) filter
    // Period of 10ms means the filter updates 100 times per second
    // IIR filter benefits:
    // - Smooths out noise and high-frequency jitter
    // - Provides stable readings without software filtering
    // - Reduces false touches from electromagnetic interference
    err = touch_pad_filter_start(20);
    if (err != ESP_OK)
    {
        Serial.println("Driver state error: " + String(esp_err_to_name(err)));
        return false;
    }

    // Allow the filter to stabilize before returning
    // First few readings might be inaccurate after startup
    delay(100);
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
uint16_t myTouch::readFiltered()
{
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
 * 1. Takes 'samples' readings spaced 20ms apart
 * 2. Averages them to reduce noise in the baseline
 * 3. Returns the average as the resting state value
 *
 * @param samples Number of samples to average (default: 10)
 * @return uint16_t The average resting value (baseline)
 */
uint16_t myTouch::calibrate(uint8_t samples)
{
    uint32_t sum = 0;

    // Collect multiple samples
    for (uint8_t i = 0; i < samples; i++)
    {
        sum += readFiltered();
        delay(20); // 20ms between samples ensures independent readings
    }

    // Return the average - this is your baseline/resting value
    // For touch detection, typically use: threshold = baseline * 0.8
    return (uint16_t)(sum / samples);
}