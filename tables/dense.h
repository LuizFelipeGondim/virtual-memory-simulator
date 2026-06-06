#ifndef DENSE_H
#define DENSE_H

#include <stddef.h>

typedef struct {
    int *entries;       // entries[vpage] = frame_index, ou -1 se inválida
    unsigned total_pages;
} DenseTable;

void   create_dense_table(unsigned total_pages);
int    lookup_dense(unsigned virtual_page);
void   insert_dense(unsigned virtual_page, int frame);
void   invalidate_dense(unsigned virtual_page);
void   free_dense_table();
size_t memory_usage_dense();

#endif
