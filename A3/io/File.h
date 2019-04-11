#ifndef FILE_H
#define FILE_H

#include "vdiskAPI.h"
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#define MAX_INODES 256
#define MAX_PATH_LEN 128
#define MAX_FILE_PATH_LEN 160
#define ROOT_INODE_BLOCK 10
#define MAX_DIR_ENTRIES 16
#define FILENAME_LEN 31
#define ROOT_DIR_BLOCK 11
#define ROOT_INODE_ID 1

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
    BYTE_t filename [FILENAME_LEN];
    
}dir_entry_t;

typedef struct
{
    dir_entry_t entries [MAX_DIR_ENTRIES];

}dir_t;


void InitLLFS(void);
void close_block(int block);
void open_block(int block);
int find_inode(void);
void close_inode(int inode_num, int block_address);
void open_inode(int inode_num);
int get_inode_address(int inode_num);

int find_block(void);

void print_inode(inode_t* inode);
void make_dir (BYTE_t* path);
void remove_dir (BYTE_t* path);
void make_root_dir (void);
void print_buf(BYTE_t* buffer);
void inode_into_buffer(inode_t* inode, BYTE_t* buffer);
void buffer_into_inode(inode_t* inode, BYTE_t* buffer);
void dir_into_buffer(dir_t* dir, BYTE_t* buffer);
void buffer_into_dir(dir_t* dir, BYTE_t* buffer);
void write_file(BYTE_t* path, FILE* file);
void read_file(BYTE_t* path, FILE* file);
void remove_file(BYTE_t* path);
int block_empty(BYTE_t* buf);
int inode_empty(BYTE_t* buf);
void file_check(void);

#endif