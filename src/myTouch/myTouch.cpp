#include "myTouch.h"

/**
 to avoid watchdog reset when using multiple touch sensor instances,
 we need to initialize the hardware module only once.
 The following static variable keeps track of this
*/
bool myTouch::_driverInitialized = false;

/**
 * Constructor: Initialize the touch pad to an invalid state.
 * The actual configuration happens in begin().
 */
myTouch::myTouch() {}

bool myTouch::globalHarwareInitialization()
{
    esp_err_t err;

    // This function is called once to set up the touch hardware module
    // It configures voltage levels, reference channel, and filter settings
    // as per ESP32 datasheet recommendations for stable operation.
    if (!_driverInitialized)
    {
        // 1. Initialize IDF touch driver
        err = touch_pad_init();
        if (err != ESP_OK)
            return false;

        // 2.Configure touch measurement timing
        // sleep_cycle: Time the touch sensor sleeps between measurements (0x1000 = 4096 cycles)
        // meas_cycle: Time spent measuring touch (0x8000 = 32768 cycles
        touch_pad_set_meas_time(0x1000, 0xffff);

        // 3. Configure voltage levels for optimal performance
        // TOUCH_HVOLT_2V7: High voltage = 2.7V (charges electrode faster, stronger signal)
        // TOUCH_LVOLT_0V5: Low voltage = 0.5V (lower baseline, cleaner signal)
        // TOUCH_HVOLT_ATTEN_1V: Attenuation = 1V (reduces overvoltage stress)
        // This configuration maximizes the signal-to-noise ratio
        touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V5);

        // 4. Configure reference channel (GPIO 4 / TOUCH_PAD_NUM0)
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

        // 5. Enable hardware IIR (Infinite Impulse Response) filter
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

        _driverInitialized = true; // Mark driver as initialized to prevent reinitialization

        // 6.Allow the filter to stabilize before returning
        // First few readings might be inaccurate after startup
        delay(100);
    }

    return true;
}

bool myTouch::singlePinConfiguration(touch_pad_t pin)
{
    esp_err_t err;

    // Configure the selected pin for touch sensing
    // Configure charging speed and tie option
    // TOUCH_PAD_SLOPE_7: Slowest charging speed (more counts per measurement cycle)
    // TOUCH_PAD_TIE_OPT_LOW: Start charging from low level (0.5V) for better sensitivity
    // This allows for a more gradual charge/discharge curve, improving resolution and stability
    touch_pad_set_cnt_mode(pin, TOUCH_PAD_SLOPE_1, TOUCH_PAD_TIE_OPT_LOW);

    // Second parameter (0) is the threshold - we handle detection logic in software
    err = touch_pad_config(pin, 0);
    if (err != ESP_OK)
    {
        Serial.println("Error configuring touch pad: " + String(esp_err_to_name(err)));
        return false;
    }

    // wait for the hardware to stabilize after configuration
    delay(50);

    // automatic calibration: store the baseline value for this pin (optional, can be used for multi-pad support)
    _baseline[pin] = calibrate(pin, 15);

    return true;
}

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
    if (!globalHarwareInitialization())
        return false;

    return singlePinConfiguration(pin);
}

bool myTouch::begin(const touch_pad_t *pins, size_t count)
{
    if (!globalHarwareInitialization())
        return false;

    // Configure each pin in the provided array
    for (size_t i = 0; i < count; i++)
    {
        if (!singlePinConfiguration(pins[i]))
            return false;
    }

    return true;
}

/**
 * Read the current hardware-filtered value from the touch sensor.
 * This function accesses the IIR-filtered reading calculated by the hardware.
 *
 * @param pin Touch pad to read from
 *
 * @return uint16_t The filtered ADC value (typically 0-4095)
 *         Lower values = touched state
 *         Higher values = untouched state
 */
uint16_t myTouch::readFiltered(touch_pad_t pin)
{
    uint16_t val;
    if (touch_pad_read_filtered(pin, &val) == ESP_OK)
    {
        return val;
    }
    return 0;
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
 * @param pin Touch pad to calibrate
 * @param samples Number of samples to average (default: 10)
 * @return uint16_t The average resting value (baseline)
 */
uint16_t myTouch::calibrate(touch_pad_t pin,uint8_t samples)
{
    uint32_t sum = 0;

    // Collect multiple samples
    for (uint8_t i = 0; i < samples; i++)
    {
        sum += readFiltered(pin);
        delay(20); // 20ms between samples ensures independent readings
    }

    // Return the average - this is your baseline/resting value
    // For touch detection, typically use: threshold = baseline * 0.8
    return (uint16_t)(sum / samples);
}