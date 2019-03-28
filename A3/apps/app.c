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
*/

    inode_t *in = (inode_t*)calloc(1,sizeof(inode_t));
   // inode_t *out = (inode_t*)calloc(1,sizeof(inode_t));

    BYTE_t *catch = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));

    //uint32_t test = 0x11111111;
    //uint16_t test1 = 0xFF;

    unsigned long long test    = 0xFFFFFFFFFFFFFFFF; 

    in->size = 0;
    in->flags = 1;
    //for(int i = 0; i<10; i++){ in->blocks [i] = 11 + i; };
    in->blocks [0] = 14;
    in->single_ind = 0;
    in->double_ind = 0;

    dir_t* dir = (dir_t*)calloc(1,sizeof(dir_t));
    
    dir->entries[0].inode_ID = 69;
    dir->entries[0].filename[0] = 'F';
    dir->entries[0].filename[1] = 'U';
    dir->entries[0].filename[2] = 'C';
    dir->entries[0].filename[3] = 'K';
    //dir->entries[0].filename[4] = '\0';

        printf("%0x \n",dir->entries[0].inode_ID);

       for(int i = 0; i<10; i++){
        printf("%0x \n",dir->entries[0].filename[i]);
        
    }

 BYTE_t* buf = (BYTE_t *)calloc(BLOCK_SIZE, sizeof(BYTE_t));;
   // buf[0] = 'F';
   // buf[1] = 'U';
   /// buf[2] = 'C';
   // buf[3] = 'K';
/*
    for(int i = 0; i<10; i++){
        //printf("%0x ",in->blocks [i]);
        write_block(in->blocks[i],buf);
    }
*/
   

    //write_block(0,buf);

    //print_inode(in);

    inode_into_buffer(in,catch);
    //buffer_into_inode(out,catch);
    write_block(10,catch);

    dir_into_buffer(dir,buf);
    write_block(14,buf);
    

    return 0;
}