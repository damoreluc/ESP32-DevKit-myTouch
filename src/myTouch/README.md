# myTouch - ESP32 Native Touch Sensor Library

A lightweight C++ wrapper for ESP32's native IDF touch sensor driver, providing hardware-filtered readings and automatic calibration for reliable touch detection.

## Overview

The `myTouch` class simplifies interaction with the ESP32's capacitive touch sensing capabilities while leveraging the built-in hardware filtering and calibration features. Unlike the standard Arduino framework approach, this library uses the ESP-IDF native touch driver to provide:

- **Hardware IIR Filtering**: Filtered values calculated directly by hardware, not instantaneous noisy readings
- **Noise Suppression**: Reference channel configuration (GPIO 4 / TOUCH_PAD_NUM0) with a 10nF capacitor creates a stable baseline
- **Optimized Voltage Configuration**: Uses TOUCH_HVOLT_2V7 for maximum signal-to-noise ratio
- **Automatic Calibration**: Built-in calibration method to automatically determine activation threshold

## Hardware Requirements

- **ESP32 DevKit Board**
- **10nF Capacitor**: Connected between GPIO 4 (TOUCH_PAD_NUM0) and GND for reference stability
- **Touch Pad**: Connected to the GPIO pin you want to use (e.g., GPIO 13 / TOUCH_PAD_NUM4)

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

Initializes the ESP32 touch sensor system and configures the specified GPIO pin.

**Parameters:**
- `pin` (touch_pad_t): The touch pad number to use. Common values:
  - `TOUCH_PAD_NUM0` - GPIO 4
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
if (touch.begin(TOUCH_PAD_NUM4)) {
    Serial.println("Touch sensor ready!");
} else {
    Serial.println("Touch initialization failed!");
}
```

---

### leggiFiltrato()

Reads the current filtered value from the touch sensor. This value is hardware-filtered and updated every 10ms.

**Parameters:** None

**Returns:** 
- `uint16_t` - The filtered ADC reading (0-4095 range)

**Note:** Lower values indicate a touched state. Typically, untouched values range from ~2000-3000, and touched values drop to ~1000-1500 depending on your setup.

**Example:**
```cpp
uint16_t reading = touch.leggiFiltrato();
Serial.println(reading);
```

---

### calibra(uint8_t campioni = 10)

Automatically calibrates the touch sensor by averaging multiple readings in the untouched state. Use this during setup before the sensor is touched.

**Parameters:**
- `campioni` (optional): Number of samples to average. Default is 10. Higher values = more stable but slower calibration.

**Returns:** 
- `uint16_t` - Average resting value (baseline)

**Example:**
```cpp
uint16_t baselineValue = touch.calibra(20); // 20 samples for more stability
```

---

## Usage Example

### Basic Setup

```cpp
#include <Arduino.h>
#include "myTouch/myTouch.h"

myTouch touch;
uint16_t activationThreshold;

void setup() {
    Serial.begin(115200);
    delay(500);
    
    // Initialize touch sensor on GPIO 13 (TOUCH_PAD_NUM4)
    if (touch.begin(TOUCH_PAD_NUM4)) {
        Serial.println("Touch sensor initialized!");
        
        // Calibrate without touching the sensor
        Serial.println("Calibrating... Do not touch the sensor!");
        delay(1000); // Give user time to read
        
        uint16_t restingValue = touch.calibra(20);
        
        // Set activation threshold to 80% of resting value
        // (sensor value drops when touched)
        activationThreshold = restingValue - (restingValue * 0.2);
        
        Serial.print("Resting value: ");
        Serial.println(restingValue);
        Serial.print("Activation threshold: ");
        Serial.println(activationThreshold);
    } else {
        Serial.println("Failed to initialize touch sensor!");
        while (1) delay(1000);
    }
}

void loop() {
    uint16_t currentReading = touch.leggiFiltrato();
    
    if (currentReading < activationThreshold) {
        Serial.println("TOUCHED!");
    } else {
        Serial.print("Not touched. Value: ");
        Serial.println(currentReading);
    }
    
    delay(100);
}
```

---

## Advanced Configuration

### Adjusting Sensitivity

Modify the threshold calculation in your code:

```cpp
// More sensitive (activates sooner)
activationThreshold = restingValue - (restingValue * 0.3);

// Less sensitive (requires stronger touch)
activationThreshold = restingValue - (restingValue * 0.1);
```

### Multiple Touch Sensors

You can create multiple instances for different touch pads:

```cpp
myTouch touchPin1;
myTouch touchPin2;

void setup() {
    touchPin1.begin(TOUCH_PAD_NUM4);
    touchPin2.begin(TOUCH_PAD_NUM5);
}

void loop() {
    if (touchPin1.leggiFiltrato() < threshold1) {
        // Button 1 touched
    }
    if (touchPin2.leggiFiltrato() < threshold2) {
        // Button 2 touched
    }
}
```

---

## Troubleshooting

### Issue: No change in readings when touched
- **Solution**: Ensure the 10nF capacitor is properly connected between GPIO 4 and GND
- **Solution**: Try increasing the sensitivity threshold (larger percentage reduction)
- **Solution**: Check that metal electrode/touch surface is in contact with the GPIO pin via PCB trace or wire

### Issue: Erratic readings or slow response
- **Solution**: Add a 100nF capacitor in parallel with the 10nF capacitor for additional filtering
- **Solution**: Increase calibration samples: `touch.calibra(30)` instead of default 10

### Issue: Calibration values are inconsistent
- **Solution**: Ensure nobody touches the sensor during calibration
- **Solution**: Wait longer before calibration to let the system stabilize
- **Solution**: Add `delay(3000)` after `begin()` before running `calibra()`

---

## Pin Reference Table

| TOUCH_PAD | GPIO | I/O Availability |
|-----------|------|------------------|
| TOUCH_PAD_NUM0 | 4 | Reserved (used as reference) |
| TOUCH_PAD_NUM2 | 2 | ✓ Available |
| TOUCH_PAD_NUM3 | 15 | ⚠ May conflict with SPI |
| TOUCH_PAD_NUM4 | 13 | ✓ Available |
| TOUCH_PAD_NUM5 | 12 | ✓ Available |
| TOUCH_PAD_NUM6 | 14 | ⚠ May conflict with SPI |
| TOUCH_PAD_NUM7 | 27 | ✓ Available |
| TOUCH_PAD_NUM8 | 33 | ✓ Available |
| TOUCH_PAD_NUM9 | 32 | ✓ Available |

---

## Performance Characteristics

- **Reading Rate**: One new filtered value every 10ms (100 Hz)
- **ADC Resolution**: 12-bit (0-4095)
- **Response Time**: ~100-200ms after touch (due to hardware filtering)
- **Power Consumption**: ~1-2mA (very low power mode available with IDF)

---

## License

This library is provided as-is for ESP32 projects. Based on ESP-IDF native touch driver.
