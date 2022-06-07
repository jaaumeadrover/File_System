//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "directorios.h"

int main(int argc, char const *argv[]){

    //Comprobar sintaxis correcta
    if (argc != 4){
        fprintf(stderr, "Error Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace\n");
        return EXIT_FAILURE_1;
    }
   
    // Montar dispositivo virtual
    if (bmount(argv[1]) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: al montar el dispositivo.\n");
        return EXIT_FAILURE_1;
    }

    //COMPROBAMOS QUE RUTA 1 ES DE FICHERO
     if (argv[2][strlen(argv[2]) - 1] == '/'){
        fprintf(stderr, "Error: La ruta del fichero original no es un fichero\n");
        return EXIT_FAILURE_1;
    }

    //COMPROBAMOS QUE RUTA 2 ES DE FICHERO
    if (argv[3][strlen(argv[3]) - 1] == '/'){
        fprintf(stderr, "Error: La ruta de ENLACE no es un fichero\n");
        return EXIT_FAILURE_1;
    }

    if (mi_link (argv[2],argv[3])==EXIT_FAILURE_1){
        return EXIT_FAILURE_1;
    }


    bumount();
    return EXIT_SUCCESS;
}