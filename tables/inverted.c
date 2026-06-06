#include <stdio.h>
#include <stdlib.h>
#include "inverted.h"

static InvertedTable table;

//Função de hash simples
static inline unsigned hash_fn(unsigned virtual_page) {
    return virtual_page % table.num_frames;
}


//Cria a tabela invertida com num_frames buckets (um por frame físico).
void create_inverted_table(unsigned num_frames) {
    table.num_frames      = num_frames;
    table.collision_nodes = 0;

    table.buckets = (InvertedEntry *)calloc(num_frames, sizeof(InvertedEntry));
    if (!table.buckets) {
        fprintf(stderr, "Erro: falha ao alocar tabela invertida.\n");
        exit(EXIT_FAILURE);
    }
    //calloc zera tudo: valid=0, next=NULL, garantindo entradas inválidas
}

//Busca a página virtual na tabela usando hash + encadeamento.
//Retorna o frame físico mapeado ou -1 se não encontrado.
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


//Insere ou atualiza o mapeamento virtual_page → frame.
void insert_inverted(unsigned virtual_page, int frame) {
    unsigned bucket = hash_fn(virtual_page);
    InvertedEntry *entry = &table.buckets[bucket];
    InvertedEntry *prev  = NULL;

    //Procurar entrada existente com a mesma página virtual
    while (entry != NULL) {
        if (entry->valid && entry->virtual_page == virtual_page) {
            entry->frame = frame;
            return;
        }
        //Aproveitar slot inválido no bucket principal
        if (!entry->valid && prev == NULL) {
            entry->virtual_page = virtual_page;
            entry->frame        = frame;
            entry->valid        = 1;
            return;
        }
        prev  = entry;
        entry = entry->next;
    }

    //Não encontrou: encadear novo nó
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

//Invalida a entrada correspondente à página virtual.
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

//Libera toda a memória da tabela, incluindo os nós de colisão encadeados.
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
