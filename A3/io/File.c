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

void close_inode(int inode_num){
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    
    read_block(((inode_num/256)+2),buf);

    for(int i = 0;i<2;i++){
        buf[(inode_num*2)-(512*(inode_num/256) + i)] = 0b11111111;
    }

    write_block((inode_num/256)+2,buf);
    free(buf);
}

int find_inode(){
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    
    int i,j;

    for(i = 0;i<2;i++){
        read_block((2+i),buf);
     
        for(j = 0; j < BLOCK_SIZE; j+=2){
            if(i==0 && j==0){ continue; }
            if(buf[j] == 0b00000000 && buf[(j+1)] == 0b00000000){ return ((256*i) + (j/2)); }
        }
    }

   
    free(buf);
}

void inode_into_buffer(inode_t* inode, BYTE_t* buffer){

    buffer[0] = (inode->size & 0xFF000000) >> 24;
    buffer[1] = (inode->size & 0x00FF0000) >> 16;
    buffer[2] = (inode->size & 0x0000FF00) >> 8;
    buffer[3] = (inode->size & 0x000000FF);

    buffer[4] = (inode->flags & 0xFF000000) >> 24;
    buffer[5] = (inode->flags & 0x00FF0000) >> 16;
    buffer[6] = (inode->flags & 0x0000FF00) >> 8;
    buffer[7] = (inode->flags & 0x000000FF);

    int cur = 8;

    for(int i = 0; i<10; i++){
        buffer[2*i + cur] = (inode->blocks[i] & 0xFF00) >> 8; 
        buffer[2*i + (cur+1)] = (inode->blocks[i] & 0x00FF); 
    }

    buffer[28] = (inode->single_ind & 0xFF00) >> 8;
    buffer[29] = (inode->single_ind & 0x00FF);
    buffer[30] = (inode->double_ind & 0xFF00) >> 8;
    buffer[31] = (inode->double_ind & 0x00FF);


    for(int i = 32; i<BLOCK_SIZE; i++){
        buffer[i] = 0x00000000;
    }
/*
    print_inode(inode);
    printf("****\n");
    print_buf(buffer);
*/

}

void buffer_into_inode(inode_t* inode, BYTE_t* buffer){

    inode->size =   (buffer[0]<< 24) | (buffer[1]<< 16) | (buffer[2]<< 8) | (buffer[3]) ;
    inode->flags =  (buffer[4]<< 24) | (buffer[5]<< 16) | (buffer[6]<< 8) | (buffer[7]) ;

    int cur = 8;

    for(int i = 0; i<10; i++){ inode->blocks[i] =   (buffer[2*i + cur]<< 8) | (buffer[2*i + (cur+1)]) ; }

    inode->single_ind = ( ( buffer[28]<< 8 ) | (buffer[29]) );
    inode->double_ind = ( ( buffer[30]<< 8 ) | (buffer[31]) );

/*
    print_buf(buffer);
    printf("****\n");
    print_inode(inode);
  */  

}

void dir_into_buffer(dir_t* dir, BYTE_t* buffer){
    
   int dir_entry_size = sizeof(dir_entry_t);

    for(int i = 0;i<16;i++){
        buffer[(i*dir_entry_size)] = dir->entries[i].inode_ID;
        
            for(int j = 1; j<32; j++){
                buffer[ j+ (i*dir_entry_size) ] = dir->entries[i].filename[j];
            }
    }
}

void buffer_into_dir(dir_t* dir, BYTE_t* buffer){
    
    int dir_entry_size = sizeof(dir_entry_t);

    for(int i = 0; i<16; i++){
        dir->entries[i].inode_ID = buffer[(i*dir_entry_size)];

        for(int j = 1; j<32; j++){
            dir->entries[i].filename[j] = buffer[ j+ (i*dir_entry_size) ] ;
        }
    }

}

void make_dir (){
    
}

/*
void print_inode(inode_t* inode){

    printf("%" PRIu32 "\n",inode->size);
    printf("%" PRIu32 "\n",inode->flags);

    for(int i = 0;i<10;i++){
        printf("%" PRIu16 " ",inode->blocks[i]);
    }

    printf("\n");

    printf("%" PRIu16 "\n",inode->single_ind);
    printf("%" PRIu16 "\n",inode->double_ind);

} //testing code

void print_buf(BYTE_t* buffer){

    
    printf("%u \n", ((buffer[0]<< 24) | (buffer[1]<< 16) | (buffer[2]<< 8) | (buffer[3])) );
    

    printf("%u \n", ((buffer[4]<< 24) | (buffer[5]<< 16) | (buffer[6]<< 8) | (buffer[7])) );

    
    BYTE_t temp [10];
    int cur = 8;

    for(int i = 0; i<10; i++){ 
        temp[i] =   (buffer[2*i + cur]<< 8) | (buffer[2*i + (cur+1)]);
    }

    for(int i =0; i<10;i++){
        printf("%u ",temp[i]);
    }

    printf("\n");

    printf("%u \n", (  (buffer[28]<< 8 ) | (buffer[29]) ) );
    printf("%u \n", (  (buffer[30]<< 8 ) | (buffer[31]) ) );

} //testing code

*/