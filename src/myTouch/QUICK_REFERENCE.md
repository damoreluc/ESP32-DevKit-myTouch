# myTouch Quick Reference

## Installation
```cpp
#include "myTouch/myTouch.h"

myTouch touch;
```

## Basic Setup
```cpp
void setup() {
    pin// Initialize on GPIO 13 (TOUCH_PAD_NUM4)
    touch.begin(TOUCH_PAD_NUM4);
    
    // Calibrate resting value
    uint16_t baseline = touch.calibrate(20);
    
    // Set activation threshold to 20% drop
    uint16_t threshold = baseline - (baseline * 0.2);
}
```

## Reading Values
```cpp
void loop() {
    uint16_t reading = touch.readFiltered();
    
    if (reading < threshold) {
        // Sensor is touched
    }
}
```

---

## Available Touch Pins

| Pad | GPIO |
|-----|------|
| NUM0 | 4 (Reference) |
| NUM2 | 2 |
| NUM3 | 15 |
| NUM4 | 13 ✓ |
| NUM5 | 12 ✓ |
| NUM6 | 14 |
| NUM7 | 27 ✓ |
| NUM8 | 33 ✓ |
| NUM9 | 32 ✓ |

---

## API Summary

### Constructor
```cpp
myTouch()  // Creates new instance
```

### Methods

| Method | Returns | Purpose |
|--------|---------|---------|
| `begin(pin)` | bool | Initialize sensor on GPIO pin |
| `readFiltered()` | uint16_t | Read current filtered value |
| `calibrate(samples)` | uint16_t | Auto-calibrate baseline |

---

## Typical Values

- **Untouched**: 2500-3500 (depends on PCB)
- **Touched**: 500-1500
- **Update Rate**: 100 Hz (10ms)
- **Debounce**: ~50-100ms recommended

---

## Sensitivity Tuning

```cpp
// Very Sensitive
threshold = baseline - (baseline * 0.30);  // 30% drop

// Normal
threshold = baseline - (baseline * 0.20);  // 20% drop

// Less Sensitive
threshold = baseline - (baseline * 0.10);  // 10% drop
```

---

## Hardware Checklist

✓ 10nF capacitor between GPIO 4 and GND  
✓ Touch electrode connected to selected GPIO  
✓ ESP32 properly powered  
✓ USB cable for serial debugging  

---

## Common Issues & Solutions

| Problem | Solution |
|---------|----------|
| No response | Check 10nF capacitor connection |
| Erratic readings | Add 100nF parallel to 10nF capacitor |
| False touches | Increase threshold percentage (use 0.25 instead of 0.20) |
| Slow response | Reduce `calibrate()` samples or add debouncing |

---

## Complete Minimal Example

```cpp
#include <Arduino.h>
#include "myTouch/myTouch.h"

myTouch touch;
uint16_t threshold;

void setup() {
    Serial.begin(115200);
    touch.begin(TOUCH_PAD_NUM4);
    uint16_t baseline = touch.calibrate(20);
    threshold = baseline - (baseline * 0.2);
    Serial.println("Ready!");
}

void loop() {
    if (touch.readFiltered() < threshold) {
        Serial.println("TOUCHED!");
    }
    delay(100);
}
```
