//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "directorios.h"

int main(int argc, char const *argv[]){

    //Primero, revisamos que la sintaxis introducida és correcta
    if (argc != 4){
        fprintf(stderr, "Error de sintaxis: ./mi_mkdir <disco> <permisos> </ruta>\n");
        return EXIT_FAILURE_1;
    }

    //Segundo, comprobamos que los permisos introducidos sean correctos (entre 0 y 7)
    if(atoi(argv[2]) < 0 || atoi(argv[2]) > 7){
        fprintf(stderr, "Error: modo inválido <<%d>>\n",atoi(argv[2]));
        return EXIT_FAILURE_1;
    }

    unsigned char permisos = atoi(argv[2]);

    //Si el último carácter que nos introducen en el comando es un "/", tenemos que crear
    //un directorio
    if ((argv[3][strlen(argv[3]) - 1] == '/')){
        //Montamos el disco
        if (bmount(argv[1]) == EXIT_FAILURE_1){
            fprintf(stderr, "Error: en el montaje del disco en el método %s()\n",__func__);
            return EXIT_FAILURE_1;
        }
        int err;
        if ((err = mi_creat(argv[3], permisos)) < 0){
            mostrar_error_buscar_entrada(err);
            return EXIT_FAILURE_1;
        }

        bumount();
    }
    //si es un directorio mostramos mensaje de error
    else{ 
        fprintf(stderr, "Error: No es una ruta de directorio válida. Se trata de un fichero. En el método %s()\n",__func__);
    }


    return EXIT_SUCCESS;
}