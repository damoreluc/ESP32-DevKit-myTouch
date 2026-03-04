# myTouch Quick Reference v2.0

## Installation
```cpp
#include "myTouch/myTouch.h"

myTouch touch;
```

---

## Single Sensor Setup

```cpp
void setup() {
    Serial.begin(115200);
    delay(500);
    
    // Initialize on GPIO 33 (TOUCH_PAD_NUM8)
    if (!touch.begin(TOUCH_PAD_NUM8)) {
        Serial.println("Initialization failed!");
        while(1) delay(1000);
    }
}

void loop() {
    uint16_t value = touch.readFiltered(TOUCH_PAD_NUM8);
    
    if (value < 1500) {
        Serial.println("TOUCHED!");
    }
    delay(100);
}
```

---

## Multiple Sensors Setup (Recommended)

```cpp
#define TOUCH1 TOUCH_PAD_NUM8   // GPIO 33
#define TOUCH2 TOUCH_PAD_NUM9   // GPIO 32

touch_pad_t sensorsPin[] = {TOUCH1, TOUCH2};
myTouch touch;

// IIR filter for smoothing
const float tau = 1.0;
const float dt = 0.15;
const float alpha = dt / (tau + dt);
float filteredValue1 = 0, filteredValue2 = 0;

void setup() {
    Serial.begin(115200);
    delay(500);
    
    if (!touch.begin(sensorsPin, sizeof(sensorsPin) / sizeof(sensorsPin[0]))) {
        Serial.println("Initialization failed!");
        while(1) delay(1000);
    }
    
    filteredValue1 = touch.readFiltered(sensorsPin[0]);
    filteredValue2 = touch.readFiltered(sensorsPin[1]);
}

void loop() {
    uint16_t val1 = touch.readFiltered(sensorsPin[0]);
    uint16_t val2 = touch.readFiltered(sensorsPin[1]);
    
    // Apply software IIR filter
    filteredValue1 = alpha * val1 + (1 - alpha) * filteredValue1;
    filteredValue2 = alpha * val2 + (1 - alpha) * filteredValue2;
    
    Serial.print("S1: ");
    Serial.print(filteredValue1, 0);
    Serial.print("  S2: ");
    Serial.println(filteredValue2, 0);
    
    delay(150);
}
```

---

## API Summary

| Method | Parameters | Returns | Purpose |
|--------|-----------|---------|---------|
| `begin(pin)` | `touch_pad_t pin` | `bool` | Initialize single sensor |
| `begin(pins, count)` | `const touch_pad_t* pins, size_t count` | `bool` | Initialize multiple sensors |
| `readFiltered(pin)` | `touch_pad_t pin` | `uint16_t` | Read filtered value from pin |
| `calibrate(pin, samples)` | `touch_pad_t pin, uint8_t samples=10` | `uint16_t` | Calibrate baseline for pin |

---

## Available Touch Pins

| Pad | GPIO | Status |
|-----|------|--------|
| NUM0 | 4 | Reference only (10nF cap required) |
| NUM2 | 2 | ✓ Available |
| NUM3 | 15 | ⚠ May conflict with SPI |
| NUM4 | 13 | ✓ Available |
| NUM5 | 12 | ✓ Available |
| NUM6 | 14 | ⚠ May conflict with SPI |
| NUM7 | 27 | ✓ Available |
| NUM8 | 33 | ✓ **Preferred** |
| NUM9 | 32 | ✓ **Preferred** |

---

## Typical Values

- **Untouched**: 2500-3500 (PCB/electrode dependent)
- **Touched**: 500-1500
- **Hardware Filter Rate**: 100 Hz (10ms updates)
- **ADC Resolution**: 12-bit (0-4095)
- **Response Time**: 100-200ms (with filtering)

---

## IIR Filter Tuning

Adjust `tau` (time constant) for different responsiveness:

```cpp
// Fast response (tau = 0.3s)
const float tau = 0.3;
const float alpha = dt / (tau + dt);  // alpha ≈ 0.33

// Balanced (tau = 1.0s)  
const float tau = 1.0;
const float alpha = dt / (tau + dt);  // alpha ≈ 0.13

// Slow/smooth (tau = 2.0s)
const float tau = 2.0;
const float alpha = dt / (tau + dt);  // alpha ≈ 0.07
```

