//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "directorios.h"

int main(int argc, char const *argv[]){

    // Comprovamos que la sintaxis es correcta
    if (argc != 4){
        fprintf(stderr, "Error de sintaxis: ./mi_mkdir <disco><permisos></ruta>\n");
        return EXIT_FAILURE_1;
    }

    //Comprobamos permisos
    if (atoi(argv[2]) < 0 || atoi(argv[2]) > 7){
        fprintf(stderr, "Error: Permisos no válidos en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    char permisos = atoi(argv[2]);

    //Caso en que es un fichero
    if (argv[3][strlen(argv[3]) - 1] != '/'){ 
        //Montamos el disco
        if (bmount(argv[1]) == EXIT_FAILURE_1){
            fprintf(stderr, "Erroral montar disco en el método %s()",__func__);
            return EXIT_FAILURE_1;
        }
        int err;
        if ((err = mi_creat(argv[3], permisos)) < 0){
            mostrar_error_buscar_entrada(err);
            return EXIT_FAILURE_1;
        }
        if (bumount() == -1){
            fprintf(stderr, "Error al desmontar disco.\n");
            return EXIT_FAILURE_1;
        }
    }
    //Caso en el que no es un fichero
    else{
        fprintf(stderr, "No es una ruta de fichero válida\n");
    }
    return EXIT_SUCCESS;
}