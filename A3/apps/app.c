#include "../io/File.h"

int main(){
    InitLLFS();
/*
    unsigned char* buf = (unsigned char *)calloc(BLOCK_SIZE, sizeof(unsigned char));;
    buf[0] = 'F';
    buf[1] = 'U';
    buf[2] = 'C';
    buf[3] = 'K';

    write_block(0,buf);
*/

    printf("%d",find_inode());

    return 0;
}