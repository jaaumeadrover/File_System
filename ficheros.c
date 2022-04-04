#include "ficheros.h"


int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    //Declaración de atributos
    struct inodo inodo;
    unsigned int primerBL,ultimoBL;
    int desp1, desp2, nbfisico, escritos = 0, escritosAux = 0;
    unsigned char buf_bloque[BLOCKSIZE];

    //Leemos el inodo
    if(leer_inodo(ninodo,&inodo)==EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    //Miramos si el inodo tiene permisos de escritura
    if ((inodo.permisos & 2) != 2){
        fprintf(stderr, "Error: El inodo no tiene permisos de escritura en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    primerBL=offset/BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    
    nbfisico = traducir_bloque_inodo(ninodo,primerBL,1);

    if(bread(nbfisico,buf_bloque) == EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }
        
        
    //Caso 1: primer bloque = ultimo bloque
    if(primerBL == ultimoBL){
        
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        escritosAux=bwrite(nbfisico,buf_bloque);
        if(escritosAux==EXIT_FAILURE){
            fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE;
        }
        escritos+=nbytes;
    }else { //Caso 2: primer bloque!= ultimo bloque

        //Primer BL
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);   
        escritosAux=bwrite(nbfisico,buf_bloque);
        if(escritosAux==EXIT_FAILURE){
            fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE;
        }
        escritos+=escritosAux-desp1;

        //bloques intermedios
        for (int i=primerBL+1;i<ultimoBL;i++){
            nbfisico = traducir_bloque_inodo(ninodo, i, 1);
            escritosAux=(bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE));
            if(escritosAux==EXIT_FAILURE){
                fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
                return EXIT_FAILURE;
            }
            escritos+=escritosAux;
        } 

        //ultimo BL
        nbfisico=traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if(bread(nbfisico,&buf_bloque) == EXIT_FAILURE){
            fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE;
        }
        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
        
        escritosAux=bwrite(nbfisico,buf_bloque);
        if(escritosAux==EXIT_FAILURE){
            fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE;
        }
        escritos+=desp2+1;
    }

    //Leer el inodo actualizado
    if(leer_inodo(ninodo,&inodo) == EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    //Actualizamos el tamaño en bytes solo si hemos escrito más allá del fichero
    if((offset + nbytes) >/*=*/ inodo.tamEnBytesLog){
        inodo.tamEnBytesLog = offset + nbytes;
        inodo.ctime=time(NULL);
    }

    inodo.mtime=time(NULL);

    if(escribir_inodo(ninodo,inodo) == EXIT_FAILURE){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    if(nbytes==escritos){
        return escritos;
    }
    fprintf(stderr,"EXPECTED %d bytes escritos, found %d.",nbytes,escritos);
    return EXIT_FAILURE;
}




int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){
    //Declaración de atributos
    struct inodo inodo;
    unsigned int primerBL,ultimoBL;
    int desp1, desp2, nbfisico, leidos = 0;
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

    //Caso 3: Lo que queremos leer cabe en un solo solo bloque (primerBL = ultimoBL)
    if(primerBL == ultimoBL){
        
        if(nbfisico!=-1){
            if((bread(nbfisico, buf_bloque)) == (EXIT_FAILURE)){
                fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
                return EXIT_FAILURE;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        leidos = nbytes;
    }else { //Caso 2: primer bloque!= ultimo bloque

        //Primer BL
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);   
        if(bread(nbfisico,buf_bloque)==EXIT_FAILURE){
            fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE;
        }

        //leemos bloques intermedios
        for (int i=primerBL+1;i<=ultimoBL;i++){
            nbfisico=traducir_bloque_inodo(ninodo,i,0);
            //si existe bloque físico
            if(nbfisico!=-1){
                if(bread(nbfisico,buf_bloque)==EXIT_FAILURE){
                    fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
                    return EXIT_FAILURE;
                }
                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }
            leidos+=BLOCKSIZE;
           } 

        //ultimo BL
        nbfisico=traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if(nbfisico != -1){
            //Lectura del bloque físico del disco
            if(bread(nbfisico,buf_bloque) == EXIT_FAILURE){
            fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE;
        }
        //Averiguamos el ultimo byte que tenemos que leer del bloque
        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

        }else{
            leidos+=desp2 + 1;
        }
        
    }
    
    //Leer el inodo actualizado
    inodo.atime=time(NULL);
    //escribir inodo actualizado
    if(escribir_inodo(ninodo,inodo)==EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
    }


    //FINALIZACIÓN DEL MÉTODO
    if(nbytes==leidos){
        return leidos;
    }else{
        return EXIT_FAILURE;
    }
    
}


int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){
    struct inodo inodo;
    if(leer_inodo(ninodo,&inodo)==EXIT_FAILURE){
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
    return EXIT_SUCCESS;

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