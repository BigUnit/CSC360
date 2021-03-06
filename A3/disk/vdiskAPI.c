#include "vdiskAPI.h"

int read_block(int block, BYTE_t* buf){
   if(block > NUM_BLOCKS || block < 0){return -1;}
   
    FILE* disk = fopen(VDISK,"rb");
    assert(disk);

    fseek(disk,block*BLOCK_SIZE,SEEK_SET);
    fread(buf,BLOCK_SIZE,1,disk); //allocate buffer outside

    assert(!fclose(disk));
    return 0;

}

int write_block(int block, BYTE_t* data){
   if(block > NUM_BLOCKS || block < 0){return -1;}

    FILE* disk = fopen(VDISK,"rb+");

    assert(disk);

    fseek(disk,block*BLOCK_SIZE,SEEK_SET);
    fwrite(data,BLOCK_SIZE,1,disk); //data should be allocated to block size already

    
    assert(!fclose(disk));
    return 0;
}

void create_disk(void){ 
    FILE* disk = fopen(VDISK,"wb+");
    assert(disk);
    assert(!fclose(disk));
    } // creates disk 