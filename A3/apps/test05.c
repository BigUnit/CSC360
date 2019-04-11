#include "../io/File.h"

int main(){
    
     remove_file("~/SmallFile");
     remove_file("~/csc360/MedFile");
     remove_file("~/csc360/assignments/LargeFile");
     
     
     remove_dir("~/new/test/test2");
     remove_dir("~/new/test");
     remove_dir("~/new");
     
     remove_dir("~/csc360/assignments");
     remove_dir("~/csc360");
     

    return 0;
}