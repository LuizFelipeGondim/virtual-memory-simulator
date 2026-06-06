#include <stdio.h>
#include <stdlib.h>
#include "dense.h"

static DenseTable table;


//Complexidade: O(n) no tempo, O(n) na memória.
void create_dense_table(unsigned total_pages) {
    table.total_pages = total_pages;
    table.entries = (int *)malloc(total_pages * sizeof(int));
    if (!table.entries) {
        fprintf(stderr, "Erro: falha ao alocar tabela densa (%u páginas).\n", total_pages);
        exit(EXIT_FAILURE);
    }
    for (unsigned i = 0; i < total_pages; i++) {
        table.entries[i] = -1;
    }
}

//Consulta O(1)
int lookup_dense(unsigned virtual_page) {
    if (virtual_page >= table.total_pages) return -1;
    return table.entries[virtual_page];
}


//página virtual → frame físico.
void insert_dense(unsigned virtual_page, int frame) {
    if (virtual_page < table.total_pages) {
        table.entries[virtual_page] = frame;
    }
}


//Invalida a entrada (remove o mapeamento).
void invalidate_dense(unsigned virtual_page) {
    if (virtual_page < table.total_pages) {
        table.entries[virtual_page] = -1;
    }
}

void free_dense_table() {
    if (table.entries) {
        free(table.entries);
        table.entries = NULL;
        table.total_pages = 0;
    }
}


//Retorna o custo de memória em bytes.
size_t memory_usage_dense() {
    return (size_t)table.total_pages * sizeof(int);
}
