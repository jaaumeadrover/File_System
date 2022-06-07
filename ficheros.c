//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "ficheros.h"

/**
 *  Función: mi_write_f:
 * ---------------------------------------------------------------------
 * params: unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes
 *
 * Función que escribe el contenido procedente de un buffer de memoria, buf_original,
 * de tamaño nbytes, en un fichero/directorio (correspondiente al inodo pasado como argumento, ninodo).
 *
 * Devuelve el número de bites escritos si se ha realizado correctamente y en caso de error -1 
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    //Declaración de atributos
    struct inodo inodo;
    unsigned int primerBL,ultimoBL;
    int desp1, desp2, nbfisico, escritos = 0, escritosAux = 0;
    unsigned char buf_bloque[BLOCKSIZE];

    //Leemos el inodo
    if(leer_inodo(ninodo,&inodo)==EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()\n",__func__);
        return EXIT_FAILURE_1;
    }

    //Miramos si el inodo tiene permisos de escritura
    if ((inodo.permisos & 2) != 2){
        fprintf(stderr, "Error: El inodo no tiene permisos de escritura en el método %s()\n",__func__);
        return EXIT_FAILURE_1;
    }

    primerBL=offset/BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    
    mi_waitSem();
    nbfisico = traducir_bloque_inodo(ninodo,primerBL,1);
    if(bread(nbfisico,buf_bloque) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()\n",__func__);
        mi_signalSem();  
        return EXIT_FAILURE_1;
    }
    mi_signalSem();    
        
    //Caso 1: primer bloque = ultimo bloque
    if(primerBL == ultimoBL){
        
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        escritosAux=bwrite(nbfisico,buf_bloque);
        if(escritosAux==EXIT_FAILURE_1){
            fprintf(stderr, "Error: escritura incorrecta en el método %s()\n",__func__);
            return EXIT_FAILURE_1;
        }
        escritos+=nbytes;
    }else { //Caso 2: primer bloque!= ultimo bloque

        //Primer BL
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);   
        escritosAux=bwrite(nbfisico,buf_bloque);
        if(escritosAux==EXIT_FAILURE_1){
            fprintf(stderr, "Error: escritura incorrecta en el método %s()\n",__func__);
            return EXIT_FAILURE_1;
        }
        escritos+=escritosAux-desp1;

        //bloques intermedios
        for (int i=primerBL+1;i<ultimoBL;i++){
            mi_waitSem();
            nbfisico = traducir_bloque_inodo(ninodo, i, 1);
            mi_signalSem();
            escritosAux=(bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE));
            if(escritosAux==EXIT_FAILURE_1){
                fprintf(stderr, "Error: escritura incorrecta en el método %s()\n",__func__);
                return EXIT_FAILURE_1;
            }
            escritos+=escritosAux;
        } 

        //ultimo BL
        mi_waitSem();
        nbfisico=traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if(bread(nbfisico,&buf_bloque) == EXIT_FAILURE_1){
            fprintf(stderr, "Error: lectura incorrecta en el método %s()\n",__func__);
            mi_signalSem();
            return EXIT_FAILURE_1;
        }
        mi_signalSem();
        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
        
        escritosAux=bwrite(nbfisico,buf_bloque);
        if(escritosAux==EXIT_FAILURE_1){
            fprintf(stderr, "Error: escritura incorrecta en el método %s()\n",__func__);
            return EXIT_FAILURE_1;
        }
        escritos+=desp2+1;
    }

    //Leer el inodo actualizado
    mi_waitSem();
    if(leer_inodo(ninodo,&inodo) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()\n",__func__);
        mi_signalSem();
        return EXIT_FAILURE_1;
    }

    //Actualizamos el tamaño en bytes solo si hemos escrito más allá del fichero
    if((offset + nbytes) >/*=*/ inodo.tamEnBytesLog){
        inodo.tamEnBytesLog = offset + nbytes;
        inodo.ctime=time(NULL);
    }

    inodo.mtime=time(NULL);

    if(escribir_inodo(ninodo,inodo) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()\n",__func__);
        mi_signalSem();
        return EXIT_FAILURE_1;
    }
    mi_signalSem();
    if(nbytes==escritos){
        return escritos;
    }
    fprintf(stderr,"EXPECTED %d bytes escritos, found %d.",nbytes,escritos);
    return EXIT_FAILURE_1;
}


