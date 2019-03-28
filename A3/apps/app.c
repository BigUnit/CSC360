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
    printf("%d\n",find_inode());
    close_inode(1);
    printf("%d\n",find_inode());
    close_inode(2);
    printf("%d\n",find_inode());
    close_inode(3);
    printf("%d\n",find_inode());
    close_inode(4);
    printf("%d\n",find_inode());
    close_inode(5);
    printf("%d\n",find_inode());
    close_inode(6);
    printf("%d\n",find_inode());

/*
    inode_t *in = (inode_t*)calloc(1,sizeof(inode_t));
    inode_t *out = (inode_t*)calloc(1,sizeof(inode_t));

    BYTE_t *catch = (BYTE_t*)calloc(32,sizeof(BYTE_t));

    unsigned long long test = 0xFFFFFFFFFFFFFFFF;

    in->size = test;
    in->flags = test;
    for(int i = 0; i<10; i++){ in->blocks [i] = test; };
    in->single_ind = test;
    in->double_ind = test;

    //print_inode(in);

    inode_into_buffer(in,catch);
    buffer_into_inode(out,catch);
    */

    return 0;
}