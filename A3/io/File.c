#include "File.h"

void InitLLFS(void){

    create_disk();

    BYTE_t* buffer = (BYTE_t *)calloc(BLOCK_SIZE, sizeof(BYTE_t));
    
    for(int i=0;i<NUM_BLOCKS;i++){ write_block(i,buffer); } //set everything to 0;

    BYTE_t* superblock = (BYTE_t *)calloc(BLOCK_SIZE, sizeof(BYTE_t));

    superblock [0]  = 0x53;
    superblock [1]  = 0x4B;
    superblock [2]  = 0x52;
    superblock [3]  = 0x54; //magic #
    
    superblock [4]  = 0x00; 
    superblock [5]  = 0x00; 
    superblock [6]  = 0x10; 
    superblock [7]  = 0x00; // 4096 blocks
     
    superblock [8]  = 0x00; 
    superblock [9]  = 0x00; 
    superblock [10] = 0x01; 
    superblock [11] = 0x00; //max 256 inodes BEACUSE ONLY 256 UNIQUE INODE ID (uint8_t)

  
    write_block(0,superblock);

    
    

    BYTE_t* FBV = (BYTE_t *)calloc(BLOCK_SIZE, sizeof(BYTE_t));
    
    FBV[0] = 0b00000000;
    FBV[1] = 0b00111111; // set bits 0-9 to not availible
    for(int i = 2;i<BLOCK_SIZE;i++){
        FBV[i] = 0b11111111;
    }

    write_block(1,FBV);
    
    free(FBV);
    free(buffer);
    free(superblock);

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

    read_block(2,buf);

    //for(int i = 0;i<2;i++){
        buf[(inode_num*2)] = first;
        buf[(inode_num*2)+1] = second;
    //}

    write_block(2,buf);
    free(buf);
}

void open_inode(int inode_num){
    assert(!(inode_num<0 || inode_num >= MAX_INODES));
    
    
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));


    //printf("\n*%u ** %u *\n",first,second);

    read_block(2,buf);

    //for(int i = 0;i<2;i++){
        buf[(inode_num*2)] = 0x00;
        buf[(inode_num*2)+1] = 0x00;
    //}

    write_block(2,buf);
    free(buf);
}



