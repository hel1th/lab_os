#include "contract.h"

#include <stdio.h>
#include <string.h>

// Функции из библиотеки в компайл тайм (например libnaive.so)
extern int prime_count(int a, int b);
extern float area(float a, float b);

int main() {
    printf("Программа 1: Статическая линковка с библиотекой\n");
    printf("Команды:\n");
    printf("  1 <a> <b>  - подсчет простых чисел на отрезке [a, b]\n");
    printf("  2 <a> <b>  - вычисление площади (a, b - стороны)\n");
    printf("  0          - выход\n\n");

    char input[512];
    int cmd, a, b;
    float fa, fb;

    while (1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin))
            break;
        
        input[strcspn(input, "\n")] = 0;

        if (sscanf(input, "%d", &cmd) != 1) {
            printf("Неверная команда\n");
            continue;
        }

        if (cmd == 0) {
            printf("Выход.\n");
            break;
        } else if (cmd == 1) {
            if (sscanf(input, "%d %d %d", &cmd, &a, &b) != 3) {
                printf("Формат: 1 <a> <b>\n");
                continue;
            }
            int result = prime_count(a, b);
            printf("Количество простых чисел на [%d, %d]: %d\n", a, b, result);
        } else if (cmd == 2) {
            if (sscanf(input, "%d %f %f", &cmd, &fa, &fb) != 3 || fa <= 0 || fb <= 0) {
                printf("Формат: 2 <a> <b> (a, b > 0)\n");
                continue;
            }
            float result = area(fa, fb);
            printf("Площадь: %.4f\n", result);
        } else {
            printf("Неизвестная команда\n");
        }
    }

    return 0;
}