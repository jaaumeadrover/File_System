//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "directorios.h"

int main(int argc, char const *argv[]){
    int liberados=0;
    //Comprobar sintaxis correcta
    if (argc != 3){
        fprintf(stderr, "Error sintaxis: ./mi_rm <disco> </ruta_fichero>\n");
        return EXIT_FAILURE_1;
    }
   
    // Montar dispositivo virtual
    if (bmount(argv[1]) == -1){
        fprintf(stderr, "Error al montar el disco.\n");
        return EXIT_FAILURE_1;
    }

    liberados=mi_unlink(argv[2]);
    if(liberados>=0){
        printf("Liberados: %d\n",liberados);
    }
    //desmontar dispositivo virtual
    bumount();
    return EXIT_SUCCESS;
}