int find_inode(){
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    
    int i,j;

    for(i = 0;i<2;i++){
        read_block((2+i),buf);
     
        for(j = 0; j < BLOCK_SIZE; j+=2){
            if(i==0 && j==0){ continue; }
            if(buf[j] == 0x00 && buf[(j+1)] == 0x00){ return ((256*i) + (j/2)); }
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


}

void buffer_into_inode(inode_t* inode, BYTE_t* buffer){

    inode->size =   (buffer[0]<< 24) | (buffer[1]<< 16) | (buffer[2]<< 8) | (buffer[3]) ;
    inode->flags =  (buffer[4]<< 24) | (buffer[5]<< 16) | (buffer[6]<< 8) | (buffer[7]) ;

    int cur = 8;

    for(int i = 0; i<10; i++){ inode->blocks[i] =   (buffer[2*i + cur]<< 8) | (buffer[2*i + (cur+1)]) ; }

    inode->single_ind = ( ( buffer[28]<< 8 ) | (buffer[29]) );
    inode->double_ind = ( ( buffer[30]<< 8 ) | (buffer[31]) );



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

    

    read_block(2,buf);

    //for(int i = 0;i<2;i++){
        first = buf[(inode_num*2)] ;
        second = buf[(inode_num*2)+ 1];
    //}

    //write_block((inode_num/256)+2,buf);
    free(buf);

    int address = (first<<8) | (second) ;
    printf("\n* %d *\n",address);
    return address;

}

void remove_dir (BYTE_t* path){
    inode_t* in = (inode_t*)calloc(1,sizeof(inode_t));
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    dir_t*  parent = (dir_t*)calloc(1,sizeof(dir_t));

    
    BYTE_t new_path [MAX_PATH_LEN];

    strncpy(new_path,path,MAX_PATH_LEN);

    BYTE_t* tok;
    const char* delim = "/"; 

    tok = strtok(new_path,delim);

    BYTE_t* tokens [4];  // change back to size 4
    int path_len = 0;
    
  
  while( tok != NULL ) {

      tokens[path_len] = tok; 
      path_len++;

      tok = strtok(NULL, delim);
   }
    int parent_dir_block = ROOT_DIR_BLOCK;

    int parent_inode_ID = ROOT_INODE_ID;
    int parent_inode_address = ROOT_INODE_BLOCK;
    
    read_block(parent_inode_address,buf);
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



                   // printf("\n inode id: %d inode add: %d \n",parent_inode_ID, parent_inode_address);

     

                read_block(parent_inode_address,buf);
                buffer_into_inode(in,buf);
                assert(in->flags==1);
                parent_dir_block = in->blocks[0]; 
       
                read_block(parent_dir_block,buf);
                buffer_into_dir(parent,buf);

                printf("\n FRFR CUH : %s\n",parent->entries[0].filename);

                break;
            }
   
        }
            
    }



    dir_t* rm_dir = (dir_t*)calloc(1,sizeof(dir_t));

    int dir_inode_ID ;
    int dir_inode_address;
    int dir_block;

     

    for(int j=2;j<MAX_DIR_ENTRIES;j++){ // first entry in directory is itself, parent is second so we can skip checking those(start at 2)
            if(!(strncmp(tokens[path_len-1],parent->entries[j].filename,FILENAME_LEN))){
                dir_inode_ID=parent->entries[j].inode_ID;
                dir_inode_address = get_inode_address(dir_inode_ID);



                   // printf("\n inode id: %d inode add: %d \n",parent_inode_ID, parent_inode_address);

                strncpy(parent->entries[j].filename,"",FILENAME_LEN);
                parent->entries[j].inode_ID = 0;

                read_block(dir_inode_address,buf);
                buffer_into_inode(in,buf);
                assert(in->flags==1);
                dir_block = in->blocks[0]; 
       
                read_block(dir_block,buf);
                buffer_into_dir(rm_dir,buf);

                printf("\n BRUH : %s\n",rm_dir->entries[0].filename);

                break;
            }

    }

    for(int i = 2; i<MAX_DIR_ENTRIES; i++){   assert(rm_dir->entries[i].inode_ID==0);  } //assert dir has no entries beyong itself and parent 


    dir_into_buffer(parent,buf);
    write_block(parent_dir_block,buf);


    BYTE_t* wipe = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    write_block(dir_inode_address,wipe);
    write_block(dir_block,wipe);

    open_inode(dir_inode_ID);
    open_block(dir_inode_address);
    open_block(dir_block);

    //printf("inode: %d , next block: %d",find_inode(), find_block());
    //close_block(find_block());
    //printf("next dir: %d",find_block());

    free(in);
    free(buf);
    free(parent);
    free(wipe);
    free(rm_dir);

}

