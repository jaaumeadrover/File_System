#include "ficheros_basico.h"

#define DEBUGN3 1

/**
 *  Función: tamMB:
 * ---------------------------------------------------------------------
 * params: unsigned int nbloques
 *
 * Calcula el tamaño en bloques necesario para el mapa de bits.
 *
 * Devuelve el numero de bloques necesario para el mapa de bits.
 */
int tamMB(unsigned int nbloques){
    int MBsize = (nbloques / 8);

    if ((nbloques % 8)){
        MBsize++;
    }
    MBsize = MBsize / BLOCKSIZE;
    if (MBsize % BLOCKSIZE){
        MBsize++;
    }
    return MBsize;
}

/**
 *  Función: tamAI:
 * ---------------------------------------------------------------------
 * params: unsigned int ninodos
 *
 * Calcula el tamaño en bloques del array de inodos.
 *
 * Devuelve el resultado del cálculo del numero de bloques de
 * array de inodos.
 */
int tamAI(unsigned int ninodos){
    int AIsize = ((ninodos * INODOSIZE) / BLOCKSIZE);
    if ((ninodos * INODOSIZE) % BLOCKSIZE){
        AIsize++;
    }
    return AIsize;
}

/**
 *  Función: initSB:
 * ---------------------------------------------------------------------
 * params: unsigned int nbloques, unsigned int ninodos
 *
 * Inicializa los datos del superbloque.
 *
 * Devuelve EXIT_FAILURE en caso de un error de escritura o
 * EXIT_SUCCESS en caso de no haberlo.
 */
