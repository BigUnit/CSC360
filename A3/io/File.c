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
    superblock [10] = 0x01; 
    superblock [11] = 0x00; //max 256 inodes BEACUSE ONLY 256 UNIQUE INODE ID (uint8_t)

  
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

    make_root_dir();

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

void close_inode(int inode_num, int block_address){
    assert(!(inode_num<0 || inode_num >= MAX_INODES));
    assert(!(block_address<0 || block_address >= NUM_BLOCKS));
    
    
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    
    BYTE_t first = (block_address & 0xFF00) >> 8;
    BYTE_t second = (block_address & 0x00FF);

    //printf("\n*%u ** %u *\n",first,second);

    read_block(((inode_num/256)+2),buf);

    //for(int i = 0;i<2;i++){
        buf[(inode_num*2)-(512*(inode_num/256))] = first;
        buf[(inode_num*2)-(512*(inode_num/256) + 1)] = second;
    //}

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
                buffer[ j + (i*dir_entry_size) ] = dir->entries[i].filename[j-1];
            }
    }
}

void buffer_into_dir(dir_t* dir, BYTE_t* buffer){
    
    int dir_entry_size = sizeof(dir_entry_t);

    for(int i = 0; i<16; i++){
        dir->entries[i].inode_ID = buffer[(i*dir_entry_size)];

        for(int j = 1; j<32; j++){
            dir->entries[i].filename[j-1] = buffer[ j+ (i*dir_entry_size) ] ;
        }
    }

}

int get_inode_address(int inode_num){
    if(inode_num<0 || inode_num >= NUM_BLOCKS){return -1;}
    
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    
    BYTE_t first ;
    BYTE_t second ;

    

    read_block(((inode_num/256)+2),buf);

    //for(int i = 0;i<2;i++){
        first = buf[(inode_num*2)-(512*(inode_num/256))] ;
        second = buf[(inode_num*2)-(512*(inode_num/256) + 1)];
    //}

    //write_block((inode_num/256)+2,buf);
    free(buf);

    int address = (first<<8) | (second) ;
    //printf("\n* %d *\n",address);
    return address;

}

