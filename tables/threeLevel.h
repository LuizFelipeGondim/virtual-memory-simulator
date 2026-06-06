#ifndef THREELEVEL_H
#define THREELEVEL_H

#include <stddef.h>

/*
 * Tabela Hierárquica de 3 Níveis
 *
 * O número de bits do número de página (page_bits = 32 - s) é dividido:
 *   p1_bits = page_bits / 3                    → índice no diretório (1º nível)
 *   p2_bits = page_bits / 3                    → índice na tabela de 2º nível
 *   p3_bits = page_bits - p1_bits - p2_bits    → índice na tabela de 3º nível
 *
 * O diretório (nível 1) é sempre alocado com p1_size entradas (ponteiros).
 * As tabelas de 2º e 3º nível são alocadas SOB DEMANDA, economizando memória.
 */
typedef struct {
    int ***dir;
    unsigned p1_bits, p2_bits, p3_bits;
    unsigned p1_size, p2_size, p3_size;
    unsigned l2_allocated; 
    unsigned l3_allocated;  
} ThreeLevelTable;

void   create_three_level_table(unsigned page_bits);
int    lookup_three_level(unsigned virtual_page);
void   insert_three_level(unsigned virtual_page, int frame);
void   invalidate_three_level(unsigned virtual_page);
void   free_three_level_table();
size_t memory_usage_three_level();

#endif