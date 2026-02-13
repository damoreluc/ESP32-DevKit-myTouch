/*
 * myTouch Library - Usage Examples
 * 
 * This file contains several practical examples demonstrating
 * different ways to use the myTouch class.
 */

// ============================================
// EXAMPLE 1: Basic Touch Detection
// ============================================
#if 0  // Set to 1 to use this example

#include <Arduino.h>
#include "myTouch.h"

myTouch touch;
uint16_t threshold;

void setup() {
    Serial.begin(115200);
    
    if (touch.begin(TOUCH_PAD_NUM4)) {
        uint16_t baseline = touch.calibra(20);
        threshold = baseline - (baseline * 0.2);
        
        Serial.println("Touch sensor ready!");
        Serial.print("Baseline: ");
        Serial.println(baseline);
    }
}

void loop() {
    uint16_t value = touch.leggiFiltrato();
    
    if (value < threshold) {
        Serial.println("TOUCHED!");
    }
    
    delay(100);
}

#endif


// ============================================
// EXAMPLE 2: Touch Button with Debouncing
// ============================================
#if 0  // Set to 1 to use this example

#include <Arduino.h>
#include "myTouch.h"

myTouch touch;
uint16_t threshold;
bool previousState = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; // 50ms debounce

void setup() {
    Serial.begin(115200);
    
    Serial.println("Calibrating touch sensor...");
    touch.begin(TOUCH_PAD_NUM4);
    uint16_t baseline = touch.calibra(30);
    threshold = baseline - (baseline * 0.25);
    
    Serial.println("Ready!");
}

void loop() {
    uint16_t value = touch.leggiFiltrato();
    bool currentState = (value < threshold);
    
    // Debounce logic
    if (currentState != previousState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (currentState && !previousState) {
            // Touch detected (rising edge)
            Serial.println("Button pressed!");
        }
        else if (!currentState && previousState) {
            // Touch released (falling edge)
            Serial.println("Button released!");
        }
    }
    
    previousState = currentState;
    delay(10);
}

#endif


// ============================================
// EXAMPLE 3: Multiple Touch Buttons
// ============================================
#if 0  // Set to 1 to use this example

#include <Arduino.h>
#include "myTouch.h"

myTouch button1;
myTouch button2;
myTouch button3;

uint16_t threshold1, threshold2, threshold3;

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println("Initializing 3 touch buttons...");
    
    // Initialize Button 1 (GPIO 13 / TOUCH_PAD_NUM4)
    if (button1.begin(TOUCH_PAD_NUM4)) {
        uint16_t baseline = button1.calibra(20);
        threshold1 = baseline - (baseline * 0.2);
        Serial.print("Button 1 calibrated. Baseline: ");
        Serial.println(baseline);
    }
    
    // Initialize Button 2 (GPIO 12 / TOUCH_PAD_NUM5)
    if (button2.begin(TOUCH_PAD_NUM5)) {
        uint16_t baseline = button2.calibra(20);
        threshold2 = baseline - (baseline * 0.2);
        Serial.print("Button 2 calibrated. Baseline: ");
        Serial.println(baseline);
    }
    
    // Initialize Button 3 (GPIO 27 / TOUCH_PAD_NUM7)
    if (button3.begin(TOUCH_PAD_NUM7)) {
        uint16_t baseline = button3.calibra(20);
        threshold3 = baseline - (baseline * 0.2);
        Serial.print("Button 3 calibrated. Baseline: ");
        Serial.println(baseline);
    }
    
    Serial.println("All buttons ready!");
}

void loop() {
    uint16_t val1 = button1.leggiFiltrato();
    uint16_t val2 = button2.leggiFiltrato();
    uint16_t val3 = button3.leggiFiltrato();
    
    if (val1 < threshold1) Serial.println("Button 1: TOUCHED");
    if (val2 < threshold2) Serial.println("Button 2: TOUCHED");
    if (val3 < threshold3) Serial.println("Button 3: TOUCHED");
    
    delay(100);
}

#endif


// ============================================
// EXAMPLE 4: Touch Counter with Hysteresis
// ============================================
#if 0  // Set to 1 to use this example

#include <Arduino.h>
#include "myTouch.h"

myTouch touch;
uint16_t activationThreshold;
uint16_t releaseThreshold;
bool isTouched = false;
uint32_t touchCount = 0;

