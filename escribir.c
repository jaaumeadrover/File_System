//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include <stdlib.h>
#include "ficheros.h"

int main(int argc, char const *argv[]){

    if (argc != 4){
        fprintf(stderr, "Error de sintaxis: ./escribir <nombre_dispositivo> <ninodo> <permisos>\n");
        return EXIT_FAILURE_1;
    }

    //Definimos los 5 offsets 
    int offsets[5] = {9000, 209000, 30725000, 409605000, 480000000};
    //Mostramos la longitud del texto
    printf("Longitud del texto: %ld\n\n", strlen(argv[2]));

    //Montamos el dispositivo virtual
    if (bmount(argv[1]) == EXIT_FAILURE){
        fprintf(stderr, "Error: al montar el dispositivo virtual en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    // Reservamos un inodo para la escritura y se comprueba.
    int ninodo = reservar_inodo('f', 6);
    if (ninodo == -1){
        fprintf(stderr, "Error: al reservar el inodo en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    // Iteramos para escribir en los distintos offsets declarados
    for (int i = 0; i < (sizeof(offsets) / sizeof(int)); i++){
        printf("Nº inodo reservado: %d\n", ninodo);
        printf("offset: %d\n", offsets[i]);

        //llamamos a mi_write_f
        int bytesEscritos = mi_write_f(ninodo, argv[2], offsets[i], strlen(argv[2]));

        if ( bytesEscritos == -1){
            fprintf(stderr, "Error: al realizar mi_write_f() en el método %s()",__func__);
            return EXIT_FAILURE_1;
        }
        printf("Bytes escritos: %d\n", bytesEscritos);

        int longitud = strlen(argv[2]);
        char *buffer_texto = malloc(longitud);
        memset(buffer_texto, 0, longitud);
        int leidos = mi_read_f(ninodo, buffer_texto, offsets[i], longitud);
        printf("Bytes leídos: %d\n", leidos);

        struct STAT stat;
        // Obtenemos la indo del inodo que acabamos de escribir
        if (mi_stat_f(ninodo, &stat)){
            fprintf(stderr, "Error: al realizar mi_stat_f() en el método %s()",__func__);
            return EXIT_FAILURE_1;
        }

        printf("stat.tamEnBytesLog = %d\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados = %d\n\n", stat.numBloquesOcupados);

        // Si el parámetro <diferentes_indodos> es 0, reserva un nuevo inodo.
        if (strcmp(argv[3], "0")){
            ninodo = reservar_inodo('f', 6);
            if (ninodo == -1){
            fprintf(stderr, "Error: al reservar el inodo en el método %s()",__func__);
            return EXIT_FAILURE_1;
            }

        }
    }

    // Desmontamos el dispositivo virtual
    if (bumount() == EXIT_FAILURE_1){
        fprintf(stderr, "Error: al desmontar el dispositivo virtual en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
    return EXIT_SUCCESS;
}