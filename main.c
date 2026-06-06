#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulador/simulator.h"
#include "tables/dense.h"
#include "tables/twoLevel.h"
#include "tables/threeLevel.h"
#include "tables/inverted.h"

// Funções Auxiliares de Desempenho e Relatório
static inline long time_ns(struct timespec start, struct timespec end) {
    return (long)(end.tv_sec - start.tv_sec) * 1000000000L
         + (long)(end.tv_nsec - start.tv_nsec);
}

static void print_table_report(const char *table_name, long ns_total, size_t mem_bytes, unsigned long n_accesses) {
    if (n_accesses == 0) n_accesses = 1;
    printf("\n======================================================\n");
    printf(" Avaliação de Desempenho — Tabela: %s\n", table_name);
    printf("======================================================\n");
    printf("Tempo total         : %ld ns\n",  ns_total);
    printf("Memória utilizada   : %zu bytes\n", mem_bytes);
    printf("Acessos simulados   : %lu\n",       n_accesses);
    printf("Tempo médio/acesso  : %ld ns\n",    ns_total / (long)n_accesses);
    printf("======================================================\n");
}

// Gerenciamento das estruturas de tabelas de páginas
static void init_active_tables(int d, int t2, int t3, int inv, unsigned total_pages, unsigned bits, unsigned frames) {
    init_memory(frames);
    if (d)   create_dense_table(total_pages);
    if (t2)  create_two_level_table(bits);
    if (t3)  create_three_level_table(bits);
    if (inv) create_inverted_table(frames);
}

static void free_active_resources(int d, int t2, int t3, int inv) {
    if (d)   free_dense_table();
    if (t2)  free_two_level_table();
    if (t3)  free_three_level_table();
    if (inv) free_inverted_table();
    free_memory();
}

static int lookup_active_table(int d, int t2, int t3, int inv, unsigned vpage) {
    if (d)   return lookup_dense(vpage);
    if (t2)  return lookup_two_level(vpage);
    if (t3)  return lookup_three_level(vpage);
    if (inv) return lookup_inverted(vpage);
    return -1;
}

static void invalidate_active_tables(int d, int t2, int t3, int inv, unsigned replaced_vpage) {
    if (d)   invalidate_dense(replaced_vpage);
    if (t2)  invalidate_two_level(replaced_vpage);
    if (t3)  invalidate_three_level(replaced_vpage);
    if (inv) invalidate_inverted(replaced_vpage);
}

static void insert_and_time_tables(int d, int t2, int t3, int inv, unsigned vpage, int frame,
                                   long *ns_d, long *ns_t2, long *ns_t3, long *ns_inv) {
    struct timespec t0, t1;

    if (d) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        insert_dense(vpage, frame); (void)lookup_dense(vpage);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        *ns_d += time_ns(t0, t1);
    }
    if (t2) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        insert_two_level(vpage, frame); (void)lookup_two_level(vpage);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        *ns_t2 += time_ns(t0, t1);
    }
    if (t3) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        insert_three_level(vpage, frame); (void)lookup_three_level(vpage);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        *ns_t3 += time_ns(t0, t1);
    }
    if (inv) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        insert_inverted(vpage, frame); (void)lookup_inverted(vpage);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        *ns_inv += time_ns(t0, t1);
    }
}

