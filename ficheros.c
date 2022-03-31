#include "ficheros.h"


int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    //Declaración de atributos
    struct inodo inodo;
    unsigned int primerBL,ultimoBL;
    int desp1, desp2, nbfisico, leidos = 0, leidosAux = 0;
    unsigned char buf_bloque[BLOCKSIZE];

    //Leemos el inodo
    if(leer_inodo(ninodo,&inodo)==EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    if ((inodo.permisos & 2) != 2){
        fprintf(stderr, "Error: El inodo no tiene permisos de escritura en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    primerBL=offset/BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    nbfisico = traducir_bloque_inodo(ninodo,primerBL,1);

    if(bread(nbfisico,&buf_bloque) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    memcpy(buf_bloque + desp1, buf_original, nbytes);
    if(bwrite(nbfisico,&buf_bloque) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }

    
}


int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){
    //Declaración de atributos
    struct inodo inodo;
    unsigned int primerBL,ultimoBL;
    int desp1, desp2, nbfisico, leidos = 0, leidosAux = 0;
    unsigned char buf_bloque[BLOCKSIZE];

    if(leer_inodo(ninodo,&inodo)==EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return leidos;
    }
    //Comprobamos que haya permisos de lectura
    if ((inodo.permisos & 4) != 4){
        fprintf(stderr, "Error: El inodo no tiene permisos de lectura en el método %s()",__func__);
        return leidos;
    }

    //Caso 1: No podemos leer nada 
    if(offset >= inodo.tamEnBytesLog){
        return leidos;
    }
    
    //Caso 2: Queremos leer más allá de EOF
    if((offset+nbytes>=inodo.tamEnBytesLog)){
        //leemos los bytes que podemos
        nbytes = inodo.tamEnBytesLog-offset;
    }

    primerBL=offset/BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //Calculamos el número de bloque físico
    nbfisico = traducir_bloque_inodo(ninodo,primerBL,0);

    //Caso 3: Lo que queremos leer cabe en un solo solo bloque
    if(primerBL == ultimoBL){
        
    }

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