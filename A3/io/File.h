#ifndef FILE_H
#define FILE_H

#include "vdiskAPI.h"
#include <stdint.h>

typedef struct
{
    
    uint32_t size;
    uint32_t flags;
    uint16_t blocks [10];
    uint16_t single_ind;
    uint16_t double_ind;
    
}inode_t;

typedef struct
{
    
    uint8_t inode;
    BYTE_t filename [31];
    
}dir_entry_t;

typedef struct
{
    dir_entry_t entries [16];

}dir_t;




void InitLLFS(void);
void close_block(int block);
void open_block(int block);
int find_inode(void);

int find_block(void);

#endif