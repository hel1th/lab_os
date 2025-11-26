#ifndef CONTRACT_H
#define CONTRACT_H

typedef float (*area_func)(float a, float b);

#ifdef __GNUC__
    #define EXPORT __attribute__((visibility("default")))
#else
    #define EXPORT
#endif

float area(float a, float b);

#endif