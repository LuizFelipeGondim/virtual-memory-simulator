#ifndef SIMULATOR_H
#define SIMULATOR_H

typedef struct {
    unsigned virtual_page;
    int is_occupied;
    int r_bit;
    int m_bit;
    unsigned last_access_time;
} Frame;

void init_memory(unsigned num_frames);
void free_memory();
void process_access(unsigned page, char rw, char *algorithm);
void print_report(char *algorithm, char *filename, unsigned page_size, unsigned mem_size);

#endif