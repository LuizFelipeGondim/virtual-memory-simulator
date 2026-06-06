#ifndef INVERTED_H
#define INVERTED_H

#include <stddef.h>

/*
 * Tabela Invertida
 *
 * Tamanho fixo de num_frames entradas (uma por frame físico).
 * Hash: virtual_page % num_frames
 * Colisões: encadeamento externo (lista ligada).
 *
 * Vantagem: uso de memória proporcional ao número de frames,
 * independente do tamanho do espaço de endereçamento virtual.
 */
typedef struct InvertedEntry {
    unsigned virtual_page;
    int      frame;
    int      valid;
    struct InvertedEntry *next;  /* próximo nó na cadeia de colisão */
} InvertedEntry;

typedef struct {
    InvertedEntry *buckets;  /* vetor de num_frames entradas (buckets)       */
    unsigned       num_frames;
    unsigned       collision_nodes; /* nós extras alocados por colisão       */
} InvertedTable;

void   create_inverted_table(unsigned num_frames);
int    lookup_inverted(unsigned virtual_page);
void   insert_inverted(unsigned virtual_page, int frame);
void   invalidate_inverted(unsigned virtual_page);
void   free_inverted_table();
size_t memory_usage_inverted();

#endif