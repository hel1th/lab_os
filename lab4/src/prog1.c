#include "contract.h"

#include <stdio.h>

extern float area(float a, float b);

int main() {
    printf("Программа 1 статическая линковка compiletime\n");
    printf("Текущая реализация: ");

    char input[256];
    int cmd;
    float a, b;

    while (1) {
        printf("\n> ");
        if (!fgets(input, sizeof(input), stdin))
            break;

        if (sscanf(input, "%d", &cmd) != 1) {
            printf("Ошибка: введите номер команды!\n");
            continue;
        }

        if (cmd == 0) {
            printf("Выход.\n");
            break;
        }

        if (cmd == 2) {
            if (sscanf(input, "%d %f %f", &cmd, &a, &b) != 3) {
                printf("Ошибка: 2 <a> <b>!\n");
                continue;
            }
            if (a <= 0 || b <= 0) {
                printf("Стороны должны быть > 0!\n");
                continue;
            }

            float res = area(a, b);
            printf("Площадь = %.4f\n", res);
        } else {
            printf("Неизвестная команда! Доступно: 2 a b  или  0\n");
        }
    }
    return 0;
}