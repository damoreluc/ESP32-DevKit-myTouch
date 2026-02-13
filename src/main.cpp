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
 * - 10nF capacitor between GPIO 4 (TOUCH_PAD_NUM0) and GND
 * - Touch electrode on selected GPIO (e.g., GPIO 13 / TOUCH_PAD_NUM4)
 * - Proper power supply with decoupling capacitors
 * 
 * Calibration: Ensure the 10nF capacitor is properly connected to GPIO 4 and GND
 * 
 * @author DL26
 * @version 1.0
 */

#include <Arduino.h>
#include "../src/myTouch/myTouch.h"

// Touch sensor instance
myTouch touch;

// Activation threshold - calculated during calibration
// When sensor value drops below this, the sensor is considered "touched"
uint16_t activationThreshold;

void setup()
{
    Serial.begin(115200);
    delay(500); // Allow Serial to stabilize
    
    // Initialize the myTouch system on GPIO 13 (TOUCH_PAD_NUM4)
    // This configures the ESP32 hardware: voltage levels, filter, reference channel
    if (touch.begin(TOUCH_PAD_NUM4))
    {
        Serial.println("Touch Sensor System Ready!");
        Serial.println("");

        // ==================== CALIBRATION PHASE ====================
        // Automatic calibration: reads baseline value without touching the sensor
        // 
        // How it works:
        // 1. Takes 20 samples of untouched readings (20ms apart)
        // 2. Averages them to eliminate noise
        // 3. Returns the stable baseline value
        // 
        // Important: DO NOT TOUCH THE SENSOR during calibration!
        Serial.println("Starting calibration...");
        Serial.println("**DO NOT TOUCH THE SENSOR DURING CALIBRATION**");
        delay(2000); // Give user 2 seconds to see the warning
        
        uint16_t baseline = touch.calibrate(20);
        
        // ==================== THRESHOLD CALCULATION ====================
        // After getting baseline, we calculate the activation threshold
        // 
        // How it works:
        // - When untouched: sensor value is HIGH (~2500-3500)
        // - When touched: sensor value DROPS (~500-1500)
        // - Threshold = baseline - (baseline * sensitivity%)
        //
        // The 0.2 (20%) sensitivity means:
        // - threshold = baseline - 20% of baseline
        // - Example: baseline=3000 → threshold = 3000 - 600 = 2400
        // - If reading drops below 2400 → sensor considered "touched"
        //
        // Adjust sensitivity:
        // - 0.1 (10%): Less sensitive, requires stronger touch
        // - 0.2 (20%): Normal sensitivity (recommended)
        // - 0.3 (30%): More sensitive, detects light touch
        activationThreshold = baseline - (baseline * 0.2);

        // ==================== DEBUG OUTPUT ====================
        Serial.println("");
        Serial.println("=== CALIBRATION COMPLETE ===");
        Serial.print("Resting value: ");
        Serial.println(baseline);
        Serial.print("Activation threshold: ");
        Serial.println(activationThreshold);
        Serial.println("");
        Serial.println("Touch detection active!");
        Serial.println("Expected readings:");
        Serial.println("  - Untouched: > " + String(activationThreshold));
        Serial.println("  - Touched:   < " + String(activationThreshold));
        Serial.println("=========================");
        Serial.println("");
    }
    else
    {
        Serial.println("ERROR: Failed to initialize touch sensor!");
        Serial.println("Check your connections:");
        Serial.println("  1. 10nF capacitor between GPIO 4 and GND");
        Serial.println("  2. Touch electrode connected to GPIO 13");
        Serial.println("Board will halt.");
        while (1) delay(1000); // Halt execution
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
    uint16_t value = touch.readFiltered();

    // Compare reading against the calibrated threshold
    if (value < activationThreshold)
    {
        // TOUCHED STATE
        // Sensor reading dropped below threshold
        Serial.print("TOUCHED! Value: ");
    }
    else
    {
        // UNTOUCHED STATE
        // Sensor reading is above threshold
        Serial.print("Free... Value: ");
    }

    // Print the current sensor reading for debugging/monitoring
    Serial.println(value);
    
    // Loop update rate: 100ms
    // This means we check the sensor 10 times per second
    // Fast enough for responsive touch detection
    // Slow enough to avoid serial flooding
    delay(100);
}