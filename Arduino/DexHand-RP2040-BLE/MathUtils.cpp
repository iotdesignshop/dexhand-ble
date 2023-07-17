#include "MathUtils.h"


float normalizedValue(int32_t value, int32_t min, int32_t max) {
    float scaled =  static_cast<float>(value - min) / static_cast<float>(max - min);
    return CLAMP(scaled, 0.0f, 1.0f);   // Avoid epsilon errors
}

int32_t mapInteger(int32_t value, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax) {
    // Get normalized value
    float scaled = normalizedValue(value, inMin, inMax);

    // Scale to output range
    int32_t val = static_cast<int32_t>(scaled * (outMax - outMin) + outMin);
    
    // Clamp
    return CLAMP(val, outMin, outMax);  // Avoid epsilon errors
    
}
