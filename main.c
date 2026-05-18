#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include<ctype.h>


#define MAX_LINE_NUMBER 4096

char** split_by_comma(const char* input, size_t* count) {
    int delimiters = 0;
    for (const char* p = input; *p; ++p) {
        if (*p == ','){
            delimiters++;
        }

    }
    *count = delimiters+1;

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
    char visiting;
};

struct row{
    int num;
    //struct Cell [<количество столбцов, задается в main (считывается из файла)>];
     struct Cell* cells;
};

struct row table[MAX_LINE_NUMBER] = {0};
//size_t col_numbers;
char** column_titels;

void print_table(size_t cols) {
    for (size_t i = 0; i < MAX_LINE_NUMBER; i++) {
        if (table[i].cells == NULL) continue; // пропускаем непарсенные строки
         printf("%d,", table[i].num);
        for (size_t j = 1; j < cols; j++) {
            struct Cell* c = &table[i].cells[j];
            
            printf("%d,", c->value);
             
        }
        printf("\n");
    }
}



int find_col_index(const char* name, size_t col_numbers ) {
    for (size_t i = 0; i < col_numbers; i++) {
        if (column_titels[i] && strcmp(column_titels[i], name) == 0) {
            return (int)i;
        }
    }
    return -1; 
}

int parse_cell_ref(const char* ref, int* out_row, char* out_col_name, size_t max_len) {
    if (!ref || !*ref) return -1;

    const char* p = ref;
    while (*p && !isdigit((unsigned char)*p)) p++;
    if (*p == '\0') return -1; // Нет цифр -> невалидная ссылка

    size_t prefix_len = p - ref;
    if (prefix_len == 0 || prefix_len >= max_len) return -1;

    strncpy(out_col_name, ref, prefix_len);
    out_col_name[prefix_len] = '\0';
    *out_row = atoi(p);
    return 0;
}

int solve_cell(int row_idx, size_t col_idx, size_t col_numbers);

int get_value_by_name(const char* ref, int* out_val, size_t col_numbers) {
    if (!ref) return -1;

    if (isdigit((unsigned char)ref[0]) || (ref[0] == '-' && isdigit((unsigned char)ref[1]))) {
        *out_val = atoi(ref);
        return 0;
    }

    int row_num;
    char col_name[64];
    if (parse_cell_ref(ref, &row_num, col_name, sizeof(col_name)) != 0) {
        fprintf(stderr, "Invalid cell name. Aborted\n");
        return -1;
    }

    
    if (row_num < 0 || row_num >= MAX_LINE_NUMBER || !table[row_num].cells) {
        fprintf(stderr, " Row didn't found.Aborted\n");
        return -1;
    }

    int col_idx = find_col_index(col_name, col_numbers);
    if (col_idx < 0 || col_idx >= col_numbers) {
        fprintf(stderr, "Colonm not found. Aborted\n");
        return -1;
    }

    struct Cell* target = &table[row_num].cells[col_idx];

    if (target->flag_solved) {
        *out_val = target->value;
        return 0;
    }

    
    if (target->visiting) {
        fprintf(stderr, " Cycle found. Aborted\n");
        return -2;
    }

    target->visiting = 1;
    int res = solve_cell(row_num, col_idx, col_numbers);
    target->visiting = 0;

    if (res == 0) {
        *out_val = target->value;
        return 0;
    }
    return res;
}

int solve_cell(int row_idx, size_t col_idx, size_t col_numbers) {
    struct Cell* cell = &table[row_idx].cells[col_idx];
    if (cell->flag_solved) return 0;

    int val1 = 0, val2 = 0;

    if (get_value_by_name(cell->arg1, &val1,  col_numbers) != 0) return -1;
    if (get_value_by_name(cell->arg2, &val2,  col_numbers) != 0) return -1;
   
    //printf("solving bitch");
    int result = 0;
    switch (cell->op) {
        case '+': result = val1 + val2; break;
        case '-': result = val1 - val2; break;
        case '*': result = val1 * val2; break;
        case '/':
            if (val2 == 0) {
                fprintf(stderr, " Zero dividing found.Aborted\n");
                return -1;
            }
            result = val1 / val2; break;
    }

    cell->value = result;
    cell->flag_solved = 1;
    return 0;
}

void evaluate_table(size_t col_numbers) {
    
    
    for (size_t i = 0; i < MAX_LINE_NUMBER; i++) {
        if (table[i].cells == NULL) continue;
        for (size_t j = 0; j < col_numbers; j++) {
            
            if (table[i].cells[j].flag_solved) continue; 
            
            int status = solve_cell(i, j, col_numbers);
            
            if (status == -2) {
                printf("Self-reference found. Aborted\n");
                return;
            }
        }
    }
}


int main(int argc, char *argv[]) {
 
    int row_idx = 0;

    FILE *file = fopen(argv[1], "r");
  
    char *line = NULL;      // row buf
    size_t len = 0;         // size
    size_t nread;          
    size_t col_numbers; //numbers of colomns
    getline(&line, &len, file);
    line[strcspn(line, "\r\n")] = '\0';
    column_titels = split_by_comma(line, &col_numbers);
    
    
    while ((nread = getline(&line, &len, file)) != -1) {
        line[strcspn(line, "\r\n")] = '\0';

        char* token = strtok(line, ",");
        if (!token) continue;

        int row_num = atoi(token);
        if (table[row_num].cells == NULL) {
            table[row_num].num = row_num;
            table[row_num].cells = calloc(col_numbers, sizeof(struct Cell));
            
            for (size_t c = 0; c < col_numbers; c++) {
                table[row_num].cells[c].flag_solved = 0;
                table[row_num].cells[c].value = 0;
                table[row_num].cells[c].op = 0;
                table[row_num].cells[c].arg1 = NULL;
                table[row_num].cells[c].arg2 = NULL;
                table[row_num].cells[c].visiting = 0;
                
                char cname[16];
                snprintf(cname, sizeof(cname), "Col%zu", c);
                table[row_num].cells[c].col_name = strdup(cname);
            }
        }
        else{
            printf("Duplicated string numbers, aborted\n");
            return 1;
        }

        for (size_t col = 1; col <= col_numbers; col++) {
            token = strtok(NULL, ",");
            if (!token) break; 

            while (*token == ' ') token++;
            char* end = token + strlen(token) - 1;
            while (end > token && *end == ' ') *end-- = '\0';

            struct Cell* cell = &table[row_num].cells[col];

            if (token[0] == '=') {
                
                cell->flag_solved = 0;
                char* expr = token + 1;
                char* op_ptr = strpbrk(expr, "+-*/");
  
                cell->op = *op_ptr;
                size_t len1 = op_ptr - expr;
                cell->arg1 = malloc(len1 + 1);
                strncpy(cell->arg1, expr, len1);
                cell->arg1[len1] = '\0';
                cell->arg2 = strdup(op_ptr + 1);
                
            }
            else {
                cell->flag_solved = 1;
                cell->value = atoi(token);
                free(cell->arg1); free(cell->arg2);
                cell->arg1 = NULL;
                cell->arg2 = NULL;
                cell->op = 0;
            }
        }
    }
    
    //print_table(col_numbers);
    
    evaluate_table( col_numbers);
    for(int i = 1; i< col_numbers; ++i){
        printf(",%s", column_titels[i]);
    }
    printf("\n");
    print_table(col_numbers);
    free(line);
    fclose(file);
    return EXIT_SUCCESS;
}