#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simulador/simulator.h"
#include "tables/dense.h"
#include "tables/twoLevel.h"
#include "tables/threeLevel.h"
#include "tables/inverted.h"


static inline long time_ns(struct timespec start, struct timespec end) {
    return (long)(end.tv_sec - start.tv_sec) * 1000000000L
         + (long)(end.tv_nsec - start.tv_nsec);
}

// Relatório de desempenho de uma única tabela
static void print_table_report(const char *table_name,
                                long ns_total,
                                size_t mem_bytes,
                                unsigned long n_accesses)
{
    if (n_accesses == 0) n_accesses = 1;

    printf("\n");
    printf("======================================================\n");
    printf(" Avaliação de Desempenho — Tabela: %s\n", table_name);
    printf("======================================================\n");
    printf("Tempo total         : %ld ns\n",  ns_total);
    printf("Memória utilizada   : %zu bytes\n", mem_bytes);
    printf("Acessos simulados   : %lu\n",       n_accesses);
    printf("Tempo médio/acesso  : %ld ns\n",    ns_total / (long)n_accesses);
    printf("======================================================\n");
}


int main(int argc, char *argv[]) {

    if (argc < 6) {
        fprintf(stderr,
            "Uso: %s <algoritmo> <arquivo.log> <tam_pagina_kb> <tam_mem_kb> <tabela>\n"
            "  <tabela>: densa | 2niveis | 3niveis | invertida\n",
            argv[0]);
        return 1;
    }

    char     *algorithm  = argv[1];
    char     *filename   = argv[2];
    unsigned  page_size  = (unsigned)atoi(argv[3]);   // em KB
    unsigned  mem_size   = (unsigned)atoi(argv[4]);   // em KB
    char     *table_name = argv[5];

    // Valida o nome da tabela
    int use_dense   = strcmp(table_name, "densa")     == 0;
    int use_two     = strcmp(table_name, "2niveis")   == 0;
    int use_three   = strcmp(table_name, "3niveis")   == 0;
    int use_inv     = strcmp(table_name, "invertida") == 0;

    if (!use_dense && !use_two && !use_three && !use_inv) {
        fprintf(stderr,
            "Erro: tabela '%s' inválida. Use: densa | 2niveis | 3niveis | invertida\n",
            table_name);
        return 1;
    }

    srand((unsigned)time(NULL));

    // Calcula shift 's' (bits de deslocamento dentro da página)
    unsigned tmp = page_size * 1024u;
    unsigned s   = 0;
    while (tmp > 1) { tmp >>= 1; s++; }

    unsigned page_bits  = 32u - s;
    unsigned num_frames = mem_size / page_size;
    unsigned total_pages = 1u << page_bits;

    // Inicializa memória física e a tabela escolhida
    init_memory(num_frames);

    if (use_dense)  create_dense_table(total_pages);
    if (use_two)    create_two_level_table(page_bits);
    if (use_three)  create_three_level_table(page_bits);
    if (use_inv)    create_inverted_table(num_frames);

    // Abre arquivo de trace
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Erro: não foi possível abrir '%s'\n", filename);
        if (use_dense) free_dense_table();
        if (use_two)   free_two_level_table();
        if (use_three) free_three_level_table();
        if (use_inv)   free_inverted_table();
        free_memory();
        return 1;
    }

    long ns_total = 0;
    struct timespec t0, t1;
    unsigned long n_accesses = 0;

    unsigned address;
    char rw;

    while (fscanf(file, "%x %c", &address, &rw) == 2) {
        unsigned vpage = address >> s;
        n_accesses++;

        AccessResult res = process_access(vpage, rw, algorithm);

        if (res.replaced) {
            if (use_dense) invalidate_dense(res.replaced_vpage);
            if (use_two)   invalidate_two_level(res.replaced_vpage);
            if (use_three) invalidate_three_level(res.replaced_vpage);
            if (use_inv)   invalidate_inverted(res.replaced_vpage);
        }

        int frame = res.frame_allocated;

        clock_gettime(CLOCK_MONOTONIC, &t0);
        if (use_dense) { insert_dense(vpage, frame);       (void)lookup_dense(vpage);       }
        if (use_two)   { insert_two_level(vpage, frame);   (void)lookup_two_level(vpage);   }
        if (use_three) { insert_three_level(vpage, frame); (void)lookup_three_level(vpage); }
        if (use_inv)   { insert_inverted(vpage, frame);    (void)lookup_inverted(vpage);    }
        clock_gettime(CLOCK_MONOTONIC, &t1);
        ns_total += time_ns(t0, t1);
    }

    fclose(file);

    // Escolhe o uso de memória correto para o relatório
    size_t mem_usage = 0;
    if (use_dense) mem_usage = memory_usage_dense();
    if (use_two)   mem_usage = memory_usage_two_level();
    if (use_three) mem_usage = memory_usage_three_level();
    if (use_inv)   mem_usage = memory_usage_inverted();

    // Relatório do simulador de reposição
    print_report(algorithm, filename, page_size, mem_size);

    // Relatório da tabela escolhida
    print_table_report(table_name, ns_total, mem_usage, n_accesses);

    // Libera recursos
    if (use_dense) free_dense_table();
    if (use_two)   free_two_level_table();
    if (use_three) free_three_level_table();
    if (use_inv)   free_inverted_table();
    free_memory();

    return 0;
}
