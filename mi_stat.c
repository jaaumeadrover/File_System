//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "directorios.h"

int main(int argc, char const *argv[]){
    //DECLARACIONES
    struct STAT p_stat;
    unsigned int p_inodo;

    if(argc!=3){
        fprintf(stderr,"Error de sintaxis: ./mi_stat <disco> </ruta>.\n");
        return EXIT_FAILURE_1;
    }
    
    //montar disco
     if (bmount(argv[1]) == EXIT_FAILURE_1){
        fprintf(stderr, "Error al montar disco.\n");
        return EXIT_FAILURE_1;
    }

    p_inodo = mi_stat(argv[2], &p_stat);

    if (p_inodo < 0){
        mostrar_error_buscar_entrada(p_inodo);
        return EXIT_FAILURE_1;
    }

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];


    ts = localtime(&p_stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    //Visualización resultados
    printf("Nº de inodo: %d\n", p_inodo);
    printf("tipo: %c\n", p_stat.tipo);
    printf("permisos: %d\n", p_stat.permisos);
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);
    printf("nlinks: %d\n", p_stat.nlinks);
    printf("tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);

    if (bumount() == -1){
        fprintf(stderr, "Error al desmontar disco.\n");
        return EXIT_FAILURE_1;
    }
    return EXIT_SUCCESS;
}

