# myTouch - ESP32 Native Touch Sensor Library v2.0

A lightweight C++ wrapper for ESP32's native IDF touch sensor driver, providing hardware-filtered readings and support for multiple touch sensors with automatic calibration.

**Author: DL'26**

## Overview

The `myTouch` class simplifies interaction with the ESP32's capacitive touch sensing capabilities while leveraging the built-in hardware filtering. Unlike the standard Arduino framework approach, this library uses the ESP-IDF native touch driver to provide:

- **Hardware IIR Filtering**: Filtered values calculated directly by hardware at 100 Hz (every 10ms), not instantaneous noisy readings
- **Noise Suppression**: Reference channel configuration (GPIO 4 / TOUCH_PAD_NUM0) with a 10nF capacitor creates a stable baseline
- **Optimized Voltage Configuration**: Uses TOUCH_HVOLT_2V7 for maximum signal-to-noise ratio
- **Multi-Sensor Support**: Initialize and read from multiple touch sensors in a single instance
- **Simplified API**: No complex parameter tuning required—works with sensible defaults

## Hardware Requirements

- **ESP32 DevKit Board**
- **10nF Capacitor**: Connected between GPIO 4 (TOUCH_PAD_NUM0) and GND for reference stability (REQUIRED)
- **Touch Pads**: Connected to GPIO pins you want to monitor (e.g., GPIO 32, GPIO 33)

## Installation

1. Add the myTouch files to your project:
   - `myTouch.h` - Header file
   - `myTouch.cpp` - Implementation

2. Include in your main sketch:
   ```cpp
   #include "myTouch/myTouch.h"
   ```

## API Reference

### Constructor

```cpp
myTouch();
```

Creates a new myTouch instance. The actual initialization happens in the `begin()` method.

---

### begin(touch_pad_t pin)

Initializes the ESP32 touch sensor system and configures a single GPIO pin.

**Parameters:**
- `pin` (touch_pad_t): The touch pad number to use. Common values:
  - `TOUCH_PAD_NUM2` - GPIO 2
  - `TOUCH_PAD_NUM4` - GPIO 13
  - `TOUCH_PAD_NUM5` - GPIO 12
  - `TOUCH_PAD_NUM7` - GPIO 27
  - `TOUCH_PAD_NUM8` - GPIO 33
  - `TOUCH_PAD_NUM9` - GPIO 32

**Returns:** 
- `true` - Initialization successful
- `false` - Initialization failed

**Example:**
```cpp
myTouch touch;
if (touch.begin(TOUCH_PAD_NUM8)) {
    Serial.println("Touch sensor ready!");
}
```

---

### begin(const touch_pad_t *pins, size_t count)

Initializes the ESP32 touch sensor system and configures multiple GPIO pins at once.

**Parameters:**
- `pins` (const touch_pad_t*): Array of touch pad numbers to initialize
- `count` (size_t): Number of touch pads in the array

**Returns:** 
- `true` - Initialization successful
- `false` - Initialization failed

**Example:**
```cpp
myTouch touch;
touch_pad_t sensorsPin[] = {TOUCH_PAD_NUM8, TOUCH_PAD_NUM9};
if (touch.begin(sensorsPin, sizeof(sensorsPin) / sizeof(sensorsPin[0]))) {
    Serial.println("Multiple touch sensors ready!");
}
```

---

### readFiltered(touch_pad_t pin)

Reads the current hardware-filtered touch sensor value. This value is updated every 10ms (100 Hz) by the hardware IIR filter.

**Parameters:**
- `pin` (touch_pad_t): The touch pad to read from

**Returns:** 
- `uint16_t` - The hardware-filtered ADC reading (0-4095 range)

**Value Interpretation:**
- Untouched: ~2500-3500 (depending on PCB layout and electrode size)
- Touched:   ~500-1500

**Notes:**
- This function returns immediately with the latest filtered value
- Hardware filter updates at 10ms intervals (100 Hz), so calling faster won't give new data
- Lower values indicate touch, higher values indicate untouched state

**Example:**
```cpp
uint16_t value = touch.readFiltered(TOUCH_PAD_NUM8);
if (value < 1500) {
    Serial.println("Button pressed!");
}
```

---

### calibrate(touch_pad_t pin, uint8_t samples = 10)

Performs automatic calibration by averaging multiple untouched readings. The returned baseline can be used to calculate activation thresholds.

**Parameters:**
- `pin` (touch_pad_t): The touch pad to calibrate
- `samples` (uint8_t): Number of samples to average. Default: 10
  - 10: Fast calibration (~200ms) - stable environments
  - 20: Medium calibration (~400ms) - recommended
  - 30+: Slow calibration (600ms+) - noisy environments

**Returns:** 
- `uint16_t` - Average resting value (baseline)

