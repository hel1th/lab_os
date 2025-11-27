#include "contract.h"

#include <ctype.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static prime_count_func current_prime_count = NULL;
static area_func current_area = NULL;
static void* current_lib = NULL;

// Текущая загруженная библиотека (0 = naive, 1 = opt)
static int current_impl = 0;
static const char* lib_paths[] = {"./libnaive.so", "./libopt.so"};

static int prime_stub(int a, int b) {
    (void)a;
    (void)b;
    fprintf(stderr, "Ошибка: функция prime_count не загружена!\n");
    return 0;
}

static float area_stub(float a, float b) {
    (void)a;
    (void)b;
    fprintf(stderr, "Ошибка: функция area не загружена!\n");
    return 0.0f;
}

int load_lib(const char* path) {
    if (current_lib) {
        dlclose(current_lib);
        current_lib = NULL;
    }

    current_lib = dlopen(path, RTLD_LAZY);
    if (!current_lib) {
        fprintf(stderr, "Не удалось загрузить %s: %s\n", path, dlerror());
        current_prime_count = prime_stub;
        current_area = area_stub;
        return 0;
    }

    // Сброс ошибок
    dlerror();

    // Загрузка prime_count
    current_prime_count = (prime_count_func)dlsym(current_lib, "prime_count");
    char* err = dlerror();
    if (err) {
        fprintf(stderr, "Символ 'prime_count' не найден: %s\n", err);
        current_prime_count = prime_stub;
    }

    // Загрузка area
    current_area = (area_func)dlsym(current_lib, "area");
    err = dlerror();
    if (err) {
        fprintf(stderr, "Символ 'area' не найден: %s\n", err);
        current_area = area_stub;
    }

    printf("Успешно загружена библиотека: %s\n", path);
    return 1;
}

void switch_implementation() {
    current_impl = 1 - current_impl; // Переключение 0/1
    printf("Переключение на реализацию %d (%s)\n", current_impl + 1,
           current_impl == 0 ? "наивная" : "оптимизированная");
    load_lib(lib_paths[current_impl]);
}

int main(int argc, char** argv) {
    printf("Программа 2: Динамическая загрузка (runtime)\n");
    printf("Команды:\n");
    printf("  0          - переключить реализацию\n");
    printf("  1 <a> <b>  - подсчет простых чисел на отрезке [a, b]\n");
    printf("  2 <a> <b>  - вычисление площади (a, b - стороны)\n\n");

    current_prime_count = prime_stub;
    current_area = area_stub;

    // Первая библиотека по дефолту
    if (argc > 1) {
        load_lib(argv[1]);
    } else {
        load_lib(lib_paths[0]);
    }

    char input[512];
    int cmd, ia, ib;
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
            // Переключение реализации
            switch_implementation();
        } else if (cmd == 1) {
            // Подсчет простых чисел
            if (sscanf(input, "%d %d %d", &cmd, &ia, &ib) != 3) {
                printf("Формат: 1 <a> <b>\n");
                continue;
            }
            int result = current_prime_count(ia, ib);
            printf("Количество простых чисел на [%d, %d]: %d\n", ia, ib, result);
        } else if (cmd == 2) {
            // Вычисление площади
            if (sscanf(input, "%d %f %f", &cmd, &fa, &fb) != 3 || fa <= 0 || fb <= 0) {
                printf("Формат: 2 <a> <b> (a, b > 0)\n");
                continue;
            }
            float result = current_area(fa, fb);
            printf("Площадь: %.4f\n", result);
        } else {
            printf("Неизвестная команда\n");
        }
    }

    if (current_lib)
        dlclose(current_lib);

    return 0;
}