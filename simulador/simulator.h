#ifndef SIMULATOR_H
#define SIMULATOR_H

typedef struct {
    unsigned virtual_page;
    int is_occupied;
    int r_bit;
    int m_bit;
    unsigned last_access_time;
} Frame;

typedef struct {
    int frame_allocated;
    int replaced;
    unsigned replaced_vpage;
} AccessResult;

void init_memory(unsigned num_frames);
void free_memory();
AccessResult process_access(unsigned page, char rw, char *algorithm);
void print_report(char *algorithm, char *filename, unsigned page_size, unsigned mem_size);

#endif