/**
 *  Función: mi_read_f:
 * ---------------------------------------------------------------------
 * params: unsigned int ninodo, void *buf_original, unsigned int offset, 
 * unsigned int nbytes
 *
 * Lee información de un fichero/directorio (correspondiente al no de 
 * inodo, ninodo, pasado como argumento) y la almacena en un buffer 
 * de memoria, buf_original: le indicamos la posición de lectura inicial 
 * offset con respecto al inodo (en bytes) y el número de bytes nbytes 
 * que hay que leer.
 *
 * Devuelve la cantidad de bytes leidos si se ha realizado correctamente o 
 * EXIT_FAILURE_1 en caso de error
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){
    //Declaraciones
    unsigned int primerBL, ultimoBL;
    int desp1, desp2, nbfisico;
    int bytesleidos = 0;
    int auxByteLeidos = 0;
    char unsigned buf_bloque[BLOCKSIZE];
    struct inodo inodo;

    //Leer el inodo.
    if (leer_inodo(ninodo, &inodo) == EXIT_FAILURE_1)
    {
        fprintf(stderr, "Error in mi_read_f(): leer_inodo()\n");
        return bytesleidos;
    }

    //Comprobamos que el inodo tenga los permisos para leer
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, "No hay permisos de lectura\n");
        return bytesleidos;
    }

    if (offset >= inodo.tamEnBytesLog)
    {
        // No podemos leer nada
        return bytesleidos;
    }

    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    { // pretende leer más allá de EOF
        nbytes = inodo.tamEnBytesLog - offset;
        // leemos sólo los bytes que podemos desde el offset hasta EOF
    }

    //Asignaciones de las variables.
    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //Obtencion del numero de bloque fisico
    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
    //Caso el cual lo que queremos leer cabe en un bloque fisico
    if (primerBL == ultimoBL)
    {
        if (nbfisico != -1)
        {
            //Leemos el bloque fisico del disco
            auxByteLeidos = bread(nbfisico, buf_bloque);
            if (auxByteLeidos == EXIT_FAILURE_1)
            {
                fprintf(stderr, "Error in mi_read_f(): bread()\n");
                return EXIT_FAILURE_1;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        
        bytesleidos = nbytes; 
    }

    //Caso en el que la lectura ocupa mas de un bloque fisico
    else if (primerBL < ultimoBL)
    {
        //Parte 1: Primero bloque leido parcialmente
        if (nbfisico != -1)
        {
            //Leemos el bloque fisico del disco
            auxByteLeidos = bread(nbfisico, buf_bloque);
            if (auxByteLeidos == EXIT_FAILURE_1)
            {
                fprintf(stderr, "Error in mi_read_f(): bread()\n");
                return EXIT_FAILURE_1;
            }
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }

        bytesleidos =  BLOCKSIZE - desp1;


        //Parte 2: Bloques intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            //Obtenemos los bloques intermedios
            nbfisico = traducir_bloque_inodo(ninodo, i, 0);
            if (nbfisico != -1)
            {
                //Leemos el bloque fisico del disco
                auxByteLeidos = bread(nbfisico, buf_bloque);
                if (auxByteLeidos == EXIT_FAILURE_1)
                {
                    fprintf(stderr, "Error in mi_read_f(): bread()\n");
                    return EXIT_FAILURE_1;
                }
                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);            
            }

            bytesleidos += BLOCKSIZE;
        }

        //Parte 3: Último bloque leido parcialmente
        //Obtenemos el bloque fisico
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        //Parte 1: Primero bloque leido parcialmente
        if (nbfisico != -1)
        {
            //Leemos el bloque fisico del disco
            auxByteLeidos = bread(nbfisico, buf_bloque);
            if (auxByteLeidos == EXIT_FAILURE_1)
            {
                fprintf(stderr, "Error in mi_read_f(): bread()\n");
                return EXIT_FAILURE_1;
            }
            //Calculamos el byte lógico del último bloque hasta donde hemos de leer
            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
            
        }

        bytesleidos += desp2 + 1;

    }
    mi_waitSem(); //Semaforos
    //Leer el inodo actualizado.
    if (leer_inodo(ninodo, &inodo) == EXIT_FAILURE_1){
        fprintf(stderr, "Error in leer_inodo(): mi_read_f()\n");
        mi_signalSem(); //Semaforos
        return EXIT_FAILURE_1;
    }
    
    //Actualizar la metainformación
    inodo.atime = time(NULL);

    //Escribimos inodo
    if (escribir_inodo(ninodo, inodo) == EXIT_FAILURE_1){
        fprintf(stderr, "Error in escribir_inodo(): mi_read_f()\n");
        mi_signalSem(); //Semaforos
        return EXIT_FAILURE_1;
    }
    mi_signalSem(); //Semaforos

    //Comprobar que no haya errores de escritura y que se haya escrito todo bien.
    if (nbytes == bytesleidos){
        return bytesleidos;
    }
    else{
        return EXIT_FAILURE_1;
    }
}

/**
 *  Función: mi_stat_f:
 * ---------------------------------------------------------------------
 * params: unsigned int ninodo, struct STAT *p_stat
 *
 * Devuelve la metainformación de un directorio o fichero
 *
 * Devuelve EXIT_SUCCESS si se ha realizado correctamente o EXIT_FAILURE_1
 * en caso de error
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){
    struct inodo inodo;
    if(leer_inodo(ninodo,&inodo)==EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
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
/**
 *  Función: mi_chmod_f:
 * ---------------------------------------------------------------------
 * params: unsigned int ninodo, unsigned char permisos
 *
 * Cambia los permisos de un fichero/directorio
 *
 * Devuelve EXIT_SUCCESS si se ha realizado correctamente o EXIT_FAILURE_1
 * en caso de error
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
    mi_waitSem();
    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo)==EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        mi_signalSem();
        return EXIT_FAILURE_1;
    }
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);
    if(escribir_inodo(ninodo, inodo)==EXIT_FAILURE_1){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
        mi_signalSem();
        return EXIT_FAILURE_1;
    }
    mi_signalSem();
    return EXIT_SUCCESS;
}
/**
 *  Función: mi_truncar_f:
 * ---------------------------------------------------------------------
 * params: unsigned int ninodo, unsigned int nbytes
 *
 * Trunca un fichero/directorio (correspondiente al nº de inodo, ninodo,
 * pasado como argumento) a los bytes indicados como nbytes,
 * liberando los bloques necesarios.
 *
 * Devuelve el número de bloques liberados si se ha realizado correctamente
 * o EXIT_FAILURE_1 en caso de error
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    struct inodo inodo;
    int primerBL=0;
    int liberados=0;
    //comprobar que el inodo tenga permisos de escritura
    if( (nbytes % BLOCKSIZE) == 0){
        primerBL = nbytes/BLOCKSIZE;
    }else{
        primerBL = nbytes/BLOCKSIZE + 1;
    }

    if(leer_inodo(ninodo, &inodo)==EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    //Miramos si el inodo tiene permisos de escritura
    if ((inodo.permisos & 2) != 2){
        fprintf(stderr, "Error: El inodo no tiene permisos de escritura en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
    
    //Miramos que no se intente truncar más allá de EOF
    if(nbytes > inodo.tamEnBytesLog){
        fprintf(stderr, "Error: al intentar truncar más allá de EOF en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    //Obtenemos el bloque logico
    if((nbytes % BLOCKSIZE) == 0){
        primerBL = nbytes/BLOCKSIZE;
    }else{
        primerBL = nbytes/BLOCKSIZE+1;
    }

    liberados=liberar_bloques_inodo(primerBL,&inodo);
    inodo.mtime=time(NULL);
    inodo.ctime=time(NULL);
    inodo.tamEnBytesLog=nbytes;
    inodo.numBloquesOcupados-=liberados;
    
    if(escribir_inodo(ninodo, inodo) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: al escribir inodo en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
    
    return liberados;
}

