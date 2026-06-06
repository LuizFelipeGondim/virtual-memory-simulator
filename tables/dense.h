#ifndef DENSE_H
#define DENSE_H

#include <stddef.h>

/*
 * Tabela de Páginas Densa (Convencional)
 *
 * É alocada contiguamente na memória com um tamanho proporcional
 * ao número total de páginas virtuais suportadas. Cada entrada da 
 * tabela corresponde a uma página virtual e contém o número do
 * quadro de memória física (ou -1 se não estiver na memória física).
 */
typedef struct {
    int *entries;
    unsigned total_pages;
} DenseTable;

void   create_dense_table(unsigned total_pages);
int    lookup_dense(unsigned virtual_page);
void   insert_dense(unsigned virtual_page, int frame);
void   invalidate_dense(unsigned virtual_page);
void   free_dense_table();
size_t memory_usage_dense();

#endif
