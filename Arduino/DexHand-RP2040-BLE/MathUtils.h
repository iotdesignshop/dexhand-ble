#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <Arduino.h>

// Clamp a value between a min and max
#define CLAMP(value, min, max) (value < min ? min : (value > max ? max : value))


// Takes an integer and range, and returns a float between 0.0 and 1.0
float normalizedValue(int32_t value, int32_t min, int32_t max);

// Maps an integer value from one range to another
int32_t mapInteger(int32_t value, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax);


#endif // MATH_UTILS_H