void make_dir (BYTE_t* path){

    inode_t* in = (inode_t*)calloc(1,sizeof(inode_t));
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    dir_t*  parent = (dir_t*)calloc(1,sizeof(dir_t));

    
    BYTE_t new_path [MAX_PATH_LEN];

    strncpy(new_path,path,MAX_PATH_LEN);

    BYTE_t* tok;
    const char* delim = "/"; 

    tok = strtok(new_path,delim);

    BYTE_t* tokens [4];  // change back to size 4
    int path_len = 0;
    
  
  while( tok != NULL ) {

      tokens[path_len] = tok; 
      path_len++;

      tok = strtok(NULL, delim);
   }
    int parent_dir_block = ROOT_DIR_BLOCK;

    int parent_inode_ID = ROOT_INODE_ID;
    int parent_inode_address = ROOT_INODE_BLOCK;
    
    read_block(parent_inode_address,buf);
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



                   // printf("\n inode id: %d inode add: %d \n",parent_inode_ID, parent_inode_address);

     

                read_block(parent_inode_address,buf);
                buffer_into_inode(in,buf);
                assert(in->flags==1);
                parent_dir_block = in->blocks[0]; 
       
                read_block(parent_dir_block,buf);
                buffer_into_dir(parent,buf);

                printf("\n Y ee t : %s\n",parent->entries[0].filename);

                break;
            }
   
        }
            
    }


    printf("\n%s\n",parent->entries[0].filename);

    inode_t* new_dir_inode = (inode_t*)calloc(1,sizeof(inode_t));
    dir_t*  new_dir = (dir_t*)calloc(1,sizeof(dir_t));
    
    
    int inode_num = find_inode();
    int inode_block = find_block();
    close_block(inode_block);
    close_inode(inode_num, inode_block);




    int new_dir_block = find_block(); 
    printf("POSSIBLR FUCKING ERROR HERE DKM : %d\n",new_dir_block);
    close_block(new_dir_block);

    printf("* inum: %d * iblock: %d * dirblock: %d *\n",inode_num, inode_block, new_dir_block);

    new_dir_inode->size = 0;
    new_dir_inode->flags = 1;
    new_dir_inode->blocks [0] = new_dir_block;




    new_dir->entries[0].inode_ID = (uint8_t)inode_num;
    strncpy(new_dir->entries[0].filename, tokens[path_len-1], FILENAME_LEN); //first entry is itself
    new_dir->entries[1].inode_ID = parent->entries[0].inode_ID;
    strncpy(new_dir->entries[1].filename, parent->entries[0].filename, FILENAME_LEN); // second entry is parent directory

    int dir_opening ;

    for(dir_opening=2;dir_opening<MAX_DIR_ENTRIES;dir_opening++){
        if(!strcmp(parent->entries[dir_opening].filename,"") ){break;};
    }


    assert(dir_opening<MAX_DIR_ENTRIES);


    parent->entries[dir_opening].inode_ID = (uint8_t)inode_num;
    strncpy(parent->entries[dir_opening].filename, tokens[path_len-1] , FILENAME_LEN); 


    inode_into_buffer(new_dir_inode,buf);
    write_block(inode_block,buf);

    dir_into_buffer(new_dir,buf);
    write_block(new_dir_block,buf);
    
    dir_into_buffer(parent,buf);
    write_block(parent_dir_block,buf);
    


    free(new_dir_inode);
    free(new_dir);
    free(in);
    free(parent);
    free(buf);


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
    strncpy(root->entries[1].filename,root->entries[0].filename,FILENAME_LEN); //  root's parent directory is itself, 31 is max filename len

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


