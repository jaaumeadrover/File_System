#include "ficheros.h"


int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    struct inodo inodo;
    unsigned int primerBL,ultimoBL,desp1;
    if(leer_inodo(ninodo,&inodo)==EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    if ((inodo.permisos & 2) != 2){
        primerBL=offset/BLOCKSIZE;
        ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
        
        desp1 = offset % BLOCKSIZE;
    }
    
}


int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){

}


int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){
    struct inodo inodo;
    if(leer(ninodo,&inodo)==EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    //init type and permissions
    p_stat->tipo=inodo.tipo;
    p_stat->permisos=inodo.permisos;
    //init entry links in directory
    p_stat->nlinks=inodo.nlinks;
    //init size
    p_stat->tamEnBytesLog=inodo.tamEnBytesLog;
    //init timestamps
    p_stat->atime=inodo.atime;
    p_stat->mtime=inodo.mtime;
    p_stat->ctime=inodo.ctime;

    //init fulfilled blocks
    p_stat->numBloquesOcupados=inodo.numBloquesOcupados;

}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo)==EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
        }
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);
    return EXIT_SUCCESS;
}