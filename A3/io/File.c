#include "File.h"

void InitLLFS(void){

    create_disk();

    BYTE_t* buffer = (BYTE_t *)calloc(BLOCK_SIZE, sizeof(BYTE_t));
    
    for(int i=0;i<NUM_BLOCKS;i++){ write_block(i,buffer); } //set everything to 0;

    free(buffer);

    BYTE_t* superblock = (BYTE_t *)calloc(BLOCK_SIZE, sizeof(BYTE_t));

    superblock [0]  = 0x53;
    superblock [1]  = 0x48;
    superblock [2]  = 0x49;
    superblock [3]  = 0x44; //magic #
    
    superblock [4]  = 0x00; 
    superblock [5]  = 0x00; 
    superblock [6]  = 0x10; 
    superblock [7]  = 0x00; // 4096 blocks
     
    superblock [8]  = 0x00; 
    superblock [9]  = 0x00; 
    superblock [10] = 0x02; 
    superblock [11] = 0x00; //max 512 inodes

  
    write_block(0,superblock);

    free(superblock);
    

    BYTE_t* FBV = (BYTE_t *)calloc(BLOCK_SIZE, sizeof(BYTE_t));
    
    FBV[0] = 0b00000000;
    FBV[1] = 0b00111111; // set bits 0-9 to not availible
    for(int i = 2;i<BLOCK_SIZE;i++){
        FBV[i] = 0b11111111;
    }

    write_block(1,FBV);
    free(FBV);

}

void close_block(int block){

    BYTE_t* buf = (BYTE_t *)calloc(BLOCK_SIZE, sizeof(BYTE_t));

    read_block(1,buf);


    BYTE_t old = buf[block/8]; 
    BYTE_t new; 
    BYTE_t mask; 
    int bit_shift = block%8;

   

    mask = ~(0b10000000 >> (bit_shift));

    new = (old & mask);
  

    buf[block/8] = new; 
    write_block(1,buf);
    free(buf);

}

void open_block(int block){

    BYTE_t* buf = (BYTE_t *)calloc(BLOCK_SIZE, sizeof(BYTE_t));

    read_block(1,buf);

    BYTE_t old = buf[block/8]; 
    BYTE_t new; 
    BYTE_t mask; 
    int bit_shift = block%8;



    mask = 0b10000000 >> (bit_shift);

    new = (old | mask);
   

    buf[block/8] = new; 
    write_block(1,buf);
    free(buf);

}

int find_block(){
    BYTE_t* FBV = (BYTE_t*)malloc(BLOCK_SIZE * sizeof(BYTE_t));
    read_block(1,FBV);

    int block_num = -1;

    for(int i = 0;i<BLOCK_SIZE;i++){
        if(FBV[i]){
            if (FBV[i] & 0b10000000){ //isolate value at this position in binary number
                block_num = (8*i);
            } else if (FBV[i] & 0b01000000) {
                block_num = ((8*i) + 1);
            } else if (FBV[i] & 0b00100000) {
                block_num = ((8*i) + 2);
            } else if (FBV[i] & 0b00010000) {
                block_num = ((8*i) + 3);
            } else if (FBV[i] & 0b00001000) {
                block_num = ((8*i) + 4);
            } else if (FBV[i] & 0b00000100) {
                block_num = ((8*i) + 5);
            } else if (FBV[i] & 0b00000010) {
                block_num = ((8*i) + 6);
            } else if (FBV[i] & 0b00000001) {
                block_num = ((8*i) + 7);
            }
            break; 
        }
    }
    
    free(FBV);
    return block_num;
}

