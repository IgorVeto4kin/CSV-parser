#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** split_by_comma(const char* input, int* count) {

    // 1. Считаем количество разделителей
    int delimiters = 0;
    for (const char* p = input; *p; ++p) {
        if (*p == ',') delimiters++;
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

/**
 * Освобождает память, выделенную split_by_comma
 */
void free_split(char** arr, int count) {
    if (arr) {
        for (int i = 0; i < count; ++i) free(arr[i]);
        free(arr);
    }
}

// 🔍 Пример использования
int main(void) {
    const char* str = ",A,B,Cell";
    int count = 0;

    char** parts = split_by_comma(str, &count);
 

    for (int i = 0; i < count; ++i) {
        printf("Part %d: \"%s\"\n", i, parts[i]);
    }

    free_split(parts, count);
    return 0;
}