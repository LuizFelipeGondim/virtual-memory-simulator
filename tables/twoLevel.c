#include <stdio.h>
#include <stdlib.h>
#include "twoLevel.h"

static TwoLevelTable table;

void create_two_level_table(unsigned page_bits) {
    table.p1_bits = page_bits / 2;
    table.p2_bits = page_bits - table.p1_bits;
    table.p1_size = 1u << table.p1_bits;
    table.p2_size = 1u << table.p2_bits;
    table.pages_allocated = 0;

    table.dir = (int **)calloc(table.p1_size, sizeof(int *));
    if (!table.dir) {
        fprintf(stderr, "Erro: falha ao alocar diretório de 2 níveis.\n");
        exit(EXIT_FAILURE);
    }

}

static inline unsigned idx1(unsigned vpage) { return vpage >> table.p2_bits; }
static inline unsigned idx2(unsigned vpage) { return vpage & (table.p2_size - 1); }

int lookup_two_level(unsigned virtual_page) {
    unsigned i1 = idx1(virtual_page);
    unsigned i2 = idx2(virtual_page);
    if (i1 >= table.p1_size) return -1;
    if (table.dir[i1] == NULL) return -1;
    return table.dir[i1][i2];
}

void insert_two_level(unsigned virtual_page, int frame) {
    unsigned i1 = idx1(virtual_page);
    unsigned i2 = idx2(virtual_page);
    if (i1 >= table.p1_size) return;

    if (table.dir[i1] == NULL) {
        table.dir[i1] = (int *)malloc(table.p2_size * sizeof(int));
        if (!table.dir[i1]) {
            fprintf(stderr, "Erro: falha ao alocar tabela de 2º nível.\n");
            exit(EXIT_FAILURE);
        }
    
        for (unsigned k = 0; k < table.p2_size; k++) {
            table.dir[i1][k] = -1;
        }
        table.pages_allocated++;
    }
    table.dir[i1][i2] = frame;
}

void invalidate_two_level(unsigned virtual_page) {
    unsigned i1 = idx1(virtual_page);
    unsigned i2 = idx2(virtual_page);
    if (i1 < table.p1_size && table.dir[i1] != NULL) {
        table.dir[i1][i2] = -1;
    }
}

void free_two_level_table() {
    if (table.dir) {
        for (unsigned i = 0; i < table.p1_size; i++) {
            if (table.dir[i]) {
                free(table.dir[i]);
            }
        }
        free(table.dir);
        table.dir = NULL;
    }
}

size_t memory_usage_two_level() {
    size_t dir_cost  = (size_t)table.p1_size * sizeof(int *);
    size_t page_cost = (size_t)table.pages_allocated * table.p2_size * sizeof(int);
    return dir_cost + page_cost;
}
