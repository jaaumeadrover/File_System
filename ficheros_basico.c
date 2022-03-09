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

int posInicialMB=SB.posPrimerBloqueMB;
int posFinalMB = SB.posUltimoBloqueMB;

//volcamos el buffer a memoria
for(int i=posInicialMB;i<=posFinalMB;i++){
    bwrite(i,bufferMB);
}
    return EXIT_SUCCESS;
}

int initAI(){
    struct superbloque SB;
    bread(posSB,&SB);
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
        bwrite(i, inodos);
    }
    return EXIT_SUCCESS;
}