void write_file(BYTE_t* path, FILE* file){
    

    inode_t* in = (inode_t*)calloc(1,sizeof(inode_t));
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    dir_t*  parent = (dir_t*)calloc(1,sizeof(dir_t));

    
    BYTE_t new_path [MAX_PATH_LEN];

    strncpy(new_path,path,MAX_PATH_LEN);

    BYTE_t* tok;
    const char* delim = "/"; 

    tok = strtok(new_path,delim);

    BYTE_t* tokens [5];  
    int path_len = 0;
    
  
  while( tok != NULL ) {

      tokens[path_len] = tok; 
      path_len++;

      tok = strtok(NULL, delim);
   }
    int parent_dir_block = ROOT_DIR_BLOCK;

    int parent_inode_ID = ROOT_INODE_ID;
    int parent_inode_address = ROOT_INODE_BLOCK;
    
    read_block(parent_inode_address,buf);
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



                   // printf("\n inode id: %d inode add: %d \n",parent_inode_ID, parent_inode_address);

     

                read_block(parent_inode_address,buf);
                buffer_into_inode(in,buf);
                assert(in->flags==1);
                parent_dir_block = in->blocks[0]; 
       
                read_block(parent_dir_block,buf);
                buffer_into_dir(parent,buf);

                //printf("\n Y ee t : %s\n",parent->entries[0].filename);

                break;
            }
   
        }
            
    }


    //printf("\n%s\n",parent->entries[0].filename);


    int dir_opening ;

    for(dir_opening=2;dir_opening<MAX_DIR_ENTRIES;dir_opening++){
        if(!strcmp(parent->entries[dir_opening].filename,"") ){break;};
    }

    assert(dir_opening<MAX_DIR_ENTRIES);

 



    fseek(file,0,SEEK_END);
    int len = ftell(file);

    fseek(file,0,SEEK_SET); // RETURN FP TO START
    printf("%d\n",len);
    
    int full_blocks = len/512;
    int extra_bytes = len%512;
    int blocks_needed = full_blocks+(extra_bytes!=0);
    
    //printf("%d ||||||||| %d",full_blocks,extra_bytes);
    printf("\n**%d**\n", blocks_needed);
    
    inode_t* file_inode = (inode_t*)calloc(1,sizeof(inode_t));
    int file_inode_ID = find_inode();
    int file_inode_block = find_block();
    close_block(file_inode_block);
    close_inode(file_inode_ID,file_inode_block);
    file_inode->size = blocks_needed * BLOCK_SIZE;
    file_inode->flags = 0;

    //printf("\n*\n** %d **\n*\n",blocks_needed);
    
    BYTE_t* double_buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    BYTE_t* single_buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t)); 

    parent->entries[dir_opening].inode_ID = (uint8_t)file_inode_ID;
    strncpy(parent->entries[dir_opening].filename, tokens[path_len-1] , FILENAME_LEN); 
    dir_into_buffer(parent,buf);
    write_block(parent_dir_block,buf);
    int block;

    if (blocks_needed<=10) {
       

        for(int i = 0; i<blocks_needed; i++){
            block = find_block();
            close_block(block);
            file_inode->blocks[i] = block;

            fseek(file,i*BLOCK_SIZE,SEEK_SET);
            fread(buf,BLOCK_SIZE,1,file);

            if(i == blocks_needed-1 ){   for(int j = extra_bytes; j<BLOCK_SIZE; j++){    buf[j]=0;   }  }

            write_block(block,buf);


        }
    } else if(blocks_needed>10 && blocks_needed<=266){
       
        block = find_block();
        close_block(block);

        file_inode->single_ind = block; 

        int di_blocks_written;
        int s_blocks_written;

        for(di_blocks_written = 0; di_blocks_written<10; di_blocks_written++){
            block = find_block();
            close_block(block);
            file_inode->blocks[di_blocks_written] = block;

            fseek(file,di_blocks_written*BLOCK_SIZE,SEEK_SET);
            fread(buf,BLOCK_SIZE,1,file);
            write_block(block,buf);
        }

        blocks_needed -= di_blocks_written;

      

        printf("^^^^^%d^^^^^\n",file_inode->single_ind);

        read_block(file_inode->single_ind,single_buf);

        for(s_blocks_written = 0; s_blocks_written<blocks_needed; s_blocks_written++){
            block = find_block();
            close_block(block);
            single_buf[2*s_blocks_written] = (block & 0xFF00) >> 8;
            single_buf[(2*s_blocks_written)+1] = (block & 0x00FF);


            fseek(file,(di_blocks_written+s_blocks_written)*BLOCK_SIZE,SEEK_SET);
            fread(buf,BLOCK_SIZE,1,file);
             if(s_blocks_written == blocks_needed-1 && extra_bytes != 0){   for(int j = extra_bytes; j<BLOCK_SIZE; j++){    buf[j]=0;  }   }
            write_block(block,buf);
        }

        write_block(file_inode->single_ind,single_buf); 
        printf("^&^%d\n",file_inode->single_ind);
            

    }   else if (blocks_needed > 266){ // if need more than 10 direct block and the 256 indirect


        printf("&&&&&&&&&&&&&");
        fflush(stdout);



        block = find_block();
        close_block(block);
        file_inode->double_ind = block; 

        
        block = find_block();
        close_block(block);

        file_inode->single_ind = block; 

        int di_blocks_written;
        int s_blocks_written;

        for(di_blocks_written = 0; di_blocks_written<10; di_blocks_written++){
            block = find_block();
            close_block(block);
            file_inode->blocks[di_blocks_written] = block;

            fseek(file,di_blocks_written*BLOCK_SIZE,SEEK_SET);
            fread(buf,BLOCK_SIZE,1,file);
            write_block(block,buf);
        }

        blocks_needed -= di_blocks_written;
      

        printf("^^^^^%d^^^^^\n",file_inode->single_ind);

        read_block(file_inode->single_ind,single_buf);

        for(s_blocks_written = 0; s_blocks_written<256; s_blocks_written++){
            block = find_block();
            close_block(block);
            single_buf[2*s_blocks_written] = (block & 0xFF00) >> 8;
            single_buf[(2*s_blocks_written)+1] = (block & 0x00FF);


            fseek(file,(di_blocks_written+s_blocks_written)*BLOCK_SIZE,SEEK_SET);
            fread(buf,BLOCK_SIZE,1,file);
             if(s_blocks_written == blocks_needed-1 && extra_bytes != 0){   for(int j = extra_bytes; j<BLOCK_SIZE; j++){    buf[j]=0;  }   }
            write_block(block,buf);
        }

        blocks_needed -= s_blocks_written;
        write_block(file_inode->single_ind,single_buf); 

     printf("^^BN^^^%d^^^^^\n",blocks_needed);
    
    int si_blocks_needed =  (blocks_needed/256) + (!((blocks_needed%256)==0)); // calculates how many single blocks need to be written in current iteration of double indirect block loop

    printf("^^SBN^^^%d^^^^^\n",si_blocks_needed);

   
    int blocks_to_write = 256*(blocks_needed>256)+((blocks_needed<=256)*blocks_needed%256);
    printf("^^B2W^^^%d^^^^^\n",blocks_to_write);

    int single_block;

    for(int k = 0; k<si_blocks_needed;k++){

            single_block = find_block();
            close_block(single_block);
            double_buf[2*k] = (single_block & 0xFF00) >> 8;
            double_buf[(2*k)+1] = (single_block & 0x00FF);

        read_block(single_block,single_buf);

        blocks_to_write = 256*(blocks_needed>256)+((blocks_needed<=256)*blocks_needed%256);

        for(int i = 0; i<blocks_to_write; i++){
            block = find_block();
            close_block(block);
            single_buf[(2*i)] = (block & 0xFF00) >> 8;
            single_buf[(2*i)+1] = (block & 0x00FF);

            fseek(file,((s_blocks_written+di_blocks_written)+i)*BLOCK_SIZE,SEEK_SET);
            fread(buf,BLOCK_SIZE,1,file);
            if(i == blocks_to_write-1 && extra_bytes != 0){   for(int j = extra_bytes; j<BLOCK_SIZE; j++){    buf[j]=0;   }  }
            write_block(block,buf);//blocks_needed -= 256;
        }
            if(blocks_needed>256){
                blocks_needed -= 256;
                s_blocks_written += 256;
            } else {
                for(int j = 2*blocks_to_write;j<BLOCK_SIZE;j++){  single_buf[j]=0;    }  
            }
            
            write_block(single_block,single_buf);
    }

    for(int j = 2*si_blocks_needed;j<BLOCK_SIZE;j++){  double_buf[j]=0;    }  
    write_block(file_inode->double_ind,double_buf);



  }  

    //printf("id: %d  ad: %d \n",file_inode_ID,file_inode_block);
    
    inode_into_buffer(file_inode,buf);
    write_block(file_inode_block,buf);

    

   free(file_inode);
   free(buf);
   free(double_buf);
   free(single_buf);
   free(parent);
   free(in);




}

