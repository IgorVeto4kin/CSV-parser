#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

typedef struct {
    char* expression; // NULL, если поле не начинается с '='
    int value;        // 0, если поле является числом
} Cell;

int try_parse_int(const char* str, size_t len, int* out) {
    if (len == 0) return 0;
    
    // Быстрая проверка: первый символ должен быть цифрой или знаком '-'
    if (!isdigit((unsigned char)str[0]) && str[0] != '-') return 0;

    char* endptr;
    errno = 0;
    long val = strtol(str, &endptr, 10);
    
    // Успех, если: нет ошибки переполнения И consumed вся строка
    if (errno == 0 && endptr == str + len) {
        *out = (int)val;
        return 1;
    }
    return 0;
}

/* 2. Парсинг одного поля в структуру Cell */
void parse_field(const char* field, size_t len, Cell* cell) {
    cell->expression = NULL;
    cell->value = 0;

    if (len == 0) return; // пустое поле между запятыми

    if (field[0] == '=') {
        // Формула: выделяем память и копируем
        cell->expression = malloc(len + 1);
        if (cell->expression) {
            memcpy(cell->expression, field, len);
            cell->expression[len] = '\0';
        }
    } else {
        int num;
        if (try_parse_int(field, len, &num)) {
            cell->value = num;
        } else {
            // Не число и не формула -> сохраняем как строку (fallback)
            cell->expression = malloc(len + 1);
            if (cell->expression) {
                memcpy(cell->expression, field, len);
                cell->expression[len] = '\0';
            }
        }
    }
}

/* 3. Вспомогательная: освобождение ресурсов Cell */
void free_cell(Cell* cell) {
    free(cell->expression);
    cell->expression = NULL;
}

int main(void) {
    FILE* fp = fopen("data.csv", "r");
    
    char* line = NULL;
    size_t cap = 0;
    size_t nread;

    // Читаем построчно
    while ((nread = getline(&line, &cap, fp)) != -1) {
        // Удаляем \n или \r\n
        line[strcspn(line, "\n\r")] = '\0';
        if (line[0] == '\0') continue; // пропуск пустых строк

        char* ptr = line;
        int field_idx = 0;

        // Разбиваем строку по запятым
        while (*ptr != '\0') {
            char* field_start = ptr;
            char* comma = strchr(ptr, ',');
            size_t field_len;

            if (comma) {
                *comma = '\0';          // временно обрезаем поле нулём
                ptr = comma + 1;        // двигаемся после запятой
                field_len = (size_t)(comma - field_start);
            } else {
                field_len = strlen(field_start);
                ptr += field_len;       // конец строки
            }

            // Парсим текущее поле
            Cell cell = {0};
            parse_field(field_start, field_len, &cell);

            // Демонстрация результата
            printf("Поле %2d: ", field_idx);
            if (cell.expression) {
                printf("expr=\"%s\"\n", cell.expression);
            } else {
                printf("value=%d\n", cell.value);
            }

            // Освобождаем память, если выделяли
            free_cell(&cell);
            field_idx++;
        }
        printf("---\n");
    }

    free(line); // getline выделял память под буфер
    fclose(fp);
    return EXIT_SUCCESS;
}