#include "../io/File.h"

int main(){
    InitLLFS();
    
     FILE* test = fopen("testfile.txt","rb+"); 
     write_file("~/test.txt",test);
     fclose(test);
  
    
   
    FILE* frt = fopen("out.txt","wb+");
    read_file("~/test.txt",frt);
    fclose(frt);
   
    return 0;
}