void remove_file(BYTE_t* path){
    inode_t* in = (inode_t*)calloc(1,sizeof(inode_t));
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    dir_t*  parent = (dir_t*)calloc(1,sizeof(dir_t));

    
    BYTE_t new_path [MAX_PATH_LEN];

    strncpy(new_path,path,MAX_PATH_LEN);

    BYTE_t* tok;
    const char* delim = "/"; 

    tok = strtok(new_path,delim);

    BYTE_t* tokens [5];  // change back to size 4
    int path_len = 0;
    
  
  while( tok != NULL ) {

      tokens[path_len] = tok; 
      path_len++;

      tok = strtok(NULL, delim);
   }
    int parent_dir_block = ROOT_DIR_BLOCK;

    int parent_inode_ID = ROOT_INODE_ID;
    int parent_inode_address = ROOT_INODE_BLOCK;
    
    read_block(parent_inode_address,buf);
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

                   // printf("\n inode id: %d inode add: %d \n",parent_inode_ID, parent_inode_address);
     
                read_block(parent_inode_address,buf);
                buffer_into_inode(in,buf);
                assert(in->flags==1);
                parent_dir_block = in->blocks[0]; 
       
                read_block(parent_dir_block,buf);
                buffer_into_dir(parent,buf);

                printf("\n FRFR CUH : %s\n",parent->entries[0].filename);

                break;
            }
   
        }
            
    }

    int file_loc;

    for(file_loc=2;file_loc<MAX_DIR_ENTRIES;file_loc++){
        if(!strcmp(parent->entries[file_loc].filename,tokens[path_len-1]) ){    break;   };
    }

    assert(file_loc<MAX_DIR_ENTRIES);
    
    inode_t* file_inode = (inode_t*)calloc(1,sizeof(inode_t));

    BYTE_t* sing_buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    BYTE_t* doub_buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    
    int file_inode_ID = parent->entries[file_loc].inode_ID;
    int file_inode_address = get_inode_address(file_inode_ID);
    read_block(file_inode_address,buf);
    buffer_into_inode(file_inode,buf);
    
    //printf("id: %d  ad: %d ",file_inode_ID,file_inode_address);

    parent->entries[file_loc].inode_ID = 0;
    strncpy(parent->entries[file_loc].filename,"",FILENAME_LEN);
    dir_into_buffer(parent,buf);
    write_block(parent_dir_block,buf);


    //print_inode(file_inode);

  

    assert(file_inode->flags == 0);

    int blocks_to_erase = (file_inode->size / BLOCK_SIZE) + (!((file_inode->size%BLOCK_SIZE)==0));
    BYTE_t* wipe = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));

    printf("\nFUCK ME: %d \n", blocks_to_erase);

    if(blocks_to_erase<=10){

        for(int i = 0; i<blocks_to_erase; i++){
            write_block(file_inode->blocks[i],wipe);
            open_block(file_inode->blocks[i]);
            //printf("\n%d\n",file_inode->blocks[0]);
        }

    } else if (blocks_to_erase>10 && blocks_to_erase<=266){

        int block;

        int di_blocks_erased;
        int s_blocks_erased;

        for(di_blocks_erased = 0; di_blocks_erased<10; di_blocks_erased++){
           
            write_block(file_inode->blocks[di_blocks_erased],wipe);
            open_block(file_inode->blocks[di_blocks_erased]);

        }

        blocks_to_erase -= di_blocks_erased;

        printf("^%d^ ",file_inode->single_ind);

        read_block(file_inode->single_ind,sing_buf);

        //print_buf(sing_buf);

        for(s_blocks_erased = 0; s_blocks_erased<blocks_to_erase; s_blocks_erased++){
            block = ((sing_buf[2*s_blocks_erased] << 8) | (sing_buf[(2*s_blocks_erased)+1]) );

            printf("^%d^ ",block);
          
            write_block(block,wipe); 
            open_block(block);

           
        }
        
        for(int i = s_blocks_erased;i<BLOCK_SIZE;i++){  sing_buf[i] = 0;    }
        write_block(file_inode->single_ind,wipe);
        open_block(file_inode->single_ind);

         

    } else {

        printf("\n ONE MORE PINT \n");

        int s_blocks_erased;
        int block;
        int sing_block;

        for(int i = 0; i<10; i++){
            write_block(file_inode->blocks[i],wipe);
            open_block(file_inode->blocks[i]);
            //printf("\n%d\n",file_inode->blocks[0]);
        }
       
         read_block(file_inode->single_ind,sing_buf);

        for(s_blocks_erased = 0; s_blocks_erased<256; s_blocks_erased++){
            block = ((sing_buf[2*s_blocks_erased] << 8) | (sing_buf[(2*s_blocks_erased)+1]) );

            write_block(block,wipe); 
            open_block(block);

           
        }
        
        write_block(file_inode->single_ind,wipe);
        open_block(file_inode->single_ind);

        read_block(file_inode->double_ind,doub_buf);

        for(int k = 0; k<256; k++){
        
            sing_block = ((doub_buf[2*k] << 8) | (doub_buf[(2*k)+1]) );

            if(sing_block == 0){    break;  }

            read_block(sing_block,sing_buf);

            for(s_blocks_erased = 0; s_blocks_erased<256; s_blocks_erased++){
                block = ((sing_buf[2*s_blocks_erased] << 8) | (sing_buf[(2*s_blocks_erased)+1]) );

                if(block==0){ break; }

                write_block(block,wipe); 
                open_block(block);

            
            }
            
            write_block(sing_block,wipe);
            open_block(sing_block);

            }

            write_block(file_inode->double_ind,wipe);
            open_block(file_inode->double_ind);
        
    }
    //

    write_block(file_inode_address,wipe);
    open_block(file_inode_address);
    open_inode(file_inode_ID);

    printf("999|   %d  |999 ",find_block());
}