Higher `tau` = smoother but slower response  
Lower `tau` = faster but more jittery

---

## Touch Detection Thresholds

```cpp
// Direct comparison (simple)
if (filteredValue < 1500) {
    // TOUCHED
}

// Percentage-based (adaptive)
uint16_t baseline = touch.calibrate(TOUCH_PAD_NUM8, 20);
uint16_t threshold = baseline * 0.8;  // 20% drop
if (filteredValue < threshold) {
    // TOUCHED
}

// Custom sensitivity
if (filteredValue < (baseline - 300)) {
    // More sensitive
}
if (filteredValue < (baseline - 100)) {
    // Less sensitive
}
```

---

## Hardware Checklist

- ✓ 10nF capacitor between GPIO 4 and GND (REQUIRED)
- ✓ Touch electrode connected to GPIO 32 or GPIO 33
- ✓ ESP32 properly powered (5V supply with capacitor filtering)
- ✓ USB cable connected for serial monitoring

---

## Troubleshooting Quick Tips

| Problem | Solution |
|---------|----------|
| No readings change | 1. Check 10nF capacitor at GPIO 4<br>2. Verify electrode connection<br>3. Try `touch.begin()` with delay(100) before reading |
| Erratic/jittery | Increase IIR `tau` value (slower response)<br>Add 100nF cap parallel to 10nF |
| Unresponsive | Lower sensitivity threshold (use < 1800)<br>Increase calibration samples to 30+ |
| False triggers | Increase threshold (use < 1200)<br>Add debouncing delay |
| Different on boot | Wait 1 second after `begin()` before calibrating |

---

## Pin Mapping Reference

```cpp
// GPIO to TOUCH_PAD mapping:
GPIO  2  →  TOUCH_PAD_NUM2
GPIO  4  →  TOUCH_PAD_NUM0  (Reference, 10nF cap required)
GPIO 12  →  TOUCH_PAD_NUM5
GPIO 13  →  TOUCH_PAD_NUM4
GPIO 14  →  TOUCH_PAD_NUM6
GPIO 15  →  TOUCH_PAD_NUM3
GPIO 27  →  TOUCH_PAD_NUM7
GPIO 32  →  TOUCH_PAD_NUM9  (Preferred)
GPIO 33  →  TOUCH_PAD_NUM8  (Preferred)
```

---

## Loop Timing Notes

Standard loop interval should be **100-150ms**:

```cpp
const float dt = 0.10;  // 100ms - responsive
const float dt = 0.15;  // 150ms - balanced (recommended)
const float dt = 0.25;  // 250ms - relaxed
```

Faster loops don't improve responsiveness (hardware updates at 100 Hz)  
Too slow loops may miss rapid touches

---

## Complete Examples

### Minimal Single Sensor
```cpp
#include <Arduino.h>
#include "myTouch/myTouch.h"

myTouch touch;

void setup() {
    Serial.begin(115200);
    touch.begin(TOUCH_PAD_NUM8);
}

void loop() {
    if (touch.readFiltered(TOUCH_PAD_NUM8) < 1500) {
        Serial.println("Button pressed!");
    }
    delay(100);
}
```

### With Calibration
```cpp
#include <Arduino.h>
#include "myTouch/myTouch.h"

myTouch touch;
uint16_t threshold;

void setup() {
    Serial.begin(115200);
    touch.begin(TOUCH_PAD_NUM8);
    delay(100);
    
    uint16_t baseline = touch.calibrate(TOUCH_PAD_NUM8, 20);
    threshold = baseline * 0.8;
    Serial.println("Calibrated: " + String(baseline));
}

void loop() {
    if (touch.readFiltered(TOUCH_PAD_NUM8) < threshold) {
        Serial.println("TOUCHED!");
    }
    delay(100);
}
```

### Dual Sensor with Filtering (Production Recommended)
See **Multiple Sensors Setup** section above
