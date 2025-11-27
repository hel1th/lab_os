#include "contract.h"

#include <stdlib.h>


int prime_count(int a, int b) {
    if (a > b) {
        int t = a;
        a = b;
        b = t;
    }

    if (b < 2)
        return 0;

    int n = b;
    char* is_prime = calloc(n + 1, 1);

    for (int i = 2; i <= n; i++)
        is_prime[i] = 1;

    for (int i = 2; i * i <= n; i++) {
        if (!is_prime[i])
            continue;
        for (int j = i * i; j <= n; j += i)
            is_prime[j] = 0;
    }

    int count = 0;
    for (int x = a; x <= b; x++)
        if (x >= 2 && is_prime[x])
            count++;

    free(is_prime);
    return count;
}

float area(float a, float b) {
    return 0.5f * a * b;
}