int initSB(unsigned int nbloques, unsigned int ninodos){
    struct superbloque SB;
    // inicialización
    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;
    // escritura
    if (bwrite(posSB, &SB) == EXIT_FAILURE){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/**
 *  Función: initMB:
 * ---------------------------------------------------------------------
 * params:
 *
 * Inicializa el mapa de bits poniendo a 1 los bits que representan
 * los metadatos.
 *
 * Devuelve EXIT_FAILURE en caso de error o EXIT_SUCCESS en
 * caso de no haberlo.
 */
int initMB(){
    // DECLARACIONES ATRIBUTOS
    unsigned char bufferMB[BLOCKSIZE];
    struct superbloque SB;

    // inicializamos a 0 todo el buffer
    memset(bufferMB, 0, BLOCKSIZE);

    // leemos el superbloque
    if (bread(posSB, &SB) == EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    // Volcamos todo el contenido del buffer a memoria
    for (int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++){

        if((bwrite(i, bufferMB))==EXIT_FAILURE){
            fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE;
        }

    }

    // Reservamos todos los bloques correspondientes a los metadatos
    for (unsigned int i = posSB; i < SB.posPrimerBloqueDatos; i++){
        reservar_bloque();
    }

    return EXIT_SUCCESS;
}

/**
 *  Función: initAI:
 * ---------------------------------------------------------------------
 * params:
 *
 * Inicializa la lista de inodos libres.
 *
 * Devuelve EXIT_FAILURE en caso de error o EXIT_SUCCESS en caso
 * de no haberlo.
 */
int initAI(){
    struct superbloque SB;
    if (bread(posSB, &SB) == EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int contInodos = SB.posPrimerInodoLibre + 1; // si hemos inicializado SB.posPrimerInodoLibre = 0
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){ // para cada bloque del AI
        if(bread(i, inodos)==EXIT_FAILURE){
            fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE;
        }
        for (int j = 0; j < (BLOCKSIZE / INODOSIZE); j++){                         // para cada inodo del AI
            inodos[j].tipo = 'l'; // libre
            if (contInodos < SB.totInodos){                                               // si no hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = contInodos; // enlazamos con el siguiente
                contInodos++;
            }
            else{ // hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                // hay que salir del bucle, el último bloque no tiene por qué estar completo !!!
            }
        }
        if (bwrite(i, inodos) == EXIT_FAILURE){
            fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

/**
 *  Función: escribir_bit:
 * ---------------------------------------------------------------------
 * params:unsigned int nbloque, unsigned int bit
 *
 * Escribe el valor indicado por el parámetro bit: 0 (libre) ó 1 (ocupado) en un
 * determinado bit del MB que representa el bloque nbloque
 *
 * Devuelve EXIT_FAILURE en caso de error o EXIT_SUCCESS en caso
 * de no haberlo.
 */
int escribir_bit(unsigned int nbloque, unsigned int bit){
    struct superbloque SB;
    // lectura para inicializar SB con datos del super bloque
    if (bread(posSB, &SB) == EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }
    // inicializar variables
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    // lectura bloque indicado
    if (bread(nbloqueabs, &bufferMB) == EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128; // 10000000
    mascara >>= posbit;          // desplazamiento de bits a la derecha

    if (bit){
        bufferMB[posbyte] |= mascara;
    }else{
        bufferMB[posbyte] &= ~mascara; // operadores AND y NOT para bits
    }
    // escritura
    if (bwrite(nbloqueabs, bufferMB) == EXIT_FAILURE){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


/**
 *  Función: leer_bit:
 * ---------------------------------------------------------------------
 * params:unsigned int nbloque
 *
 * Lee un determinado bit del MB.
 *
 * Devuelve EXIT_FAILURE en caso de error o el valor del bit leido en 
 * caso de no haberlo
 */
char leer_bit(unsigned int nbloque){
    struct superbloque SB;
    // lectura superbloque
    if (bread(posSB, &SB) == EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    // declaración variables
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; // cálculo nbloque absoluto
    unsigned char bufferMB[BLOCKSIZE];                 // buffer

    // lectura bloque determinado
    if (bread(nbloqueabs, &bufferMB) == EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }
    // localizar byte concreto dentro del bloque
    posbyte = posbyte % BLOCKSIZE;

    // máscara + desplazamiento de bits
    unsigned char mascara = 128;
    mascara >>= posbit;           // mdesplazamiento derecha
    mascara &= bufferMB[posbyte]; // AND
    mascara >>= (7 - posbit);     // desplazamiento derecha

#if DEBUGN3
    printf("[leer_bit(%i) → posbyte:%i, posbit:%i, nbloqueMB:%i, nbloqueabs:%i)]\n", nbloque, posbyte, posbit, nbloqueMB, nbloqueabs);
#endif
    
    return mascara;
}

/**
 *  Función: reservar_bloque:
 * ---------------------------------------------------------------------
 * params:
 *
 * Encuentra el primer bloque libre, consultando el MB (primer bit a 0), 
 * lo ocupa
 *
 * Devuelve EXIT_FAILURE en caso de error o la posición absoluta del 
 * bloque reservado
 */
int reservar_bloque(){
    //Declaraciones de atributos
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    struct superbloque SB;
    int nbloqueabs;
    int posbyte;
    int posbit;

    //leemos superbloque
    if (bread(posSB, &SB) == -1) {
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    //inicializamos buffer auxiliar
    memset(bufferAux, 255, BLOCKSIZE);

    if (SB.cantBloquesLibres > 0) {
        //buscamos primer bloque libre
        for (nbloqueabs = SB.posPrimerBloqueMB;nbloqueabs < SB.posUltimoBloqueMB;nbloqueabs++) {
            if (bread(nbloqueabs, bufferMB) == -1) {
                fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
                return EXIT_FAILURE;
            }
            if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0) {
                break;
            }

        }
        posbyte = 0;
        //localizamos que byte tiene el valor diferente a 1
        while (bufferMB[posbyte] == 255) {
            posbyte++;
        }

        unsigned char mascara = 128;
        posbit = 0;
        //encontrar el primer bit a 0 en ese byte
        while (bufferMB[posbyte] & mascara) { // operador AND para bits
            bufferMB[posbyte] <<= 1;          // desplazamiento de bits a la izquierda
            posbit++;
        }

    nbloqueabs = ((nbloqueabs - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;

    if (escribir_bit(nbloqueabs, 1) == EXIT_FAILURE){
        fprintf(stderr, "Error: escritura bit incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    SB.cantBloquesLibres--;

    //Rellenar el bufffer con 0's
    memset(bufferAux, 0, BLOCKSIZE);

    if (bwrite(SB.posPrimerBloqueDatos + nbloqueabs - 1, bufferAux) == EXIT_FAILURE){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()\n",__func__);
        return EXIT_FAILURE;
    }

       if (bwrite(posSB, &SB) == EXIT_FAILURE){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }
        return nbloqueabs;
    }else {
        fprintf(stderr,"NO QUEDAN BLOQUES LIBRES!!!");
        return EXIT_FAILURE;
    }

}

/**
 *  Función: liberar_bloque:
 * ---------------------------------------------------------------------
 * params: unsigned int nbloque
 *
 * Libera un bloque determinado
 *
 * Devuelve EXIT_FAILURE en caso de error o el numero de bloque liberado
 */
int liberar_bloque(unsigned int nbloque)
{
    struct superbloque SB;
    // lectura superbloque
    if (bread(posSB, &SB) == -1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }
    // Escribimos 0 en el MB y aumentamos nbloques libres
    escribir_bit(nbloque, 0);
    SB.cantBloquesLibres++;

    // Actualizamos el Superbloque
    if (bwrite(posSB, &SB) == -1){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    // Devolvemos el número de bloque liberado
    return nbloque;
}

/**
 *  Función: escribir_inodo:
 * ---------------------------------------------------------------------
 * params: unsigned int nbloque
 *
 * Escribe el contenido de una variable de tipo struct inodo en un 
 * determinado inodo del array de inodos.
 *
 * Devuelve EXIT_FAILURE en caso de error o EXIT_SUCCESS en caso
 * de no haberlo.
 */
int escribir_inodo(unsigned int ninodo, struct inodo inodo){
    // variables
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // lectura superbloque
    if (bread(posSB, &SB) == -1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    // calcular posición inodo
    unsigned int posInodo = (ninodo / (BLOCKSIZE / INODOSIZE));
    posInodo += SB.posPrimerBloqueAI;

    // lectura Inodo
    if (bread(posInodo, inodos) == -1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }
    // escritura inodo
    int id = ninodo % (BLOCKSIZE / INODOSIZE);
    inodos[id] = inodo;

    // actualizamos bloque correspondiente
    if (bwrite(posInodo, inodos) == -1){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 *  Función: leer_inodo:
 * ---------------------------------------------------------------------
 * params: unsigned int ninodo, struct inodo *inodo
 *
 * Lee un determinado inodo del array de inodos para volcarlo en una 
 * variable de tipo struct inodo pasada por referencia.
 *
 * Devuelve EXIT_FAILURE en caso de error o EXIT_SUCCESS en caso
 * de no haberlo.
 */
int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    struct superbloque SB;

    if (bread(posSB, &SB) == EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }
    // obtenemos el nº de bloque del array de inodos que tiene el inodo solicitado.
    unsigned int primerBloqInodo = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));
    // creamos el array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // calculamos que inodo del array de inodoos nos interesa leer
    if (bread(primerBloqInodo, inodos) == EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }
    // cambiamos el valor de la variable pasada por referencia
    int id = ninodo % (BLOCKSIZE / INODOSIZE);
    *inodo = inodos[id];

    return EXIT_SUCCESS;
}

/**
 *  Función: reservar_inodo:
 * ---------------------------------------------------------------------
 * params: unsigned char tipo, unsigned char permisos
 *
 * Encuentra el primer inodo libre (dato almacenado en el superbloque), 
 * lo reserva, devuelve su número y actualiza la lista enlazada de inodos 
 * libres.
 *
 * Devuelve EXIT_FAILURE en caso de error o la posición del inodo reservado
 */
int reservar_inodo(unsigned char tipo, unsigned char permisos){
    struct superbloque SB;
    struct inodo inodoAuxiliar;
    // lectura del superbloque
    if (bread(posSB, &SB) == EXIT_FAILURE){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE;
    }

    // Quedan bloques restantes?
    if (SB.cantBloquesLibres == 0){
        return EXIT_FAILURE;
    }

    // Actualizamos los valores del superbloque
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre++;
    SB.cantInodosLibres--;

    // Inicialización
    inodoAuxiliar.tipo = tipo;
    inodoAuxiliar.permisos = permisos;
    inodoAuxiliar.nlinks = 1;
    inodoAuxiliar.tamEnBytesLog = 0;
    // inicializar tiempos acceso,mod y creación
    inodoAuxiliar.atime = time(NULL);
    inodoAuxiliar.mtime = time(NULL);
    inodoAuxiliar.ctime = time(NULL);
    inodoAuxiliar.numBloquesOcupados = 0;

    for (int i = 0; i < 12; i++){
        for (int j = 0; j < 3; j++){
            inodoAuxiliar.punterosIndirectos[j] = 0;
        }
        inodoAuxiliar.punterosDirectos[i] = 0;
    }

    
    if (escribir_inodo(posInodoReservado, inodoAuxiliar) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    struct inodo debug;
    leer_inodo(posInodoReservado,&debug);


    if (bwrite(posSB, &SB) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    // Escribimos el superbloque actualizado
    return posInodoReservado;
}

int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr){
    if (nblogico < DIRECTOS){
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }else if (nblogico < INDIRECTOS0){
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }else if (nblogico < INDIRECTOS1){
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }else if (nblogico < INDIRECTOS2){
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }else{
        *ptr = 0;
        perror("Bloque lógico fuera de rango");
        return -1;
    }
}

int obtener_indice(unsigned int nblogico, int nivel_punteros){
    if (nblogico < DIRECTOS){
        return nblogico; // ej nblogico=8
    }else if (nblogico < INDIRECTOS0){
        return nblogico - DIRECTOS; // ej nblogico=204
    }else if (nblogico < INDIRECTOS1){ // ej nblogico=30.004
        if (nivel_punteros == 2){
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }else if (nivel_punteros == 1){
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }else if (nblogico < INDIRECTOS2){ // ej nblogico=400.004
        if (nivel_punteros == 3){
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }
        else if (nivel_punteros == 2){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        }else if (nivel_punteros == 1){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
    return -1;
}

int traducir_bloque_inodo(int ninodo, int nblogico, char reservar){
    struct inodo inodo;
    unsigned int ptr, ptr_ant;
    int salvar_inodo, nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];

    leer_inodo(ninodo, &inodo);
    ptr = 0;
    ptr_ant = 0;
    salvar_inodo = 0;
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros = nRangoBL;                           // el nivel_punteros +alto es el que cuelga del inodo
    while (nivel_punteros > 0){ // iterar para cada nivel de punteros indirectos
        if (ptr == 0){ // no cuelgan bloques de punteros
            if (reservar == 0)
                return -1; // bloque inexistente -> no imprimir nada por pantalla!!!
            else{ // reservar bloques de punteros y crear enlaces desde el  inodo hasta el bloque de datos
                salvar_inodo = 1;
                ptr = reservar_bloque(); // de punteros
                inodo.numBloquesOcupados++;
                inodo.ctime = time(NULL); // fecha actual
                if (nivel_punteros == nRangoBL){                                                 // el bloque cuelga directamente del inodo
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr; // (imprimirlo para test)
                        printf("[traducir_bloque_inodo()→ inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)]\n",
                           nRangoBL-1, ptr, ptr, nivel_punteros);

                }else{                            // el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr;
                    
                    printf("[traducir_bloque_inodo()→ inodo.punteros_nivel%i[%i] = %i (reservado BF %i para punteros_nivel%i)]\n",
                           nivel_punteros+1, indice, ptr, ptr, nivel_punteros);   // (imprimirlo para test)
                    bwrite(ptr_ant, buffer); // salvamos en el dispositivo el buffer de punteros modificado
                }
                memset(buffer, 0, BLOCKSIZE); // ponemos a 0 todos los punteros del buffer
            }
        }
        else{
            if(bread(ptr, buffer)==EXIT_FAILURE){
                fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
                return EXIT_FAILURE;
            } // leemos del dispositivo el bloque de punteros ya existente
        }
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        // guardamos el puntero actual
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        nivel_punteros--;
    } // al salir de este bucle ya estamos al nivel de datos
    if (ptr == 0){ // no existe bloque de datos
        if (reservar == 0){
            return -1; // error lectura ∄ bloque
        }else{
            salvar_inodo = 1;
            ptr = reservar_bloque(); // de datos
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            if (nRangoBL == 0){
                inodo.punterosDirectos[nblogico] = ptr; //
                printf("[traducir_bloque_inodo()→ inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n",
                       nblogico, ptr, ptr, nblogico);
            }else{
                buffer[indice] = ptr;    // asignamos la dirección del bloque de datos (imprimirlo para test)
                printf("[traducir_bloque_inodo()→ inodo.punteros_nivel1[%i] = %i (reservado BF %i para BL %i)]\n",
                       indice, ptr, ptr, nblogico);
                bwrite(ptr_ant, buffer); // salvamos en el dispositivo el buffer de punteros modificado
            }
        }
    }
    if (salvar_inodo == 1){
        escribir_inodo(ninodo, inodo); // sólo si lo hemos actualizado
    }
    return ptr; // nº de bloque físico correspondiente al bloque de datos lógico, nblogico
}


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