#include "bloques.h"
int fd; //file descriptor
int nbloques;
char* buf;

int main(int argc, char **argv){
    //MONTAR
    fd=bmount(argv[1]);
    nbloques=argv[2];
    memset(&buf,0,BLOCKSIZE);

    //ESCRITURA
    for(int i=0;i<nbloques;i++){
        bwrite(i,*buf);
    }

    //DESMONTAR
    bumount(); //file descriptor
}