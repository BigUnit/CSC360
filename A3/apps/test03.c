#include "../io/File.h"

int main(){
     
    FILE*  small_file = fopen("small.txt","rb+");
    FILE* med_file = fopen("med.txt","rb+");
    FILE* large_file = fopen("large.txt","rb+");

    write_file("~/SmallFile",small_file);
    write_file("~/csc360/MedFile",med_file);
    write_file("~/csc360/assignments/LargeFile",large_file);

    fclose(small_file);
    fclose(med_file);
    fclose(large_file);

    return 0;
}