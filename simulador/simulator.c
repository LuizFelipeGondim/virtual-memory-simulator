#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simulator.h"

Frame *physical_memory = NULL;
unsigned total_frames = 0;
unsigned global_time = 0;

unsigned total_accesses = 0;
unsigned page_faults = 0;
unsigned dirty_pages_written = 0;

static unsigned clock_hand = 0;

// Memória física é criada como um vetor de frames
void init_memory(unsigned num_frames) {
    total_frames = num_frames;
    physical_memory = (Frame *)malloc(num_frames * sizeof(Frame));
    
    for (unsigned i = 0; i < num_frames; i++) {
        physical_memory[i].virtual_page = 0;
        physical_memory[i].is_occupied = 0;
        physical_memory[i].r_bit = 0;
        physical_memory[i].m_bit = 0;
        physical_memory[i].last_access_time = 0;
    }
}

void free_memory() {
    if (physical_memory) {
        free(physical_memory);
    }
}

int select_victim(char *algorithm) {
    if (strcmp(algorithm, "lru") == 0) {
        int victim = 0;
        unsigned min_time = physical_memory[0].last_access_time;

        for (unsigned i = 1; i < total_frames; i++) {
            if (physical_memory[i].last_access_time < min_time) {
                min_time = physical_memory[i].last_access_time;
                victim = i;
            }
        }

        return victim;
    } 
    else if (strcmp(algorithm, "clock") == 0) {

        while (1) {
            if (physical_memory[clock_hand].r_bit == 0) {
                int victim = clock_hand;
                clock_hand = (clock_hand + 1) % total_frames;
                return victim;
            } else {
                physical_memory[clock_hand].r_bit = 0;
                clock_hand = (clock_hand + 1) % total_frames;
            }
        }
    } 
    else if (strcmp(algorithm, "nru") == 0) {
        int classes[4] = {-1, -1, -1, -1};

        for (unsigned i = 0; i < total_frames; i++) {
            int r = physical_memory[i].r_bit;
            int m = physical_memory[i].m_bit;
            int class_idx = (r << 1) | m;
            
            if (classes[class_idx] == -1) {
                classes[class_idx] = i;
            }
        }

        for (int i = 0; i < 4; i++) {
            if (classes[i] != -1) {
                return classes[i];
            }
        }

        return 0;
    } 
    else if (strcmp(algorithm, "random") == 0) {
        return random() % total_frames;
    }
    
    fprintf(stderr, "\nErro: Algoritmo de substituicao desconhecido: '%s'\n", algorithm);
    fprintf(stderr, "Use apenas: lru, nru, clock ou random (em letras minusculas).\n\n");
    exit(EXIT_FAILURE);
}

AccessResult process_access(unsigned page, char rw, char *algorithm) {
    AccessResult res = {0, 0, 0};
    total_accesses++;
    global_time++;

    // Reset global para o algoritmo de NRU funcionar corretamente
    if (strcmp(algorithm, "nru") == 0 && total_accesses % 1000 == 0) {
        for (unsigned i = 0; i < total_frames; i++) {
            physical_memory[i].r_bit = 0;
        }
    }

    // Verificar Page Hit
    for (unsigned i = 0; i < total_frames; i++) {
        if (physical_memory[i].is_occupied && physical_memory[i].virtual_page == page) {
            physical_memory[i].r_bit = 1;
            if (rw == 'W') {
                physical_memory[i].m_bit = 1;
            }
            physical_memory[i].last_access_time = global_time;
            res.frame_allocated = i;
            return res;
        }
    }

    // Page Fault - Procurar por um frame vazio
    page_faults++;
    for (unsigned i = 0; i < total_frames; i++) {
        if (!physical_memory[i].is_occupied) {
            physical_memory[i].is_occupied = 1;
            physical_memory[i].virtual_page = page;
            physical_memory[i].r_bit = 1;
            physical_memory[i].m_bit = (rw == 'W') ? 1 : 0;
            physical_memory[i].last_access_time = global_time;
            res.frame_allocated = i;
            return res;
        }
    }

    // Page Fault - A memória está cheia, é necessário trocar páginas
    int victim_index = select_victim(algorithm);
    
    // Verifica se a página ejetada foi modificada (suja)
    if (physical_memory[victim_index].m_bit == 1) {
        dirty_pages_written++;
    }

    res.replaced = 1;
    res.replaced_vpage = physical_memory[victim_index].virtual_page;

    // Substitui a página velha pela nova
    physical_memory[victim_index].virtual_page = page;
    physical_memory[victim_index].r_bit = 1;
    physical_memory[victim_index].m_bit = (rw == 'W') ? 1 : 0;
    physical_memory[victim_index].last_access_time = global_time;
    
    res.frame_allocated = victim_index;
    return res;
}

void print_report(char *algorithm, char *filename, unsigned page_size, unsigned mem_size) {
    printf("Executando o simulador...\n");
    printf("Arquivo de entrada: %s\n", filename);
    printf("Tamanho da memoria: %u KB\n", mem_size);
    printf("Tamanho das paginas: %u KB\n", page_size);
    printf("Tecnica de reposicao: %s\n", algorithm);
    printf("Paginas lidas: %u\n", page_faults);
    printf("Paginas escritas: %u\n", dirty_pages_written);
}