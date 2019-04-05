#include "../io/File.h"

int main(){
    InitLLFS();
    
    FILE* test = fopen("boyboyboy.txt","r+");
    FILE* frt = fopen("fuck.txt","w+");
     //make_dir("~/new");
     //make_dir("~/new/skrt");
     //make_dir("~/new/BOI");
     //make_dir("~/new/BOI/pp");
     //make_dir("~/new/skrt/yeet");

     
    
    write_file("~/new/skrt/yeet/FUCK",test);
    read_file("~/new/skrt/yeet/FUCK",frt);
    //remove_file("~/new/skrt/yeet/FUCK");
    //write_file("~/boy.txt",test);

    fclose(frt);
    fclose(test);
    return 0;
}