//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "ficheros.h"

int main(int argc, char const *argv[]){

    //COmprobamos que la sintaxis sea correcta
    if (argc != 4){
        fprintf(stderr, "Error sintaxis: ./permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return EXIT_FAILURE_1;
    }

    int ninodo = atoi(argv[2]);
    int permisos = atoi(argv[3]);
    
    if (bmount(argv[1]) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: al montar el dispositivo virtual en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    if (mi_chmod_f(ninodo,permisos)) {
        fprintf(stderr, "Error: al modificar los permisos en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    if (bumount() == EXIT_FAILURE_1){
        fprintf(stderr, "Error: al desmontar el dispositivo virtual en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
}