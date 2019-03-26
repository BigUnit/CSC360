#ifndef vdiskAPI_H
#define vdiskAPI_H

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define VDISK "../disk/vdisk"
#define BLOCK_SIZE 512
#define NUM_BLOCKS 4096

typedef unsigned char BYTE_t;

int read_block(int block, BYTE_t* buf);
int write_block(int block, BYTE_t* data);
void create_disk(void);

#endif 