**Example:**
```cpp
Serial.println("Calibrating... do not touch!");
uint16_t baseline = touch.calibrate(TOUCH_PAD_NUM8, 20);
uint16_t threshold = baseline * 0.8;  // 20% drop triggers detection
Serial.println("Baseline: " + String(baseline));
Serial.println("Threshold: " + String(threshold));
```

---

## Usage Examples

### Basic Single Sensor Setup

```cpp
#include <Arduino.h>
#include "myTouch/myTouch.h"

myTouch touch;

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println("Check your connections:");
    Serial.println("  1. 10nF capacitor between GPIO 4 and GND");
    Serial.println("  2. Touch electrode connected to GPIO 33");
    
    // Initialize touch sensor on GPIO 33 (TOUCH_PAD_NUM8)
    if (touch.begin(TOUCH_PAD_NUM8)) {
        Serial.println("Touch sensor initialized!");
    } else {
        Serial.println("Failed to initialize touch sensor!");
        while (1) delay(1000);
    }
}

void loop() {
    // Read the hardware-filtered value from the sensor
    uint16_t value = touch.readFiltered(TOUCH_PAD_NUM8);
    
    // Display the current reading
    Serial.print("Sensor value: ");
    Serial.println(value);
    
    // Check if sensor is touched
    if (value < 1500) {
        Serial.println("TOUCHED!");
    } else {
        Serial.println("Not touched");
    }
    
    delay(150);
}
```

---

### Multiple Sensors with IIR Smoothing (Recommended)

This example demonstrates the recommended usage pattern from the main application—initializing multiple sensors and applying additional software filtering for even smoother results.

```cpp
#include <Arduino.h>
#include "myTouch/myTouch.h"

// Define touch sensor pins
#define TOUCH1 TOUCH_PAD_NUM8   // GPIO 33
#define TOUCH2 TOUCH_PAD_NUM9   // GPIO 32

// Array of sensors to initialize
touch_pad_t sensorsPin[] = {TOUCH1, TOUCH2};

// Sensor instance
myTouch touch;

// IIR filter parameters for software smoothing
const float tau = 1.0;               // Time constant (seconds)
const float dt = 0.15;               // Loop interval (seconds)
const float alpha = dt / (tau + dt); // Smoothing factor
float filteredValue1 = 0;            // Filtered reading for sensor 1
float filteredValue2 = 0;            // Filtered reading for sensor 2

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println("Check your connections:");
    Serial.println("  1. 10nF capacitor between GPIO 4 and GND");
    Serial.println("  2. Touch electrodes connected to GPIO 32 and GPIO 33");
    
    Serial.println("Starting initialization...");
    Serial.println("DO NOT TOUCH THE SENSORS");
    delay(2000);
    
    // Initialize both touch sensors
    if (touch.begin(sensorsPin, sizeof(sensorsPin) / sizeof(sensorsPin[0]))) {
        Serial.println("Touch Sensor System Ready!");
        
        // Initialize filtered values with baseline
        filteredValue1 = touch.readFiltered(sensorsPin[0]);
        filteredValue2 = touch.readFiltered(sensorsPin[1]);
    } else {
        Serial.println("ERROR: Failed to initialize touch sensors!");
        Serial.println("Check your connections:");
        Serial.println("  1. 10nF capacitor between GPIO 4 and GND");
        Serial.println("  2. Touch electrode connected to GPIO 32 and GPIO 33");
        while (1) delay(1000);
    }
}

void loop() {
    // Read hardware-filtered values from both sensors
    uint16_t value1 = touch.readFiltered(sensorsPin[0]);
    uint16_t value2 = touch.readFiltered(sensorsPin[1]);
    
    // Apply additional IIR filter for ultra-smooth readings
    filteredValue1 = alpha * value1 + (1 - alpha) * filteredValue1;
    filteredValue2 = alpha * value2 + (1 - alpha) * filteredValue2;
    
    // Calculate difference between sensors (useful for differential sensing)
    float delta = filteredValue1 - filteredValue2;
    
    // Print data for visualization or monitoring
    Serial.print("Sensor1: ");
    Serial.print(filteredValue1, 2);
    Serial.print("  Sensor2: ");
    Serial.print(filteredValue2, 2);
    Serial.print("  Delta: ");
    Serial.println(delta, 2);
    
    // Check activation state
    if (filteredValue1 < 1500) {
        Serial.println("Sensor 1: TOUCHED");
    }
    if (filteredValue2 < 1500) {
        Serial.println("Sensor 2: TOUCHED");
    }
    
    // Loop runs every 150ms (~6.7 Hz)
    delay(dt * 1000);
}
```

---

## Advanced Configuration

### Tuning IIR Filter Parameters

The software IIR filter allows you to control response smoothness:

```cpp
// Fast response (follows sensor closely)
const float tau = 0.3;  // Short time constant
const float dt = 0.15;
const float alpha = dt / (tau + dt);

// Slow response (very smooth, less jitter)
const float tau = 2.0;  // Long time constant
const float dt = 0.15;
const float alpha = dt / (tau + dt);
```