int block_empty(BYTE_t* buf){

    for(int i = 0; i < BLOCK_SIZE; i++)
    {
        if(buf[i]!=0){
            return 0;
        }
    }
    
    return 1;
}

void file_check(){
    BYTE_t* FBV = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    BYTE_t* buf = (BYTE_t*)calloc(BLOCK_SIZE,sizeof(BYTE_t));
    read_block(1,FBV); //read in free block vector

    if(block_empty(FBV)){
        printf("I WANNA DIE");
    }


    BYTE_t block_byte; 
    BYTE_t mask; 
    int bit_shift;

    for(int block = 0; block<NUM_BLOCKS;block++){
        block_byte = FBV[block/8];
        bit_shift = block%8;
        mask = (0b10000000 >> (bit_shift));
        read_block(block,buf);

        if( (block_byte & mask)!=mask ){ // if block is marked as used
            
            if(block_empty(buf)){ //if  block is marked as used but it is empty, check will free it
                if(block>=10){ //avoid the allocated block at the start that are empty
                    printf("Block %-4d Status: Empty but marked as used\n",block);
                    printf("Block %-4d Status: Opening...\n",block);
                    open_block(block);
                    printf("Block %-4d Status: Good\n",block);
                } else {  
                    printf("Block %-4d Status: Good\n",block); 
                    }
            
            } else {
                printf("Block %-4d Status: Good\n",block);
            }
            

        } else { // block is marked as free
            if(!block_empty(buf)){ //if  block is marked as free but it is occupied, check will free it
                    printf("Block %-4d Status: Occupied but marked as free\n",block);
                    printf("Block %-4d Status: Closing...\n",block);
                    close_block(block);
                    printf("Block %-4d Status: Good\n",block);
            } else {
                printf("Block %-4d Status: Good\n",block);
            }
        }


    }

}