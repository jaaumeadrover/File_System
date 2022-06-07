//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "ficheros_basico.h"


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
 * Devuelve EXIT_FAILURE_1_1 en caso de un error de escritura o
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
    if (bwrite(posSB, &SB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
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
 * Devuelve EXIT_FAILURE_1 en caso de error o EXIT_SUCCESS en
 * caso de no haberlo.
 */
int initMB(){
    // DECLARACIONES ATRIBUTOS
    unsigned char bufferMB[BLOCKSIZE];
    struct superbloque SB;

    // inicializamos a 0 todo el buffer
    memset(bufferMB, 0, BLOCKSIZE);

    // leemos el superbloque
    if (bread(posSB, &SB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    // Volcamos todo el contenido del buffer a memoria
    for (int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++){

        if((bwrite(i, bufferMB))==EXIT_FAILURE_1){
            fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE_1;
        }

    }
    //OPTIMIZAR

    
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
 * Devuelve EXIT_FAILURE_1 en caso de error o EXIT_SUCCESS en caso
 * de no haberlo.
 */
int initAI(){
    struct superbloque SB;
    if (bread(posSB, &SB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int contInodos = SB.posPrimerInodoLibre + 1; // si hemos inicializado SB.posPrimerInodoLibre = 0
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){ // para cada bloque del AI
        if(bread(i, inodos)==EXIT_FAILURE_1){
            fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE_1;
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
        if (bwrite(i, inodos) == EXIT_FAILURE_1){
            fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE_1;
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
 * Devuelve EXIT_FAILURE_1 en caso de error o EXIT_SUCCESS en caso
 * de no haberlo.
 */
int escribir_bit(unsigned int nbloque, unsigned int bit){
    struct superbloque SB;
    // lectura para inicializar SB con datos del super bloque
    if (bread(posSB, &SB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
    // inicializar variables
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    // lectura bloque indicado
    if (bread(nbloqueabs, &bufferMB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
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
    if (bwrite(nbloqueabs, bufferMB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
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
 * Devuelve EXIT_FAILURE_1 en caso de error o el valor del bit leido en 
 * caso de no haberlo
 */
char leer_bit(unsigned int nbloque){
    struct superbloque SB;
    // lectura superbloque
    if (bread(posSB, &SB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    // declaración variables
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; // cálculo nbloque absoluto
    unsigned char bufferMB[BLOCKSIZE];                 // buffer

    // lectura bloque determinado
    if (bread(nbloqueabs, &bufferMB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

#if DEBUGN3
    printf("[leer_bit(%i) → posbyte:%i, posbit:%i, nbloqueMB:%i, nbloqueabs:%i)]\n", nbloque, posbyte, posbit, nbloqueMB, nbloqueabs);
#endif
    
    // localizar byte concreto dentro del bloque
    posbyte = posbyte % BLOCKSIZE;

    // máscara + desplazamiento de bits
    unsigned char mascara = 128;
    mascara >>= posbit;           // mdesplazamiento derecha
    mascara &= bufferMB[posbyte]; // AND
    mascara >>= (7 - posbit);     // desplazamiento derecha

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
 * Devuelve EXIT_FAILURE_1 en caso de error o la posición absoluta del 
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
        return EXIT_FAILURE_1;
    }

    //inicializamos buffer auxiliar
    memset(bufferAux, 255, BLOCKSIZE);

    if (SB.cantBloquesLibres > 0) {
        //buscamos primer bloque libre
        for (nbloqueabs = SB.posPrimerBloqueMB;nbloqueabs < SB.posUltimoBloqueMB;nbloqueabs++) {
            if (bread(nbloqueabs, bufferMB) == -1) {
                fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
                return EXIT_FAILURE_1;
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

    if (escribir_bit(nbloqueabs, 1) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: escritura bit incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    SB.cantBloquesLibres--;

    //Rellenar el bufffer con 0's
    memset(bufferAux, 0, BLOCKSIZE);

    if (bwrite(SB.posPrimerBloqueDatos + nbloqueabs - 1, bufferAux) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()\n",__func__);
        return EXIT_FAILURE_1;
    }

       if (bwrite(posSB, &SB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
        return nbloqueabs;
    }else {
        fprintf(stderr,"NO QUEDAN BLOQUES LIBRES!!!");
        return EXIT_FAILURE_1;
    }

}

/**
 *  Función: liberar_bloque:
 * ---------------------------------------------------------------------
 * params: unsigned int nbloque
 *
 * Libera un bloque determinado
 *
 * Devuelve EXIT_FAILURE_1 en caso de error o el numero de bloque liberado
 */
int liberar_bloque(unsigned int nbloque)
{
    struct superbloque SB;
    // lectura superbloque
    if (bread(posSB, &SB) == -1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
    // Escribimos 0 en el MB y aumentamos nbloques libres
    escribir_bit(nbloque, 0);
    SB.cantBloquesLibres++;

    // Actualizamos el Superbloque
    if (bwrite(posSB, &SB) == -1){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    // Devolvemos el número de bloque liberado
    return nbloque;
}

/**
 * Función: escribir_inodo:
 * ---------------------------------------------------------------------
 * params: unsigned int nbloque
 *
 * Escribe el contenido de una variable de tipo struct inodo en un 
 * determinado inodo del array de inodos.
 *
 * Devuelve EXIT_FAILURE_1 en caso de error o EXIT_SUCCESS en caso
 * de no haberlo.
 */
int escribir_inodo(unsigned int ninodo, struct inodo inodo){
    // variables
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // lectura superbloque
    if (bread(posSB, &SB) == -1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    // calcular posición inodo
    unsigned int posInodo = (ninodo / (BLOCKSIZE / INODOSIZE));
    posInodo += SB.posPrimerBloqueAI;

    // lectura Inodo
    if (bread(posInodo, inodos) == -1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
    // escritura inodo
    int id = ninodo % (BLOCKSIZE / INODOSIZE);
    inodos[id] = inodo;

    // actualizamos bloque correspondiente
    if (bwrite(posInodo, inodos) == -1){
        fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
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
 * Devuelve EXIT_FAILURE_1 en caso de error o EXIT_SUCCESS en caso
 * de no haberlo.
 */
int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    struct superbloque SB;

    if (bread(posSB, &SB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
    // obtenemos el nº de bloque del array de inodos que tiene el inodo solicitado.
    unsigned int primerBloqInodo = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));
    // creamos el array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // calculamos que inodo del array de inodoos nos interesa leer
    if (bread(primerBloqInodo, inodos) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
    // cambiamos el valor de la variable pasada por referencia
    int id = ninodo % (BLOCKSIZE / INODOSIZE);
    *inodo = inodos[id];

    return EXIT_SUCCESS;
}

/**
 * Función: reservar_inodo:
 * ---------------------------------------------------------------------
 * params: unsigned char tipo, unsigned char permisos
 *
 * Encuentra el primer inodo libre (dato almacenado en el superbloque), 
 * lo reserva, devuelve su número y actualiza la lista enlazada de inodos 
 * libres.
 *
 * Devuelve EXIT_FAILURE_1 en caso de error o la posición del inodo reservado
 */
int reservar_inodo(unsigned char tipo, unsigned char permisos){
    struct superbloque SB;
    struct inodo inodoAuxiliar;
    // lectura del superbloque
    if (bread(posSB, &SB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }

    // Quedan bloques restantes?
    if (SB.cantBloquesLibres == 0){
        return EXIT_FAILURE_1;
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

    
    if (escribir_inodo(posInodoReservado, inodoAuxiliar) == EXIT_FAILURE_1){
        return EXIT_FAILURE_1;
    }


    if (bwrite(posSB, &SB) == EXIT_FAILURE_1){
        return EXIT_FAILURE_1;
    }
    // Escribimos el superbloque actualizado
    return posInodoReservado;
}
/**
 *  Función: obtener_nRangoBL:
 * ---------------------------------------------------------------------
 * params: struct inodo *inodo, unsigned int nblogico, unsigned int *ptr
 *
 * Función auxiliar para obtener el rango de punteros en el que se situa
 * el bloque lógico que buscamos (0:D, 1:I0, 2:I1, 3:I2), y obtenemos 
 * además la dirección almacenada en el puntero correspondiente del inodo
 *
 * Devuelve la dirección almacenada en el puntero correspondiente del 
 * inodo o, en caso de error EXIT_FAILURE_1 
 */
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
/**
 *  Función: obtener_indice:
 * ---------------------------------------------------------------------
 * params: unsigned int nblogico, int nivel_punteros
 *
 * Función que generaliza la obtención de los índices de los bloques de 
 * punteros
 *
 * Devuelve el indice de los bloques de punteros o EXIT_FAILURE_1 en caso de error
 */
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
    return EXIT_FAILURE_1;
}
/**
 *  Función: traducir_bloque_inodo:
 * ---------------------------------------------------------------------
 * params: int ninodo, int nblogico, char reservar
 *
 * Esta función se encarga de obtener el numero de bloque físico 
 * correspondiente a un bloque lógico determinado del inodo indicado.
 *
 * Devuelve la posición fisica del bloque o EXIT_FAILURE_1 en caso de error 
 */
int traducir_bloque_inodo(int ninodo, int nblogico, char reservar){
    //DECLARACIONES
    struct inodo inodo;
    unsigned int ptr, ptr_ant;
    int salvar_inodo, nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];

    if(leer_inodo(ninodo, &inodo)==EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
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
                if (nivel_punteros == nRangoBL){// el bloque cuelga directamente del inodo
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr; // (imprimirlo para test)
#if DEBUGN6
                        printf("[traducir_bloque_inodo()→ inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)]\n",
                           nRangoBL-1, ptr, ptr, nivel_punteros);
#endif
                }else{// el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr;
#if DEBUGN6                  
                    printf("[traducir_bloque_inodo()→ inodo.punteros_nivel%i[%i] = %i (reservado BF %i para punteros_nivel%i)]\n",
                           nivel_punteros+1, indice, ptr, ptr, nivel_punteros);   // (imprimirlo para test)
#endif
                    bwrite(ptr_ant, buffer); // salvamos en el dispositivo el buffer de punteros modificado
                }
                memset(buffer, 0, BLOCKSIZE); // ponemos a 0 todos los punteros del buffer
            }
        }
        else{
            if(bread(ptr, buffer)==EXIT_FAILURE_1){
                fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
                return EXIT_FAILURE_1;
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
#if DEBUGN6                
                printf("[traducir_bloque_inodo()→ inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n",
                       nblogico, ptr, ptr, nblogico);
#endif
            }else{
                buffer[indice] = ptr;    // asignamos la dirección del bloque de datos (imprimirlo para test)
#if DEBUGN6                
                printf("[traducir_bloque_inodo()→ inodo.punteros_nivel1[%i] = %i (reservado BF %i para BL %i)]\n",
                       indice, ptr, ptr, nblogico);
#endif
                bwrite(ptr_ant, buffer); // salvamos en el dispositivo el buffer de punteros modificado
            }
        }
    }
    if (salvar_inodo == 1){
        escribir_inodo(ninodo, inodo); // sólo si lo hemos actualizado
    }
    return ptr; // nº de bloque físico correspondiente al bloque de datos lógico, nblogico
}

/**
 *  Función: liberar_inodo:
 * ---------------------------------------------------------------------
 * params: unsigned int ninodo
 *
 * Libera el inodo y todos los bloques de datos vinculados al inodo.
 *
 * Devuelve el número de inodo liberado o EXIT_FAILURE_1 en caso de error
 */
int liberar_inodo(unsigned int ninodo){
    struct superbloque SB;
    struct inodo inodo;
    int liberados=0;
    if(leer_inodo(ninodo,&inodo)==EXIT_FAILURE_1){
        fprintf(stderr, "Error: lectura incorrecta en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
    liberados=liberar_bloques_inodo(0,&inodo);
    if(liberados == EXIT_FAILURE_1){
        fprintf(stderr, "Error: al liberar bloques inodo en el método %s()",__func__);
        return EXIT_FAILURE_1;
    }
    inodo.numBloquesOcupados-=liberados;//debería quedar a 0
    //marcar el inodo a libre
    inodo.tipo='l';
    inodo.tamEnBytesLog=0;
    //actualizar lista de inodos libres
        // leemos el superbloque
        if (bread(posSB, &SB) == EXIT_FAILURE_1){
            fprintf(stderr, "Error: lectura incorrecta del SB en el método %s()",__func__);
            return EXIT_FAILURE_1;
        }
        //Incluir el inodo que queremos liberar en la lista de inodos libres
        int auxInode=SB.posPrimerInodoLibre;
        SB.posPrimerInodoLibre=ninodo;
        inodo.punterosDirectos[0]=auxInode;
        SB.cantInodosLibres++;
        
        //Escribimos los datos del inodo modificados
        if(escribir_inodo(ninodo, inodo) == EXIT_FAILURE_1){
            fprintf(stderr, "Error: escritura del inodo incorrecta en el método %s()",__func__);
            return EXIT_FAILURE_1;
        }    

        //Salvamos el superbloque
        if(bwrite(posSB, &SB) == EXIT_FAILURE_1){
            fprintf(stderr, "Error: escritura incorrecta en el método %s()",__func__);
            return EXIT_FAILURE_1;
        }

    return ninodo;

}
/**
 *  Función: liberar_bloques_inodo:
 * ---------------------------------------------------------------------
 * params: unsigned int primerBL, struct inodo *inodo
 *
 * Se encarga de liberar los bloques de datos unidos al inodo.
 *
 * Devuelve el número de bloques liberados o EXIT_FAILURE_1 en caso de error
 */
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo){
    unsigned int nivel_punteros, indice, ptr, nBL, ultimoBL;
    int nRangoBL;
    unsigned int bloques_punteros[3][NPUNTEROS]; //array de bloques de punteros
    unsigned char bufAux_punteros[BLOCKSIZE];
    int ptr_nivel[3];  //punteros a bloques de punteros de cada nivel
    int indices[3];    //indices de cada nivel
    int liberados = 0; // nº de bloques liberados
    int breads=0;
    int bwrites=0;

    if ((inodo->tamEnBytesLog) == 0){// el fichero está vacío
        return liberados; 
    }

    //Calculamos el último bloque lógico del inodo
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0){
        //Ultimo Bloque Lógico es un bloque entero
        ultimoBL = ((inodo->tamEnBytesLog) / BLOCKSIZE) - 1;
    }
    else{
        //Ultimo Bloque Lógico no es un bloque entero
        ultimoBL = (inodo->tamEnBytesLog) / BLOCKSIZE;
    }

    memset(bufAux_punteros, 0, BLOCKSIZE);
    ptr = 0;

#if DEBUGN6
    printf("[liberar_bloques_inodo()-> primerBL: %d, ultimoBL: %d]\n", primerBL, ultimoBL);
#endif

    //Recorrido de los bloques lógicos del inodo
    for (nBL = primerBL; nBL <= ultimoBL; nBL++){

        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr); //0:D, 1:I0, 2:I1, 3:I2
        if (nRangoBL < 0){
            fprintf(stderr, "Error: al obtener el rango del BL en el método %s()",__func__);
            return EXIT_FAILURE_1;
        }

        nivel_punteros = nRangoBL; //el nivel_punteros  mas alto cuelga del inodo

        while (ptr > 0 && nivel_punteros > 0)
        { //cuelgan bloques de punteros
            indice = obtener_indice(nBL, nivel_punteros);
            if ((indice == 0) || (nBL == primerBL)){
                //solo leemos del dispositivo si no está ya cargado previamente en un buffer
                if (bread(ptr, bloques_punteros[nivel_punteros - 1]) == EXIT_FAILURE_1){
                    fprintf(stderr, "Error: leer el dispositivo no cargado previamente, en el método %s()",__func__);
                    return EXIT_FAILURE_1;
                }
                breads++;//incrementamos número de lecturas
            }

            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        if (ptr > 0){ //si existe bloque de datos

            liberar_bloque(ptr);
            liberados++;
#if DEBUGN6
    printf("[liberar_bloques_inodo()-> liberado BF %d de datos par a BL %d]\n", ptr, nBL);
#endif
            if (nRangoBL == 0){ //es un puntero Directo
                inodo->punterosDirectos[nBL] = 0;
            }
            else
            {
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL){

                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];

                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0){
                        //No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        liberar_bloque(ptr);
                        liberados++;

                       //Incluir mejora para saltar los bloques que no sea necesario explorar!!! ...
                        switch (nivel_punteros-1){
                        case 0:
                            nBL += NPUNTEROS - indices[nivel_punteros-1] - 1;
                            break;
                        case 1:
                            nBL += NPUNTEROS * (NPUNTEROS -
                                                indices[nivel_punteros-1]) -1;
                            break;
                        case 2:
                            nBL += (NPUNTEROS * NPUNTEROS) *
                                       (NPUNTEROS - indices[nivel_punteros-1]) - 1;
                            break;
                        default:
                            break;
                        }
                        

#if DEBUGN6
    printf("[liberar_bloques_inodo()→ liberado BF %i de punteros_nivel%i correspondiente al BL: %i]\n", ptr, nivel_punteros, nBL);
#endif

                        if (nivel_punteros == nRangoBL){
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }
                        nivel_punteros++;
                    }
                    else{ //escribimos en el dispositivo el bloque de punteros modificado
                        if (bwrite(ptr, bloques_punteros[nivel_punteros - 1]) == EXIT_FAILURE_1){
                            fprintf(stderr, "Error: de escritura en el método %s()",__func__);
                            return EXIT_FAILURE_1;
                        }
                        bwrites++;//incrementamos número de escrituras
                        //hemos de salir del bucle ya que no será necesario liberar los bloques de niveles
                        //superiores de los que cuelga
                        nivel_punteros = nRangoBL + 1;
                    }//fsi
                }//fmientras
            }//fsi

        }//fsi
    }//fpara
#if DEBUGN6
    printf("[liberar_bloques_inodo()-> total bloques liberados: %d,total breads: %d, total bwrites: %d]\n", liberados,breads,bwrites);
#endif
    return liberados;
}


