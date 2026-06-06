#ifndef THREELEVEL_H
#define THREELEVEL_H

#include <stddef.h>


typedef struct {
    int ***dir;    
    unsigned p1_bits, p2_bits, p3_bits;
    unsigned p1_size, p2_size, p3_size;
    unsigned l2_allocated; 
    unsigned l3_allocated;  
} ThreeLevelTable;

void   create_three_level_table(unsigned page_bits);
int    lookup_three_level(unsigned virtual_page);
void   insert_three_level(unsigned virtual_page, int frame);
void   invalidate_three_level(unsigned virtual_page);
void   free_three_level_table();
size_t memory_usage_three_level();

#endif