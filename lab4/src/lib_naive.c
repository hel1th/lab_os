#include "contract.h"

#include <math.h>

int prime_count(int a, int b) {
    if (a > b) {
        int t = a;
        a = b;
        b = t;
    }

    int count = 0;

    for (int x = a; x <= b; x++) {
        if (x < 2)
            continue;

        int is_prime = 1;
        for (int d = 2; d < x; d++) {
            if (x % d == 0) {
                is_prime = 0;
                break;
            }
        }

        count += is_prime;
    }

    return count;
}

float area(float a, float b) {
    return a * b;
}
