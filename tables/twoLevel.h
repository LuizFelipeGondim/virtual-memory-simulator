#ifndef TWOLEVEL_H
#define TWOLEVEL_H

#include <stddef.h>

/*
 * Tabela Hierárquica de 2 Níveis
 *
 * O número de bits do número de página (page_bits = 32 - s) é dividido:
 *   p1_bits = page_bits / 2        → índice no diretório (1º nível)
 *   p2_bits = page_bits - p1_bits  → índice na tabela de 2º nível
 *
 * O diretório (nível 1) é sempre alocado com p1_size entradas (ponteiros).
 * As tabelas de 2º nível são alocadas SOB DEMANDA, economizando memória.
 */
typedef struct {
    int    **dir;       /* dir[i1] → tabela de 2º nível ou NULL              */
    unsigned p1_bits;
    unsigned p2_bits;
    unsigned p1_size;   /* 1 << p1_bits */
    unsigned p2_size;   /* 1 << p2_bits */
    unsigned pages_allocated; /* quantidade de tabelas de 2º nível alocadas */
} TwoLevelTable;

void   create_two_level_table(unsigned page_bits);
int    lookup_two_level(unsigned virtual_page);
void   insert_two_level(unsigned virtual_page, int frame);
void   invalidate_two_level(unsigned virtual_page);
void   free_two_level_table();
size_t memory_usage_two_level();

#endif