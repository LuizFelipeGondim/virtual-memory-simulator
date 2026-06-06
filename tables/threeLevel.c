#include <stdio.h>
#include <stdlib.h>
#include "threeLevel.h"

static ThreeLevelTable table;

void create_three_level_table(unsigned page_bits) {
    table.p1_bits = page_bits / 3;
    table.p2_bits = page_bits / 3;
    table.p3_bits = page_bits - table.p1_bits - table.p2_bits;

    table.p1_size = 1u << table.p1_bits;
    table.p2_size = 1u << table.p2_bits;
    table.p3_size = 1u << table.p3_bits;

    table.l2_allocated = 0;
    table.l3_allocated = 0;

    table.dir = (int ***)calloc(table.p1_size, sizeof(int **));
    if (!table.dir) {
        fprintf(stderr, "Erro: falha ao alocar diretório de 3 níveis.\n");
        exit(EXIT_FAILURE);
    }

}

static inline unsigned idx1(unsigned vp) { return vp >> (table.p2_bits + table.p3_bits); }
static inline unsigned idx2(unsigned vp) { return (vp >> table.p3_bits) & (table.p2_size - 1); }
static inline unsigned idx3(unsigned vp) { return vp & (table.p3_size - 1); }

int lookup_three_level(unsigned virtual_page) {
    unsigned i1 = idx1(virtual_page);
    unsigned i2 = idx2(virtual_page);
    unsigned i3 = idx3(virtual_page);
    if (i1 >= table.p1_size)           return -1;
    if (table.dir[i1] == NULL)         return -1;
    if (table.dir[i1][i2] == NULL)     return -1;
    return table.dir[i1][i2][i3];
}

// Insere mapeamento, alocando os níveis 2 e 3 se necessário.
void insert_three_level(unsigned virtual_page, int frame) {
    unsigned i1 = idx1(virtual_page);
    unsigned i2 = idx2(virtual_page);
    unsigned i3 = idx3(virtual_page);
    if (i1 >= table.p1_size) return;

    if (table.dir[i1] == NULL) {
        table.dir[i1] = (int **)calloc(table.p2_size, sizeof(int *));
        if (!table.dir[i1]) {
            fprintf(stderr, "Erro: falha ao alocar nível 2.\n");
            exit(EXIT_FAILURE);
        }
        table.l2_allocated++;
    }

    if (table.dir[i1][i2] == NULL) {
        table.dir[i1][i2] = (int *)malloc(table.p3_size * sizeof(int));
        if (!table.dir[i1][i2]) {
            fprintf(stderr, "Erro: falha ao alocar nível 3.\n");
            exit(EXIT_FAILURE);
        }
        for (unsigned k = 0; k < table.p3_size; k++) {
            table.dir[i1][i2][k] = -1;
        }
        table.l3_allocated++;
    }

    table.dir[i1][i2][i3] = frame;
}

void invalidate_three_level(unsigned virtual_page) {
    unsigned i1 = idx1(virtual_page);
    unsigned i2 = idx2(virtual_page);
    unsigned i3 = idx3(virtual_page);
    if (i1 < table.p1_size &&
        table.dir[i1] != NULL &&
        table.dir[i1][i2] != NULL) {
        table.dir[i1][i2][i3] = -1;
    }
}

void free_three_level_table() {
    if (!table.dir) return;
    for (unsigned i = 0; i < table.p1_size; i++) {
        if (!table.dir[i]) continue;
        for (unsigned j = 0; j < table.p2_size; j++) {
            if (table.dir[i][j]) {
                free(table.dir[i][j]);
            }
        }
        free(table.dir[i]);
    }
    free(table.dir);
    table.dir = NULL;
}

size_t memory_usage_three_level() {
    size_t l1 = (size_t)table.p1_size * sizeof(int **);
    size_t l2 = (size_t)table.l2_allocated * table.p2_size * sizeof(int *);
    size_t l3 = (size_t)table.l3_allocated * table.p3_size * sizeof(int);
    return l1 + l2 + l3;
}
