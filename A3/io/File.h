#ifndef FILE_H
#define FILE_H

#include "vdiskAPI.h"
#include <stdint.h>
#include <inttypes.h>

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
    uint8_t inode_ID;
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
void close_inode(int inode_num);

int find_block(void);

void print_inode(inode_t* inode);
void print_buf(BYTE_t* buffer);
void inode_into_buffer(inode_t* inode, BYTE_t* buffer);
void buffer_into_inode(inode_t* inode, BYTE_t* buffer);
void dir_into_buffer(dir_t* dir, BYTE_t* buffer);
void buffer_into_dir(dir_t* dir, BYTE_t* buffer);

#endif