void setup() {
    Serial.begin(115200);
    
    Serial.println("Initializing touch counter...");
    touch.begin(TOUCH_PAD_NUM4);
    
    uint16_t baseline = touch.calibra(25);
    
    // Create hysteresis band (20% below baseline to activate, 15% to release)
    activationThreshold = baseline - (baseline * 0.20);
    releaseThreshold = baseline - (baseline * 0.15);
    
    Serial.print("Baseline: ");
    Serial.println(baseline);
    Serial.print("Activation threshold: ");
    Serial.println(activationThreshold);
    Serial.print("Release threshold: ");
    Serial.println(releaseThreshold);
    Serial.println("Ready to count touches!");
}

void loop() {
    uint16_t value = touch.leggiFiltrato();
    
    // Hysteresis logic prevents bouncing between states
    if (!isTouched && value < activationThreshold) {
        isTouched = true;
        touchCount++;
        Serial.print("Touch detected! Count: ");
        Serial.println(touchCount);
    }
    else if (isTouched && value > releaseThreshold) {
        isTouched = false;
        Serial.println("Touch released");
    }
    
    delay(50);
}

#endif


// ============================================
// EXAMPLE 5: Real-time Value Display
// ============================================
#if 0  // Set to 1 to use this example

#include <Arduino.h>
#include "myTouch.h"

myTouch touch;
uint16_t baseline;
uint16_t threshold;
unsigned long lastPrintTime = 0;

void setup() {
    Serial.begin(115200);
    
    touch.begin(TOUCH_PAD_NUM4);
    baseline = touch.calibra(20);
    threshold = baseline - (baseline * 0.2);
}

void loop() {
    uint16_t value = touch.leggiFiltrato();
    
    // Print every 200ms to avoid flooding serial
    if (millis() - lastPrintTime >= 200) {
        lastPrintTime = millis();
        
        int difference = baseline - value;
        int percentChange = (difference * 100) / baseline;
        
        Serial.print("Raw: ");
        Serial.print(value);
        Serial.print(" | Baseline: ");
        Serial.print(baseline);
        Serial.print(" | Diff: ");
        Serial.print(difference);
        Serial.print(" (");
        Serial.print(percentChange);
        Serial.print("%) | Status: ");
        
        if (value < threshold) {
            Serial.println("TOUCHED");
        } else {
            Serial.println("FREE");
        }
    }
    
    delay(10);
}

#endif


// ============================================
// EXAMPLE 6: Adaptive Sensitivity
// ============================================
#if 0  // Set to 1 to use this example

#include <Arduino.h>
#include "myTouch.h"

myTouch touch;
uint16_t baseline;

// Sensitivity levels: 1=most sensitive, 3=least sensitive
enum Sensitivity { VERY_SENSITIVE = 1, NORMAL = 2, INSENSITIVE = 3 };
Sensitivity currentSensitivity = NORMAL;

uint16_t getThreshold(Sensitivity sensitivity) {
    switch(sensitivity) {
        case VERY_SENSITIVE:
            return baseline - (baseline * 0.30);  // 30% drop
        case NORMAL:
            return baseline - (baseline * 0.20);  // 20% drop
        case INSENSITIVE:
            return baseline - (baseline * 0.10);  // 10% drop
        default:
            return baseline - (baseline * 0.20);
    }
}

const char* sensitivityName(Sensitivity s) {
    switch(s) {
        case VERY_SENSITIVE: return "VERY SENSITIVE";
        case NORMAL: return "NORMAL";
        case INSENSITIVE: return "INSENSITIVE";
        default: return "UNKNOWN";
    }
}

void setup() {
    Serial.begin(115200);
    
    Serial.println("Touch sensor with adaptive sensitivity");
    touch.begin(TOUCH_PAD_NUM4);
    baseline = touch.calibra(20);
    
    Serial.print("Baseline: ");
    Serial.println(baseline);
    printSensitivityThreshold();
}

void printSensitivityThreshold() {
    uint16_t threshold = getThreshold(currentSensitivity);
    Serial.print("Sensitivity: ");
    Serial.print(sensitivityName(currentSensitivity));
    Serial.print(" | Threshold: ");
    Serial.println(threshold);
}

void loop() {
    uint16_t value = touch.leggiFiltrato();
    uint16_t threshold = getThreshold(currentSensitivity);
    
    // Simple touch detection
    static bool wasTouched = false;
    bool isTouched = (value < threshold);
    
    if (isTouched && !wasTouched) {
        Serial.println("TOUCHED!");
        
        // Cycle sensitivity on touch (for demo purposes)
        currentSensitivity = (Sensitivity)((currentSensitivity + 1) % 3);
        printSensitivityThreshold();
    }
    
    wasTouched = isTouched;
    delay(100);
}

#endif
