#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE_NUMBER 4096

char** split_by_comma(const char* input, size_t* count) {
    int delimiters = 0;
    for (const char* p = input; *p; ++p) {
        if (*p == ','){
            delimiters++;
        }

    }
    *count = delimiters + 1;

    // 2. Выделяем массив указателей
    char** result = malloc(*count * sizeof(char*));

    const char* start = input;
    for (int i = 0; i < *count; ++i) {
        const char* end = strchr(start, ',');
        size_t len = end ? (size_t)(end - start) : strlen(start);

        result[i] = malloc(len + 1);
        
        memcpy(result[i], start, len);
        result[i][len] = '\0';

        start = end ? end + 1 : NULL;
    }

    return result;
}

void free_split(char** arr, size_t count) {
    if (arr) {
        for (int i = 0; i < count; ++i) free(arr[i]);
        free(arr);
    }
}

struct Cell{
    int row_num;
    char* col_name;
    //char* expression;
    int value;
    char flag_solved;
    char* arg1;
    char* arg2;
    char op;
};

struct row{
    int num;
    //struct Cell [<количество столбцов, задается в main (считывается из файла)>];
     struct Cell* cells;
};
   struct row table[MAX_LINE_NUMBER] = {0};

int main(int argc, char *argv[]) {
    //opening file and parsing
 
    int row_idx = 0;

    FILE *file = fopen(argv[1], "r");
  
    char *line = NULL;      // указатель на буфер строки
    size_t len = 0;         // размер буфера (0 для начального выделения)
    size_t nread;          // количество прочитанных байт
    size_t col_numbers; //numbers of colomns
    getline(&line, &len, file);
    char** colomn_titels = split_by_comma(line, &col_numbers);
 while ((nread = getline(&line, &len, file)) != -1) {
        // 1. Отрезаем перевод строки
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue; // пропускаем пустые строки

        // 2. Первый токен всегда номер строки
        char* token = strtok(line, ",");
        if (!token) continue;

        int row_num = atoi(token);
        if (row_num < 0 || row_num >= MAX_LINE_NUMBER) {
            fprintf(stderr, "Warning: row %d out of bounds, skipped.\n", row_num);
            continue;
        }

        // 3. Если строка встречается впервые, выделяем память под ячейки
        if (table[row_num].cells == NULL) {
            table[row_num].num = row_num;
            table[row_num].cells = calloc(col_numbers, sizeof(struct Cell));
            
            // Базовая инициализация ячеек
            for (size_t c = 0; c < col_numbers; c++) {
                table[row_num].cells[c].flag_solved = 0;
                table[row_num].cells[c].value = 0;
                table[row_num].cells[c].op = 0;
                table[row_num].cells[c].arg1 = NULL;
                table[row_num].cells[c].arg2 = NULL;
                
                // Генерируем имя столбца (Col0, Col1, ...)
                char cname[16];
                snprintf(cname, sizeof(cname), "Col%zu", c);
                table[row_num].cells[c].col_name = strdup(cname);
            }
        }

        // 4. Парсим данные столбцов
        for (size_t col = 0; col < col_numbers; col++) {
            token = strtok(NULL, ",");
            if (!token) break; // в файле может быть меньше столбцов

            // Убираем ведущие/ведомые пробелы
            while (*token == ' ') token++;
            char* end = token + strlen(token) - 1;
            while (end > token && *end == ' ') *end-- = '\0';

            struct Cell* cell = &table[row_num].cells[col];

            if (token[0] == '=') {
                // === ФОРМУЛА ===
                cell->flag_solved = 0;
                char* expr = token + 1; // пропускаем '='
                char* op_ptr = strpbrk(expr, "+-*/");

                if (op_ptr) {
                    cell->op = *op_ptr;
                    size_t len1 = op_ptr - expr;
                    cell->arg1 = malloc(len1 + 1);
                    strncpy(cell->arg1, expr, len1);
                    cell->arg1[len1] = '\0';
                    cell->arg2 = strdup(op_ptr + 1);
                } else {
                    // Формула без оператора (напр. =A1)
                    cell->arg1 = strdup(expr);
                    cell->arg2 = NULL;
                    cell->op = 0;
                }
            } else {
                // === ЧИСЛО ===
                cell->flag_solved = 1;
                cell->value = atoi(token);
                // Очищаем поля формулы, если ячейка перезаписывается
                free(cell->arg1); free(cell->arg2);
                cell->arg1 = NULL;
                cell->arg2 = NULL;
                cell->op = 0;
            }
        }
    }
    
    print_table(col_numbers-1);
    free(line);
    fclose(file);
    return EXIT_SUCCESS;
}