#include <stdio.h>
#include <stdlib.h>
#include "inverted.h"

static InvertedTable table;

static inline unsigned hash_fn(unsigned virtual_page) {
    return virtual_page % table.num_frames;
}

void create_inverted_table(unsigned num_frames) {
    table.num_frames      = num_frames;
    table.collision_nodes = 0;

    table.buckets = (InvertedEntry *)calloc(num_frames, sizeof(InvertedEntry));
    if (!table.buckets) {
        fprintf(stderr, "Erro: falha ao alocar tabela invertida.\n");
        exit(EXIT_FAILURE);
    }
}

//Busca a página virtual na tabela usando hash + encadeamento e retorna o frame físico mapeado ou -1 se não encontrado.
int lookup_inverted(unsigned virtual_page) {
    unsigned bucket = hash_fn(virtual_page);
    InvertedEntry *entry = &table.buckets[bucket];

    while (entry != NULL) {
        if (entry->valid && entry->virtual_page == virtual_page) {
            return entry->frame;
        }
        entry = entry->next;
    }
    return -1;
}

void insert_inverted(unsigned virtual_page, int frame) {
    unsigned bucket = hash_fn(virtual_page);
    InvertedEntry *entry = &table.buckets[bucket];
    InvertedEntry *prev  = NULL;
    InvertedEntry *reusable_slot = NULL;

    // Procurar entrada existente ou um slot inválido para reaproveitar
    while (entry != NULL) {
        if (entry->valid && entry->virtual_page == virtual_page) {
            entry->frame = frame;
            return;
        }

        if (!entry->valid && reusable_slot == NULL) {
            reusable_slot = entry;
        }

        prev  = entry;
        entry = entry->next;
    }

    if (reusable_slot != NULL) {
        reusable_slot->virtual_page = virtual_page;
        reusable_slot->frame        = frame;
        reusable_slot->valid        = 1;
        return;
    }

    InvertedEntry *new_node = (InvertedEntry *)malloc(sizeof(InvertedEntry));
    if (!new_node) {
        fprintf(stderr, "Erro: falha ao alocar nó de colisão.\n");
        exit(EXIT_FAILURE);
    }
    new_node->virtual_page = virtual_page;
    new_node->frame        = frame;
    new_node->valid        = 1;
    new_node->next         = NULL;
    prev->next             = new_node;
    table.collision_nodes++;
}

void invalidate_inverted(unsigned virtual_page) {
    unsigned bucket = hash_fn(virtual_page);
    InvertedEntry *entry = &table.buckets[bucket];

    while (entry != NULL) {
        if (entry->valid && entry->virtual_page == virtual_page) {
            entry->valid = 0;
            return;
        }
        entry = entry->next;
    }
}

void free_inverted_table() {
    if (!table.buckets) return;
    for (unsigned i = 0; i < table.num_frames; i++) {
        InvertedEntry *node = table.buckets[i].next;
        while (node) {
            InvertedEntry *next = node->next;
            free(node);
            node = next;
        }
    }
    free(table.buckets);
    table.buckets = NULL;
}

//Custo de memória:
//Vetor de buckets: num_frames × sizeof(InvertedEntry) (sempre alocado)
//Nós extras por colisão: collision_nodes × sizeof(InvertedEntry)
size_t memory_usage_inverted() {
    size_t base      = (size_t)table.num_frames * sizeof(InvertedEntry);
    size_t collision = (size_t)table.collision_nodes * sizeof(InvertedEntry);
    return base + collision;
}
