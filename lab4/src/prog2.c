#include "contract.h"

#include <ctype.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static area_func current_area = NULL;
static void* current_lib = NULL;

static float stub(float a, float b) {
    (void)a;
    (void)b;
    fprintf(stderr, "Ошибка: библиотека не загружена!\n");
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
        current_area = stub;
        return 0;
    }

    // Сброс ошибок
    dlerror();

    current_area = (area_func)dlsym(current_lib, "area");
    char* err = dlerror();
    if (err) {
        fprintf(stderr, "Символ 'area' не найден: %s\n", err);
        dlclose(current_lib);
        current_lib = NULL;
        current_area = stub;
        return 0;
    }

    printf("Успешно загружена библиотека: %s\n", path);
    current_area = (area_func)dlsym(current_lib, "area"); // уже проверено выше
    return 1;
}

int main(int argc, char** argv) {
    printf("Реализация 2 Динамическая загрузка (runtime)\n");

    current_area = stub;

    if (argc > 1) {
        load_lib(argv[1]);
    } else {
        printf("Для запуска без пути к библиотеке используйте опцию 1\n");
    }

    char input[512];
    int cmd;
    float a, b;

    while (1) {
        printf("\n> ");
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
            char* p = input;
            while (*p && !isspace(*p))
                p++;
            while (*p && isspace(*p))
                p++;
            if (!*p) {
                printf("Укажите путь к .so\n");
                continue;
            }
            load_lib(p);
        } else if (cmd == 2) {
            if (sscanf(input, "%d %f %f", &cmd, &a, &b) != 3 || a <= 0 || b <= 0) {
                printf("Формат: 2 <a> <b>  (a,b > 0)\n");
                continue;
            }
            float res = current_area(a, b);
            printf("Площадь = %.4f\n", res);
        } else {
            printf("Команды: 1 <путь>      2 <a> <b>      0\n");
        }
    }

    if (current_lib)
        dlclose(current_lib);
    return 0;
}