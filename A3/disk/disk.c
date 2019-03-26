#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define VDISK "./vdisk"
#define BLOCK_SIZE 512
#define FREE_BLOCK_VECT 512
#define NUM_BLOCKS 4096

void init(){
    FILE* disk = fopen(VDISK,"w");
    assert(disk);

    unsigned char* buffer = (unsigned char *)calloc(BLOCK_SIZE*NUM_BLOCKS, sizeof(unsigned char));

    fwrite(buffer,1,BLOCK_SIZE*NUM_BLOCKS,disk);

    free(buffer);

    unsigned char superblock  [12] = {0x53, 0x48, 0x49, 0x44, //magic #
                                      0x00, 0x00, 0x10, 0x00, // 4096 blocks
                                      0x00, 0x00, 0x02, 0x00}; //max 512 inodes

    fseek(disk, 0, SEEK_SET);
    fwrite(superblock,1,12,disk);
    fseek(disk, 500, SEEK_CUR);

    

    unsigned char free_block [BLOCK_SIZE];
    
    free_block[0] = 0b00000000;
    free_block[1] = 0b00111111; // set bits 0-9 to not availibe
    for(int i = 2;i<BLOCK_SIZE;i++){
        free_block[i] = 0b11111111;
    }

    fwrite(free_block,1,BLOCK_SIZE,disk);

    assert(!fclose(disk));

}

int read_block(int block, unsigned char* buf){
   if(block > NUM_BLOCKS || block < 0){return -1;}
   
    FILE* disk = fopen(VDISK,"r");
    assert(disk);

    fseek(disk,block*BLOCK_SIZE,SEEK_SET);
    fread(buf,BLOCK_SIZE,1,disk); //allocate buffer outside

    assert(!fclose(disk));
    return 0;

}

int write_block(int block, unsigned char* data){
   if(block > NUM_BLOCKS || block < 0){return -1;}
   
    FILE* disk = fopen(VDISK,"r+");
    assert(disk);

    fseek(disk,block*BLOCK_SIZE,SEEK_SET);
    fwrite(data,BLOCK_SIZE,1,disk); //data should be allocated to block size already

    
    assert(!fclose(disk));
    return 0;
}

int find_block(){
    unsigned char* FBV = (unsigned char*)malloc(BLOCK_SIZE * sizeof(unsigned char));
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

void close_block(int block){

    FILE* disk = fopen(VDISK,"r+");
    assert(disk);

    fseek(disk, FREE_BLOCK_VECT + (block/8), SEEK_SET);

    unsigned char old; 
    unsigned char new; 
    unsigned char mask; 
    int bit_shift = block%8;

    fread(&old,1,1,disk);

    mask = ~(0b10000000 >> (bit_shift));

    new = (old & mask);
    fseek(disk, FREE_BLOCK_VECT + (block/8), SEEK_SET);
    fwrite(&new,1,1,disk);

    assert(!fclose(disk));
}

void open_block(int block){

    FILE* disk = fopen(VDISK,"r+");
    assert(disk);

    fseek(disk, FREE_BLOCK_VECT + (block/8), SEEK_SET);

    unsigned char old; 
    unsigned char new; 
    unsigned char mask; 
    int bit_shift = block%8;

    fread(&old,1,1,disk);

    mask = 0b10000000 >> (bit_shift);

    new = (old | mask);
    fseek(disk, FREE_BLOCK_VECT + (block/8), SEEK_SET);
    fwrite(&new,1,1,disk);

    assert(!fclose(disk));
}

int main(){
    
    init();
/*
    unsigned char* skrt = (unsigned char*)malloc(BLOCK_SIZE * sizeof(unsigned char));
    skrt[0] = 'y';
    skrt[1] = 'e';
    skrt[2] = 'e';
    skrt[3] = 't';
*/

   
    //close_block(find_block());
    //printf("%d\n",find_block());
    //open_block(find_block()-1);
    //printf("%d\n",find_block());
 
    //open_block(find_block()-1);
    //printf("%d\n",find_block());
    

    //write_block(write_to,skrt);
    


    return 0;
}