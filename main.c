#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "simulator.h"

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <algorithm> <file.log> <page_size_kb> <mem_size_kb>\n", argv[0]);
        return 1;
    }

    char *algorithm = argv[1];
    char *filename = argv[2];
    unsigned page_size = atoi(argv[3]);
    unsigned mem_size = atoi(argv[4]);

    srand(time(NULL));

    // Calcula o shift 's' baseado no tamanho da página em bytes
    unsigned tmp = page_size * 1024;
    unsigned s = 0;
    while (tmp > 1) {
        tmp = tmp >> 1;
        s++;
    }

    unsigned num_frames = mem_size / page_size;
    init_memory(num_frames);

    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        free_memory();
        return 1;
    }

    unsigned address;
    char rw;

    while (fscanf(file, "%x %c", &address, &rw) != EOF) {
        unsigned page = address >> s;
        process_access(page, rw, algorithm);
    }

    fclose(file);

    print_report(algorithm, filename, page_size, mem_size);
    free_memory();

    return 0;
}