void make_dir (BYTE_t* path){

    inode_t* in = (inode_t*)calloc(1,sizeof(inode_t));
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    dir_t*  parent = (dir_t*)calloc(1,sizeof(dir_t));

    //printf("\n%d\n", find_block());
/*
    int inode_num = find_inode();
    int inode_block = find_block();
    close_block(inode_block);
    close_inode(inode_num, inode_block);
    
    int dir_block = find_block();
    close_block(dir_block);
    
    
    in->size = 0;
    in->flags = 1;
    in->blocks [0] = dir_block;
    in->blocks [1] = dir_block; // root points to self


    dir->entries[0].inode_ID = inode_num;
    dir->entries[0].filename[0] = '~';
    dir->entries[0].filename[1] = '\0';




    inode_into_buffer(in,buf);
    write_block(inode_block,buf);
    dir_into_buffer(dir,buf);
    write_block(dir_block,buf);

*/
    BYTE_t new_path [MAX_PATH_LEN];

    strncpy(new_path,path,MAX_PATH_LEN);

    BYTE_t* tok;
    const char* delim = "/"; 

    tok = strtok(new_path,delim);

    BYTE_t* tokens [4];
    int path_len = 0;
    
  
  while( tok != NULL ) {
     //printf( " %s\n", tok );
      tokens[path_len] = tok; 
      path_len++;
      //printf( " %s\n", tokens[path_len] );
      tok = strtok(NULL, delim);
   }
    int parent_dir_block;
    printf("%d",path_len);
    int parent_inode_ID;
    int parent_inode_address;
    
    read_block(ROOT_INODE_BLOCK,buf);
    buffer_into_inode(in,buf);
    assert(in->flags==1);
    parent_dir_block = in->blocks[0];
    read_block(parent_dir_block,buf);
    buffer_into_dir(parent,buf);

    

    for(int i = 1;i<path_len-1;i++){ // reads thorugh intermediate directories on path
        
        for(int j=2;j<MAX_DIR_ENTRIES;j++){ // first entry in directory is itself, parent is second so we can skip checking those(start at 2)
            if(!(strncmp(tokens[i],parent->entries[j].filename,FILENAME_LEN))){
                parent_inode_ID=parent->entries[j].inode_ID;
                parent_inode_address = get_inode_address(parent_inode_ID);

                read_block(parent_inode_address,buf);
                buffer_into_inode(in,buf);
                assert(in->flags==1);
                parent_dir_block = in->blocks[0];
                read_block(parent_dir_block,buf);
                buffer_into_dir(parent,buf);
                break;
            }
   
        }

    }


    inode_t* new_dir_inode = (inode_t*)calloc(1,sizeof(inode_t));
   //BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    dir_t*  new_dir = (dir_t*)calloc(1,sizeof(dir_t));
    
    
    int inode_num = find_inode();
    int inode_block = find_block();
    close_block(inode_block);
    close_inode(inode_num, inode_block);


    int new_dir_block = find_block();
    close_block(new_dir_block);

    new_dir_inode->size = 0;
    new_dir_inode->flags = 1;
    new_dir_inode->blocks [0] = new_dir_block;

/*
    BYTE_t* temp_tok = (BYTE_t*)malloc(FILENAME_LEN* sizeof(BYTE_t));
    
    strncpy(temp_tok, tokens[path_len], FILENAME_LEN);
   // strcat(temp_tok,"\0");
*/ 
//THIS SHIT BROKE, PRINTS OUT GARBAGE RATHER THAN NAME LIKELY PROBABLY SOMETHING WITH NULL TERMINATOR

    new_dir->entries[0].inode_ID = (uint8_t)inode_num;
    strncpy(new_dir->entries[0].filename, temp_tok, FILENAME_LEN+1); //first entry is itself
    new_dir->entries[1].inode_ID = parent->entries[0].inode_ID;
    strncpy(new_dir->entries[1].filename, parent->entries[0].filename, FILENAME_LEN); // second entry is parent directory

    int dir_opening;

    for(dir_opening=0;dir_opening<MAX_DIR_ENTRIES;dir_opening++){
        if(!strcmp(parent->entries[dir_opening].filename,"") ){break;};
    }

    //printf("%d",dir_opening);

    assert(dir_opening<MAX_DIR_ENTRIES);


    parent->entries[dir_opening].inode_ID = (uint8_t)inode_num;
    strncpy(parent->entries[dir_opening].filename, temp_tok , FILENAME_LEN); 

    dir_into_buffer(parent,buf);
    write_block(parent_dir_block,buf);
    inode_into_buffer(new_dir_inode,buf);
    write_block(inode_block,buf);
    dir_into_buffer(new_dir,buf);
    write_block(new_dir_block,buf);

    
    for(int i=0;i<path_len;i++){    
        printf( " %s\n", tokens[i] );
    }

    free(new_dir_inode);
    free(new_dir);
    free(in);
    free(parent);
    free(buf);
    free(temp_tok);

}

void make_root_dir (){
    inode_t* in = (inode_t*)calloc(1,sizeof(inode_t));
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    dir_t*  root = (dir_t*)calloc(1,sizeof(dir_t));

    //printf("\n%d\n", find_block());

    int inode_num = find_inode();
    int inode_block = find_block();
    close_block(inode_block);
    close_inode(inode_num, inode_block);
    //get_inode_address(inode_num);
    
    //printf("\n%d\n", find_inode());
    //printf("\n%d\n", get_inode_address(inode_num));
    
    int root_block = find_block();
    close_block(root_block);
    
    
    
    in->size = 0;
    in->flags = 1;
    in->blocks [0] = root_block;


    root->entries[0].inode_ID = (uint8_t)inode_num;
    root->entries[0].filename[0] = '~';
    root->entries[0].filename[1] = '\0';
    root->entries[1].inode_ID = root->entries[0].inode_ID;
    strncpy(root->entries[1].filename,root->entries[0].filename,31); //  root's parent directory is itself, 31 is max filename len

    inode_into_buffer(in,buf);
    write_block(inode_block,buf);
    dir_into_buffer(root,buf);
    write_block(root_block,buf);

    free(in);
    free(root);
    free(buf);
    
}

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

