//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "directorios.h"
#define DEBUGGER 1

int main(int argc, char const *argv[])
{
    if (argc != 5){
        fprintf(stderr, "Error de sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return EXIT_FAILURE_1;
    }
    //Comprabar que se trata de un fichero
    if ((argv[2][strlen(argv[2]) - 1]) == '/'){
        fprintf(stderr, "No es un fichero.\n");
        return EXIT_FAILURE_1;
    }

    int bytes_escritos;
    // Monta el dispositivo virtual en el sistema.
    if (bmount(argv[1]) == EXIT_FAILURE_1){
        fprintf(stderr, "mi_escribir.c: Error al montar el dispositivo virtual.\n");
        return EXIT_FAILURE_1;
    }

#if DEBUGGER
    fprintf(stderr, "Longitud texto: %ld\n", strlen(argv[3]));
#endif

    bytes_escritos = mi_write(argv[2], argv[3], atoi(argv[4]), strlen(argv[3]));
    if (bytes_escritos < 0){
        bytes_escritos=0;
    }

    if (bumount() == EXIT_FAILURE_1){
        return EXIT_FAILURE_1;
    }

#if DEBUGGER
    fprintf(stderr, "Bytes escritos: %d\n", bytes_escritos);
#endif

    return EXIT_SUCCESS;
}