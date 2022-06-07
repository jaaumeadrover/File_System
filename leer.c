//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include <stdlib.h>
#include "ficheros.h"

#define tambuffer = 1500

int main(int argc, char const *argv[]){
    //Variables
    int ninodo;
    struct superbloque SB;
    struct inodo inodo;
    int offset = 0;
    int nbytes = 1500;
    int bytesleidos = 0;
    char buffer[nbytes];

    //Sintaxis incorrecta
    if (argc != 3){
        fprintf(stderr, "Error de sintaxis: leer <nombre_dispositivo><numero_inodo>\n");
        return EXIT_FAILURE_1;
    }

    //Inicializamos el buffer declarado
    memset(buffer, 0, nbytes);
    ninodo = atoi(argv[2]);
    // Montamos el dispositivo virtual
    if (bmount(argv[1]) == -1){
        fprintf(stderr, "Error: al montar el dispositivo virtual en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    //Leemos el superbloque
    if (bread(0, &SB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: al leer el superblque en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    // Leemos el fichero 
    int auxBytesleidos = mi_read_f(ninodo, buffer, offset, nbytes);
    while (auxBytesleidos > 0){
        bytesleidos = bytesleidos + auxBytesleidos;
        // Escribimos el contenido del buffer en la posición indicada
        write(1, buffer, auxBytesleidos);
        //Limpiamos el buffer
        memset(buffer, 0, nbytes);
        //Actualizamos el offset
        offset = offset + nbytes;
        auxBytesleidos = mi_read_f(ninodo, buffer, offset, nbytes);
    }

    // Leemos el i¡nodo del archivo
    if (leer_inodo(ninodo, &inodo)){
        fprintf(stderr, "Error: al leer el inodo en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    fprintf(stderr, "\ntotal_bytesleidos: %d\ntamEnBytesLog: %d\n", bytesleidos, inodo.tamEnBytesLog);
        
    //Desmontamos el dispositivo virtual el dispositivo virtual
    if (bumount() == EXIT_FAILURE_1){
        fprintf(stderr, "Error: al desmontar el dispositivo virtual en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
    return EXIT_SUCCESS;
}