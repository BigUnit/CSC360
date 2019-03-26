#ifndef FILE_H
#define FILE_H

#include "vdiskAPI.h"

void init(void);
void close_block(int block);
void open_block(int block);

int find_block(void);

#endif