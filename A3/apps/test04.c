#include "../io/File.h"

int main(){
     
    FILE*  small_file = fopen("small_read.txt","wb+");
    FILE* med_file = fopen("med_read.txt","wb+");
    FILE* large_file = fopen("large_read.txt","wb+");

    read_file("~/SmallFile",small_file);
    read_file("~/csc360/MedFile",med_file);
    read_file("~/csc360/assignments/LargeFile",large_file);

    fclose(small_file);
    fclose(med_file);
    fclose(large_file);

    return 0;
}