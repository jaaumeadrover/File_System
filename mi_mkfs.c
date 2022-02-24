#include "bloques.h"
int fd; //file descriptor
char* buf;

int main(int argc, char **argv){
    //ATRIBUTOS
    char *camino = argv[1];
    int nbloques = atoi(argv[2]);
    unsigned char buf[BLOCKSIZE];

    //Reservamos espacio
    if (!memset(buf, 0, BLOCKSIZE)){
        return EXIT_FAILURE;
    }

    //Comprobamos la sintaxis
    if (argc != 3){
        fprintf(stderr, "Error sintaxis: ./mi_fks <nombre del fichero> <numero de bloques>\n");
        return EXIT_FAILURE;
    }

    //Montaje
    if (bmount(camino) == -1){
        fprintf(stderr, "Error al montar el dispositivo.\n");
        return EXIT_FAILURE;
    }

    //Escritura
    for (int i = 0; i < nbloques; i++){
        if (bwrite(i, buf) == -1){
            fprintf(stderr, "Error escritura en el índice %i.\n", i);
            return EXIT_FAILURE;
        }
    }

    //DESMONTAR
    bumount(); //file descriptor
}