// Debug mode
static void print_debug_step(unsigned long n, unsigned addr, unsigned vpage, char rw, 
                             int frame_antes, AccessResult res, const char *algo) {
    printf("[Acesso #%lu] Endereço: 0x%08X -> Página Virtual: %u | Operação: %c\n", 
           n, addr, vpage, rw);
    if (frame_antes != -1) {
        printf("  -> STATUS: PAGE HIT (Mapeada no Quadro Físico %d)\n", frame_antes);
    } else {
        printf("  -> STATUS: PAGE FAULT\n");
        if (res.replaced) {
            printf("     - Memória cheia. Algoritmo '%s' despejou a vpage %u\n", algo, res.replaced_vpage);
            printf("     - Quadro Físico %d realocado: substituída vpage %u por vpage %u\n", 
                   res.frame_allocated, res.replaced_vpage, vpage);
        } else {
            printf("     - Inicialização da Memória. Alocado no Quadro Livre: %d\n", res.frame_allocated);
        }
    }
    printf("---------------------------------------------------------------------------------\n");
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Uso: %s <algoritmo> <arquivo.log> <tam_pagina_kb> <tam_mem_kb> [tabela] [debug]\n", argv[0]);
        return 1;
    }

    char     *algorithm  = argv[1];
    char     *filename   = argv[2];
    unsigned  page_size  = (unsigned)atoi(argv[3]);
    unsigned  mem_size   = (unsigned)atoi(argv[4]);
    char     *table_name = "all";

    int use_dense, use_two, use_three, use_inv;
    int debug = 0;

    if (argc >= 6 && strcmp(argv[5], "debug") != 0) {
        table_name = argv[5];
        use_dense  = (strcmp(table_name, "densa")     == 0);
        use_two    = (strcmp(table_name, "2niveis")   == 0);
        use_three  = (strcmp(table_name, "3niveis")   == 0);
        use_inv    = (strcmp(table_name, "invertida") == 0);

        if (argc >= 7 && strcmp(argv[6], "debug") == 0) {
            debug = 1;
        }
    } else {
        use_dense = use_two = use_three = use_inv = 1;
        if (argc >= 6 && strcmp(argv[5], "debug") == 0) {
            debug = 1;
        }
    }

    srandom((unsigned)time(NULL));

    // Cálculos de deslocamento e dimensionamento da Memória Virtual
    unsigned tmp = page_size * 1024u;
    unsigned s   = 0;
    while (tmp > 1) { tmp >>= 1; s++; }

    unsigned page_bits   = 32u - s;
    unsigned num_frames  = mem_size / page_size;
    unsigned total_pages = 1u << page_bits;

    init_active_tables(use_dense, use_two, use_three, use_inv, total_pages, page_bits, num_frames);

    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Erro: não foi possível abrir '%s'\n", filename);
        free_active_resources(use_dense, use_two, use_three, use_inv);
        return 1;
    }

    long ns_dense = 0, ns_two = 0, ns_three = 0, ns_inv = 0;
    unsigned long n_accesses = 0;
    unsigned address;
    char rw;

    while (fscanf(file, "%x %c", &address, &rw) == 2) {
        unsigned vpage = address >> s;
        n_accesses++;

        int frame_antes = -1;
        if (debug) {
            frame_antes = lookup_active_table(use_dense, use_two, use_three, use_inv, vpage);
        }

        AccessResult res = process_access(vpage, rw, algorithm);

        if (debug) {
            print_debug_step(n_accesses, address, vpage, rw, frame_antes, res, algorithm);
        }

        if (res.replaced) {
            invalidate_active_tables(use_dense, use_two, use_three, use_inv, res.replaced_vpage);
        }

        insert_and_time_tables(use_dense, use_two, use_three, use_inv, vpage, res.frame_allocated,
                               &ns_dense, &ns_two, &ns_three, &ns_inv);
    }

    fclose(file);

    print_report(algorithm, filename, page_size, mem_size);

    if (use_dense)  print_table_report("Densa", ns_dense, memory_usage_dense(), n_accesses);
    if (use_two)    print_table_report("Hierárquica (2 Níveis)", ns_two, memory_usage_two_level(), n_accesses);
    if (use_three)  print_table_report("Hierárquica (3 Níveis)", ns_three, memory_usage_three_level(), n_accesses);
    if (use_inv)    print_table_report("Invertida", ns_inv, memory_usage_inverted(), n_accesses);

    free_active_resources(use_dense, use_two, use_three, use_inv);

    return 0;
}