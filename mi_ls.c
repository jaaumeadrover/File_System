//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "directorios.h"


int main(int argc, char const *argv[]){

 if (argc != 3){
        fprintf(stderr, "Error de sintaxis: ./mi_ls <disco> </ruta_directorio>\n");
        return EXIT_FAILURE_1;
    }

    if (bmount(argv[1]) == -1){
        fprintf(stderr, "Error al montar disco.\n");
        return EXIT_FAILURE_1;
    }

    char buff[TAMBUFFER];
    memset(buff, 0, TAMBUFFER);

    int res = mi_dir(argv[2], buff);

    if (res < 0){
        mostrar_error_buscar_entrada(res);
        return EXIT_FAILURE_1;
    }else{
        printf("Total de entradas: %d\n", res);
        printf("%s\n", buff);
    }


    if (bumount() == -1){
        fprintf(stderr, "Error al desmontar disco.\n");
        return EXIT_FAILURE_1;
    }

    return EXIT_SUCCESS;
}