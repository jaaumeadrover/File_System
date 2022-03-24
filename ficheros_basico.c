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
int tamMB(unsigned int nbloques) {
    int MBsize = (nbloques / 8);

    if ((nbloques % 8)) {
        MBsize++;
    }
    MBsize = MBsize / BLOCKSIZE;
    if (MBsize % BLOCKSIZE) {
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
int tamAI(unsigned int ninodos) {
    int AIsize = ((ninodos * INODOSIZE) / BLOCKSIZE);
    if ((ninodos * INODOSIZE) % BLOCKSIZE) {
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
 * EXIT_SUCCESS en caso de no haber error.
 */
int initSB(unsigned int nbloques, unsigned int ninodos) {
    struct superbloque SB;
    //inicialización
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
    //escritura
    if (bwrite(posSB, &SB) == EXIT_FAILURE) {
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
 * caso de no haber error.
 */
int initMB() {
    //DECLARACIONES ATRIBUTOS
    unsigned char bufferMB[BLOCKSIZE];
    struct superbloque SB;

    //inicializamos a 0 todo el buffer
    memset(bufferMB, 0, BLOCKSIZE);

    //leemos el superbloque
    if (bread(posSB, &SB) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }


    //Volcamos todo el contenido del buffer a memoria
    for (int i = SB.posPrimerBloqueMB;i <= SB.posUltimoBloqueMB;i++) {
        bwrite(i, bufferMB);
    }

    //Reservamos todos los bloques correspondientes a los metadatos
    for (unsigned int i = posSB; i < SB.posPrimerBloqueDatos; i++) {
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
 * de no haber error.
 */
int initAI() {
    struct superbloque SB;
    if (bread(posSB, &SB) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int contInodos = SB.posPrimerInodoLibre + 1; // si hemos inicializado SB.posPrimerInodoLibre = 0
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) { // para cada bloque del AI
        bread(i, inodos);
        for (int j = 0; j < (BLOCKSIZE / INODOSIZE); j++) {                         // para cada inodo del AI
            inodos[j].tipo = 'l'; // libre
            if (contInodos < SB.totInodos) {                                               // si no hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = contInodos; // enlazamos con el siguiente
                contInodos++;
            }
            else { // hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                // hay que salir del bucle, el último bloque no tiene por qué estar completo !!!
            }
        }
        if (bwrite(i, inodos) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}


int escribir_bit(unsigned int nbloque, unsigned int bit) {
    struct superbloque SB;
    //lectura para inicializar SB con datos del super bloque
    if (bread(posSB, &SB) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    //inicializar variables
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    //lectura bloque indicado
    if (bread(nbloqueabs, &bufferMB) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128;    // 10000000
    mascara >>= posbit;   // desplazamiento de bits a la derecha   

    if (bit) {
        bufferMB[posbyte] |= mascara;
    }
    else {
        bufferMB[posbyte] &= ~mascara;  // operadores AND y NOT para bits
    }
    //escritura
    if (bwrite(nbloqueabs, bufferMB) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

char leer_bit(unsigned int nbloque) {
    struct superbloque SB;
    //lectura superbloque
    if (bread(posSB, &SB) == EXIT_FAILURE) {
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }

    //declaración variables
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;//cálculo nbloque absoluto
    unsigned char bufferMB[BLOCKSIZE];//buffer 

    //lectura bloque determinado
    if (bread(nbloqueabs, &bufferMB) == EXIT_FAILURE) {
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
    //localizar byte concreto dentro del bloque
    posbyte = posbyte % BLOCKSIZE;

    //máscara + desplazamiento de bits
    unsigned char mascara = 128;
    mascara >>= posbit;           // mdesplazamiento derecha
    mascara &= bufferMB[posbyte]; // AND
    mascara >>= (7 - posbit);     // desplazamiento derecha

    return mascara;
}



int reservar_bloque() {
    //Declaraciones de atributos
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    struct superbloque SB;
    int nbloqueabs;
    int posbyte;
    int posbit;

    //leemos superbloque
    if (bread(posSB, &SB) == -1) {
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }

    //inicializamos buffer auxiliar
    memset(bufferAux, 255, BLOCKSIZE);

    if (SB.cantBloquesLibres > 0) {
        //buscamos primer bloque libre
        for (nbloqueabs = SB.posPrimerBloqueMB;nbloqueabs < SB.posUltimoBloqueMB;nbloqueabs++) {
            if (bread(nbloqueabs, bufferMB) == -1) {
                fprintf(stderr, "Error: lectura incorrecta");
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


    }
    else {
        printf("NO QUEDAN BLOQUES LIBRES!!!");
        return EXIT_FAILURE;
    }




    /*
        PARTE 2!!!!!!!!!!!!!!!!!!
    */


    //Leer el superbloque del dispositivo virtual
    struct superbloque SB;
    if (bread(posSB, &SB) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }

    //Compruebamos si hay bloques libres en el disco.
    if (SB.cantBloquesLibres == 0)
    {
        return EXIT_FAILURE;
    }

    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferaux[BLOCKSIZE];
    unsigned int posBloqueMB = SB.posPrimerBloqueMB;
    int libre = 0;

    //llenamos de 1's el buffer auxiliar
    if (memset(bufferaux, 255, BLOCKSIZE) == NULL)
    {
        return EXIT_FAILURE;
    }

    //Localizamos el primer bloque que contenga un 0
    while (libre == 0) // No lee mas que una linea
    {
        if (bread(posBloqueMB, bufferMB) == EXIT_FAILURE) // Que cojones esta leyendo aqui
        {
            fprintf(stderr, "Error while reading at reservar_bloque()\n");
            return EXIT_FAILURE;
        }

        int iguales = memcmp(bufferMB, bufferaux, BLOCKSIZE); // AQUI FALLA porque ve que hay un bloque libre
        if (iguales != 0)
        {
            libre = 1;
            break;
        }
        posBloqueMB++;
    }

    //Localizamos el byte que contiene el 0 dentro del bloque encontrado anteriormente
    unsigned int posbyte = 0;
    while (bufferMB[posbyte] == 255) {
        posbyte++;
    }

    //Localizamos el bit que es un 0 dentro del byte encontrado anteriormente
    unsigned char mascara = 128; // 10000000
    unsigned int posbit = 0;
    while (bufferMB[posbyte] & mascara) // operador AND para bits
    {
        bufferMB[posbyte] <<= 1; // desplazamiento de bits a la izquierda
        posbit++;
    }

    //Posición absoluta del bit en el dispositivo virtual
    unsigned int nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;
    // printf("posBloqueMB : %d \nSB.posPrimerBloqueMB : %d \nposbyte : %d\nposbit : %d\n\n",posBloqueMB, SB.posPrimerBloqueMB, posbyte, posbit);

    if (escribir_bit(nbloque, 1) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    //Decrementamos el número de bloques libres en 1
    SB.cantBloquesLibres--;

    // Rellenar el bufffer con 0's
    if (memset(bufferaux, 0, BLOCKSIZE) == NULL)
    {
        fprintf(stderr, "Error while memset in reservar_bloque()\n");
        return EXIT_FAILURE;
    }

    //POSIBLE PROBLEMA FALTA REVISAR///////////////////////////////////////////////
    //Escribimos en ese bloque el buffer anterior por si habia información "basura"
    if (bwrite(SB.posPrimerBloqueDatos + nbloque - 1, bufferaux) == EXIT_FAILURE)
    {
        fprintf(stderr, "Error while writting in reservar_bloque()\n");
        return EXIT_FAILURE;
    }
    ////////////////////////////////////////////////////////////////////////////////

    //Salvamos el superbloque
    if (bwrite(posSB, &SB) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }

    return nbloque;

}


int liberar_bloque(unsigned int nbloque) {
    struct superbloque SB;
    //lectura superbloque
    if (bread(posSB, &SB) == -1) {
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
    //Escribimos 0 en el MB y aumentamos nbloques libres
    escribir_bit(nbloque, 0);
    SB.cantBloquesLibres++;

    //Actualizamos el Superbloque
    if (bwrite(posSB, &SB) == -1) {
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }

    //Devolvemos el número de bloque liberado
    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo inodo) {
    //variables
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    //lectura superbloque
    if (bread(posSB, &SB) == -1) {
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }

    //calcular posición inodo
    unsigned int posInodo = (ninodo / (BLOCKSIZE / INODOSIZE));
    posInodo += SB.posPrimerBloqueAI;

    //lectura Inodo
    if (bread(posInodo, inodos) == -1) {
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
    //escritura inodo
    int id = ninodo % (BLOCKSIZE / INODOSIZE);
    inodos[id] = inodo;

    //actualizamos bloque correspondiente
    if (bwrite(posInodo, inodos) == -1) {
        fprintf(stderr, "Error: escritura incorrecta");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


int leer_inodo(unsigned int ninodo, struct inodo* inodo) {
    struct superbloque SB;

    if (bread(posSB, &SB) == EXIT_FAILURE) {
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
    //obtenemos el nº de bloque del array de inodos que tiene el inodo solicitado. 
    unsigned int primerBloqInodo = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));
    //creamos el array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    //calculamos que inodo del array de inodoos nos interesa leer
    if (bread(primerBloqInodo, inodos) == EXIT_FAILURE) {
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
    //cambiamos el valor de la variable pasada por referencia
    int id = ninodo % (BLOCKSIZE / INODOSIZE);
    *inodo = inodos[id];

    return EXIT_SUCCESS;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    struct superbloque SB;
    struct inodo inodoAuxiliar;
    //lectura del superbloque
    if (bread(posSB, &SB) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    //Quedan bloques restantes?
    if (SB.cantBloquesLibres == 0) {
        return EXIT_FAILURE;
    }

    //Actualizamos los valores del superbloque
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre++;
    SB.cantInodosLibres--;

    //Inicialización
    inodoAuxiliar.tipo = tipo;
    inodoAuxiliar.permisos = permisos;
    inodoAuxiliar.nlinks = 1;
    inodoAuxiliar.tamEnBytesLog = 0;
    //inicializar tiempos acceso,mod y creación
    inodoAuxiliar.atime = time(NULL);
    inodoAuxiliar.mtime = time(NULL);
    inodoAuxiliar.ctime = time(NULL);
    inodoAuxiliar.numBloquesOcupados = 0;

    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 3; j++) {
            inodoAuxiliar.punterosIndirectos[j] = 0;
        }
        inodoAuxiliar.punterosDirectos[i] = 0;
    }


    if (escribir_inodo(posInodoReservado, inodoAuxiliar) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (bwrite(posSB, &SB) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    //Escribimos el superbloque actualizado
    return EXIT_SUCCESS;
}



int obtener_nRangoBL(struct inodo* inodo, unsigned int nblogico, unsigned int* ptr) {
    if (nblogico < DIRECTOS) {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0) {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1) {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2) {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    else {
        *ptr = 0;
        error("Bloque lógico fuera de rango"); 
        return -1;
    }
}

int obtener_indice(unsigned int nblogico, int nivel_punteros) {
    if (nblogico < DIRECTOS) {
        return nblogico;   //ej nblogico=8
    }
    else if (nblogico < INDIRECTOS0) {
        return nblogico - DIRECTOS;   //ej nblogico=204
    }
    else if (nblogico < INDIRECTOS1) {    //ej nblogico=30.004        
        if (nivel_punteros == 2) {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }
        else if (nivel_punteros == 1) {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }    
else if (nblogico < INDIRECTOS2) {    //ej nblogico=400.004           
        if (nivel_punteros == 3) {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }
        else if (nivel_punteros == 2) {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;        
}
        else if (nivel_punteros == 1) {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
}


int traducir_bloque_inodo(int ninodo, int nblogico, char reservar) {
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
    while (nivel_punteros > 0) { // iterar para cada nivel de punteros indirectos
        if (ptr == 0) { // no cuelgan bloques de punteros
            if (reservar == 0)
                return -1; // bloque inexistente -> no imprimir nada por pantalla!!!
            else { // reservar bloques de punteros y crear enlaces desde el  inodo hasta el bloque de datos
                salvar_inodo = 1;
                ptr = reservar_bloque(); // de punteros
                inodo.numBloquesOcupados++;
                inodo.ctime = time(NULL); // fecha actual
                if (nivel_punteros == nRangoBL) {                                                 // el bloque cuelga directamente del inodo
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr; // (imprimirlo para test)
                }
                else {                            // el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr;    // (imprimirlo para test)
                    bwrite(ptr_ant, buffer); // salvamos en el dispositivo el buffer de punteros modificado
                }
                memset(buffer, 0, BLOCKSIZE); // ponemos a 0 todos los punteros del buffer
            }
        }
        else {
            bread(ptr, buffer); // leemos del dispositivo el bloque de punteros ya existente
        }
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        // guardamos el puntero actual
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        nivel_punteros--;
    } // al salir de este bucle ya estamos al nivel de datos
    if (ptr == 0) { // no existe bloque de datos
        if (reservar == 0) {
            return -1; // error lectura ∄ bloque
        }
        else {
            salvar_inodo = 1;
            ptr = reservar_bloque(); // de datos
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            if (nRangoBL == 0) {
                inodo.punterosDirectos[nblogico] = ptr; // (imprimirlo para test)
            }
            else {
                buffer[indice] = ptr;    // asignamos la dirección del bloque de datos (imprimirlo para test)
                bwrite(ptr_ant, buffer); // salvamos en el dispositivo el buffer de punteros modificado
            }
        }
    }
    if (salvar_inodo == 1) {
        escribir_inodo(ninodo, inodo); // sólo si lo hemos actualizado
    }
    return ptr; // nº de bloque físico correspondiente al bloque de datos lógico, nblogico
}

