//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "directorios.h"

int main(int argc, char const *argv[]){
    
    //DECLARACIONES
    int tambuffer = BLOCKSIZE * 4; //tamaño multiplo blocksize
    int leidos = 0, aux=0, offset=0;
    char const *camino = argv[2]; 
    char buffer[tambuffer];
    memset(buffer, 0, tambuffer);

    //Comprobar sintaxis 
    if (argc != 3){
        fprintf(stderr, "Error sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
        return EXIT_FAILURE_1;
    }
    //Montar dispositivo
    if (bmount(argv[1]) == EXIT_FAILURE_1){
        fprintf(stderr, "Error al montar el dispositivo.\n");
        return EXIT_FAILURE_1;
    }

    //COMPROBAMOS QUE RUTA 1 ES DE FICHERO
     if (argv[2][strlen(argv[2]) - 1] == '/'){
        fprintf(stderr, "Error: La ruta no es un fichero\n");
        return EXIT_FAILURE_1;
    }

    //Leemos todo el fichero o hasta completar el buffer
    if((aux = mi_read(camino, buffer, offset, tambuffer))==EXIT_FAILURE_1){
        return EXIT_FAILURE_1;
    }
    
    while (aux > 0){
        leidos += aux;
        write(1, buffer, aux);
        memset(buffer, 0, tambuffer);
        offset += tambuffer;
        //volvemos a leer
        if((aux = mi_read(camino, buffer, offset, tambuffer))==EXIT_FAILURE_1){
            return EXIT_FAILURE_1;
    }
    }
    fprintf(stderr," \n");
    if (leidos < 0){
        mostrar_error_buscar_entrada(leidos);
        leidos = 0;
    }
    fprintf(stderr, "Total_leidos: %d\n", leidos);

    bumount();
    return EXIT_SUCCESS;
}