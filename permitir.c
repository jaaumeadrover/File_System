#include "ficheros_basico.h"

int main(int argc, char const *argv[]){
    if (argc != 4)
    {
        fprintf(stderr, "Error sintaxis: ./permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return EXIT_FAILURE;
    }

    int ninodo = atoi(argv[2]);
    int permisos = atoi(argv[2]);
    char *camino = argv[1];

    if (bmount(camino) == EXIT_FAILURE){
        fprintf(stderr, "Error al montar el dispositivo.\n");
        return EXIT_FAILURE;
    }

    mi_chmod_f(ninodo,permisos); 

    if (bumount() == EXIT_FAILURE){
        fprintf(stderr, "Error al desmontar el dispositivo.\n");
        return EXIT_FAILURE;
    }
}