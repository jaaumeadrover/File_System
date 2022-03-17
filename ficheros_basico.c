#include "ficheros_basico.h"

int tamMB(unsigned int nbloques){ // Calcula el tamaño en bloques necesario para el mapa de bits.
    int MBsize;
    MBsize = (nbloques / 8);
    if ((nbloques % 8)){
        MBsize++;
    }
    MBsize = MBsize / BLOCKSIZE;
    if (MBsize % BLOCKSIZE)
    {
        MBsize++;
    }
    return MBsize;
}

int tamAI(unsigned int ninodos){ // Calcula el tamaño en bloques del array de inodos.
    int AIsize = ((ninodos * INODOSIZE)/ BLOCKSIZE);
    if ((ninodos * INODOSIZE)%BLOCKSIZE){
        AIsize++;
    }
    return AIsize;

}

int initSB(unsigned int nbloques, unsigned int ninodos){ // Inic.
    struct superbloque SB;
    //SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques-1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;
    //escritura
    if(bwrite(posSB,&SB)==-1){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int initMB(){ // inicializa el mapa de bits.
unsigned char bufferMB[BLOCKSIZE];
struct superbloque SB;

//escribimos a 0 todas las posiciones del buffer
memset(bufferMB, 0, BLOCKSIZE);

//lectura SB
if(bread(posSB,&SB)==-1){
    return EXIT_FAILURE;
}

int posInicialMB = SB.posPrimerBloqueMB;
int posFinalMB = SB.posUltimoBloqueMB;

//volcamos el buffer a memoria
for(int i=posInicialMB;i<=posFinalMB;i++){
    bwrite(i,bufferMB);
}
    return EXIT_SUCCESS;
}

int initAI(){
    struct superbloque SB;
    if(bread(posSB,&SB)==-1){
        return EXIT_FAILURE;
    }
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int contInodos = SB.posPrimerInodoLibre + 1; // si hemos inicializado SB.posPrimerInodoLibre = 0
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){ // para cada bloque del AI
        bread(i, inodos);
        for (int j = 0; j < (BLOCKSIZE / INODOSIZE); j++){                         // para cada inodo del AI
            inodos[j].tipo = 'l'; // libre
            if (contInodos < SB.totInodos){                                               // si no hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = contInodos; // enlazamos con el siguiente
                contInodos++;
            }else{ // hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                // hay que salir del bucle, el último bloque no tiene por qué estar completo !!!
            }
        }
        if (bwrite(i, inodos)==-1){
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}


int escribir_bit(unsigned int nbloque, unsigned int bit){
    struct superbloque SB;
    if(bread(posSB,&SB)==-1){
        return EXIT_FAILURE;
    }
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs,&bufferMB)==-1){
        return EXIT_FAILURE;
    }
    posbyte = posbyte % BLOCKSIZE;

    unsigned char mascara = 128;    // 10000000
    mascara >>= posbit;   // desplazamiento de bits a la derecha   
    if(bit){
        bufferMB[posbyte]|= mascara ;
    } else {
        bufferMB[posbyte] &= ~mascara;  // operadores AND y NOT para bits
    }
}
//MÉTODO 5
/*
int escribir_inodo(unsigned int ninodo, struct inodo inodo){
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    if(bread(posSB,&SB)==-1){
        
        return EXIT_FAILURE;
    }
    unsigned int posInodo=(ninodo/(BLOCKSIZE/INODOSIZE));
    posInodo+=SB.posPrimerBloqueAI;

    //lectura Inodo
    if(bread(posInodo,inodos)==-1){
        return EXIT_FAILURE;
    }
    //escritura inodo
    int id=ninodo%(BLOCKSIZE/INODOSIZE);
    inodos[id]=inodo;
    if(bwrite(posInodo,inodos)==-1){
        return EXIT_FAILURE;
    }
}*/

char leer_bit(unsigned int nbloque){
    struct superbloque SB;
    if(bread(posSB,&SB)==-1){
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs,&bufferMB)==-1){
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128; // 10000000
    mascara >>= posbit;          // desplazamiento de bits a la derecha
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha
    return mascara;
}



 int reservar_bloque(){
     
 }
 
 int liberar_bloque(unsigned int nbloque){
 
 }

int escribir_inodo(unsigned int ninodo, struct inodo inodo){
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    if(bread(posSB,&SB)==-1){
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
    unsigned int posInodo=(ninodo/(BLOCKSIZE/INODOSIZE));
    posInodo+=SB.posPrimerBloqueAI;

    //lectura Inodo
    if(bread(posInodo,inodos)==-1){
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
    //escritura inodo
    int id=ninodo%(BLOCKSIZE/INODOSIZE);
    inodos[id]=inodo;
    if(bwrite(posInodo,inodos)==-1){
        fprintf(stderr, "Error: escritura incorrecta");
        return EXIT_FAILURE;
    }
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    struct superbloque SB;
    //leemos el superbloque para obtener la localización del array de inodos.
    if(bread(posSB,&SB)==-1){
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
    //obtenemos el nº de bloque del array de inodos que tiene el inodo solicitado. 
    unsigned int primerBloqInodo = SB.posPrimerBloqueAI + (BLOCKSIZE / INODOSIZE);
    //creamos el array de inodos
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    //calculamos que inodo del array de inodoos nos interesa leer
    int posInodoSolici = ninodo%(BLOCKSIZE/INODOSIZE);
    //leemos el inodo del bloque correspondiente del disco
    int e = bread(primerBloqInodo, inodos);
    if (e < 0){
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
}