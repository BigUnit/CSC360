#ifndef FILE_H
#define FILE_H

#include "vdiskAPI.h"

typedef struct
{
    
    
    
    
    
}inode_t;

typedef struct
{
    dir_entry_t entries [16];

}dir_t;

typedef struct
{
    
    
    
    
    
}dir_entry_t;


void InitLLFS(void);
void close_block(int block);
void open_block(int block);

int find_block(void);

#endif