#include "bloques.h"

static int descriptor=0;




int bmount(const char *camino){

    descriptor = open(camino, O_RDWR|O_CREAT, 0666);
    return descriptor;
}

int bumount(){
    int close(descriptor);
    if(close==EXIT_SUCCESS){
        //print se ha creado con éxito
    }else if(close==EXIT_FAILURE){
        perror(" No se ha podido cerrar el fichero.");
    }
}
int bwrite(unsigned int nbloque, const void *buf){

}

int bread(unsigned int nbloque, void *buf){

}

