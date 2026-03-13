/**
 * Project: Touch Sensor with ESP32 IDF
 * Description: Uses the native ESP32 touch sensor capabilities to read filtered values
 * and automatically calibrate activation threshold.
 *
 * Why this library is superior to standard Arduino-ESP32 touch library:
 *
 * 1. Hardware IIR Filtering
 *    - Raw instantaneous readings are noisy and unreliable
 *    - This uses hardware-calculated weighted average updated every 10ms
 *    - Result: Clean, responsive readings without software overhead
 *
 * 2. Noise Suppression via Reference Channel
 *    - TOUCH_PAD_NUM0 (GPIO 4) with 10nF capacitor acts as stable reference
 *    - Creates a baseline for hardware differentiation
 *    - Reduces noise, EMI interference, and capacitive coupling effects
 *    - Dramatically improves reliability in real-world environments
 *
 * 3. Optimized Voltage Configuration
 *    - TOUCH_HVOLT_2V7: Charges electrode at higher voltage (faster, stronger signal)
 *    - TOUCH_LVOLT_0V5: Lower baseline voltage (cleaner discharge curve)
 *    - TOUCH_HVOLT_ATTEN_1V: Voltage attenuation (protects hardware)
 *    - Result: Maximum Signal-to-Noise Ratio for reliable detection
 *
 * Hardware Requirements:
 * - Remove the ESP32 DevkitV4 from the ESP32-LCDkit before flashing
 * - 10nF capacitor between GPIO 4 (TOUCH_PAD_NUM0) and GND
 * - Touch electrode on GPIO 32 / TOUCH_PAD_NUM9 and GPIO 33 / TOUCH_PAD_NUM8
 * - Proper power supply with decoupling capacitors
 *
 * Calibration: Ensure the 10nF capacitor is properly connected to GPIO 4 and GND
 *
 * @author DL26
 * @version 2.0
 */

#include <Arduino.h>
#include "../src/myTouch/myTouch.h"

#define TOUCH1 TOUCH_PAD_NUM8
#define TOUCH2 TOUCH_PAD_NUM9

// Define an array of touch pads to initialize (optional, can be used for multi-pad support)
// TOUCH_PAD_NUM9 is mapped on GPIO 32
// TOUCH_PAD_NUM8 is mapped on GPIO 33
touch_pad_t sensorsPin[] = {TOUCH1, TOUCH2};

// Sensor instance
myTouch touch;

// IIR low pass filtering
const float tau = 1.0;               // Time constant for the IIR filter (in seconds)
const float dt = 0.15;               // Loop update interval (in seconds)
const float alpha = dt / (tau + dt); // Smoothing factor for the IIR filter
float filteredValue1 = 0;            // Initialize filtered value
float filteredValue2 = 0;            // Unused second filtered value for demonstration
float delta;

void setup()
{
    Serial.begin(115200);
    delay(500); // Allow Serial to stabilize
    Serial.println("Check your connections:");
    Serial.println("  1. 10nF capacitor between GPIO 4 and GND");
    Serial.println("  2. Touch electrodes connected to GPIO 32 and GPIO 33");

    // Important: DO NOT TOUCH THE SENSOR during calibration!
    Serial.println("Starting calibration...");
    Serial.println("**DO NOT TOUCH THE SENSOR DURING CALIBRATION**");
    delay(2000); // Give user 2 seconds to see the warning

    // Initialize the myTouch system on GPIO 32 (TOUCH_PAD_NUM9) and GPIO 33 (TOUCH_PAD_NUM8)
    // This configures the ESP32 hardware: voltage levels, filter, reference channel
    if (touch.begin(sensorsPin, sizeof(sensorsPin) / sizeof(sensorsPin[0])))
    {
        Serial.println("Touch Sensor System Ready!");
        Serial.println("");

        // Initialize the filtered value with the baseline for smooth startup
        filteredValue1 = touch.readFiltered(sensorsPin[1]);
    }
    else
    {
        Serial.println("ERROR: Failed to initialize touch sensor!");
        Serial.println("Check your connections:");
        Serial.println("  1. 10nF capacitor between GPIO 4 and GND");
        Serial.println("  2. Touch electrode connected to GPIO 32 and GPIO 33");
        Serial.println("Board will halt.");
        while (1)
            delay(1000); // Halt execution
    }
}

void loop()
{
    // Read the current hardware-filtered value from the sensor
    //
    // What does this do?
    // - The ESP32 hardware maintains an IIR (Infinite Impulse Response) filter
    // - Filter updates every 10ms (100 Hz)
    // - You get a smooth, noise-free value instead of jittery raw ADC reading
    // - Returns immediately with the latest filtered value
    //
    // Value interpretation:
    // - Higher value (untouched):  ~2500-3500
    // - Lower value (touched):     ~500-1500

    // Read from GPIO 33 / TOUCH_PAD_NUM8
    uint16_t value1 = touch.readFiltered(sensorsPin[0]);
    // Read from GPIO 32 / TOUCH_PAD_NUM9
    uint16_t value2 = touch.readFiltered(sensorsPin[1]);
    // Apply IIR filter to smooth readings
    filteredValue1 = alpha * value1 + (1 - alpha) * filteredValue1;
    filteredValue2 = alpha * value2 + (1 - alpha) * filteredValue2;

    delta = filteredValue1 - filteredValue2;

    // Plot data on Teleplot for visualization (optional)
    Serial.print(">level:");
    //Serial.println(delta, 2);
    Serial.print(filteredValue1, 2);


    /*
        // Compare reading against the calibrated threshold
        if (filteredValue < activationThreshold)
        {
            // TOUCHED STATE
            // Sensor reading dropped below threshold
          //  Serial.print("TOUCHED! Value: ");
        }
        else
        {
            // UNTOUCHED STATE
            // Sensor reading is above threshold
          //  Serial.print("Free... Value: ");
        }
    */
    // Print the current sensor reading for debugging/monitoring
    // Serial.println(filteredValue);

    // Loop update rate: 150ms
    // This means we check the sensor ~6-7 times per second
    // Fast enough for responsive touch detection
    // Slow enough to avoid serial flooding
    delay(dt * 1000);
}