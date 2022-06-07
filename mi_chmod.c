//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "directorios.h"

int main(int argc, char const *argv[]){
    //DECLARACIONES
    char permisos;
    int err;

    //comprobamos sintaxis
    if(argc!=4){
        fprintf(stderr,"Error sintaxis incorrecta:./mi_chmod <disco> <permisos> </ruta>\n");
        return EXIT_FAILURE_1;
    }
    permisos = atoi(argv[2]);
    //comprobamos permisos
    if (permisos > 7|| permisos<0){
        fprintf(stderr,"Error: permisos debe ser un número válido (0-7)");
        return EXIT_FAILURE_1;
    }
    //montar disco
    if(bmount(argv[1])==EXIT_FAILURE_1){
        fprintf(stderr,"Error al montar el disco.");
        return EXIT_FAILURE_1;
    }
    //cambiamos permisos
    err=mi_chmod(argv[3],permisos);
    if(err<0){
        mostrar_error_buscar_entrada(err);
        return EXIT_FAILURE_1;
    }

    bumount();//desmontamos disco
    return EXIT_SUCCESS;
}