The `alpha` value:
- Higher alpha (0.5-1.0): Sensor changes tracked quickly, more responsive
- Lower alpha (0.1-0.3): Sensor changes filtered heavily, less jitter

### Adjusting Sensitivity Threshold

Without a fixed calibration threshold, use these values:

```cpp
// More sensitive (lower threshold)
if (filteredValue < 1200) {
    // TOUCHED
}

// Less sensitive (higher threshold)
if (filteredValue < 1800) {
    // TOUCHED
}

// Dynamic threshold based on baseline
uint16_t threshold = baseline * 0.8;  // 20% drop triggers detection
```

### Multiple Independent Sensor Instances

You can also create separate myTouch instances for truly independent sensor management:

```cpp
myTouch touch1;
myTouch touch2;

void setup() {
    touch1.begin(TOUCH_PAD_NUM8);
    touch2.begin(TOUCH_PAD_NUM9);
}

void loop() {
    uint16_t val1 = touch1.readFiltered(TOUCH_PAD_NUM8);
    uint16_t val2 = touch2.readFiltered(TOUCH_PAD_NUM9);
    // Process independently
}
```

---

## Troubleshooting

### Issue: No change in readings when touched

**Cause:** Missing or improperly connected 10nF reference capacitor

- Verify the 10nF capacitor is soldered between GPIO 4 and GND
- Check for cold solder joints or broken traces
- Use a multimeter to confirm continuity

**Cause:** Electrode not properly connected

- Confirm touch electrode is connected to the GPIO pin (GPIO 32 or GPIO 33)
- Try pressing directly on the trace or PCB pad if using a trace

**Cause:** Sensitivity threshold too aggressive

- Lower the threshold value by 200-300 counts
- Example: change from `< 1500` to `< 1800`

---

### Issue: Erratic readings or slow response

**Cause:** IIR filter time constant too high

- Reduce `tau` parameter: change from `1.0` to `0.5`
- This makes the filter more responsive

**Cause:** Electromagnetic interference (EMI)

- Add a 100nF ceramic capacitor in parallel with the existing 10nF
- Keep electrode wires short and twisted
- Keep away from power lines and high-current paths

---

### Issue: Readings unstable or changing over time

**Cause:** Hardware not fully stabilized

- Ensure `delay(100)` after `begin()` before first read
- Wait 500ms-1s before starting main operation

**Cause:** Temperature drift

- Add the 100nF capacitor for better stability
- ESP32's touch calibration drifts with temperature; this is normal behavior

---

## How It Works (Technical Details)

### Hardware Architecture

1. **Reference Channel (GPIO 4 / TOUCH_PAD_NUM0)**
   - Configured but not actively read
   - Provides a stable baseline for the hardware comparator
   - The 10nF capacitor ensures clean reference voltages

2. **Voltage Configuration**
   - High voltage: 2.7V (charges electrode quickly)
   - Low voltage: 0.5V (baseline, cleaner signal)
   - Attenuation: 1.5V (protects hardware)
   - Result: Maximum signal-to-noise ratio

3. **Hardware IIR Filter**
   - Updates every 10ms (100 Hz)
   - Smooths out high-frequency noise automatically
   - Software filter adds additional smoothing if desired

4. **Measurement Timing**
   - Sleep cycle: 4096 counts between measurements
   - Measurement cycle: 32768 counts per measurement
   - Frame count: 8 measurements per frame

---

## Performance Characteristics

- **Hardware Filter Rate**: 100 Hz (10ms updates)
- **ADC Resolution**: 12-bit (0-4095)
- **Single Reading Latency**: <1ms
- **Typical Response Time**: 100-200ms (depending on filter settings)
- **Power Consumption**: ~1-2mA
- **Untouched Range**: ~2500-3500 (capacitive coupling dependent)
- **Touched Range**: ~500-1500

---

## Pin Reference Table

| TOUCH_PAD | GPIO | Recommended Use |
|-----------|------|-----------------|
| TOUCH_PAD_NUM0 | 4 | Reference only (required) |
| TOUCH_PAD_NUM2 | 2 | ✓ Touch input |
| TOUCH_PAD_NUM3 | 15 | ⚠ May conflict with SPI |
| TOUCH_PAD_NUM4 | 13 | ✓ Touch input |
| TOUCH_PAD_NUM5 | 12 | ✓ Touch input |
| TOUCH_PAD_NUM6 | 14 | ⚠ May conflict with SPI |
| TOUCH_PAD_NUM7 | 27 | ✓ Touch input |
| TOUCH_PAD_NUM8 | 33 | ✓ Touch input (preferred) |
| TOUCH_PAD_NUM9 | 32 | ✓ Touch input (preferred) |

**Recommended pins**: TOUCH_PAD_NUM8 (GPIO 33) and TOUCH_PAD_NUM9 (GPIO 32) are generally the most reliable.

---

## License

This library is provided as-is for ESP32 projects. Based on ESP-IDF native touch driver.
