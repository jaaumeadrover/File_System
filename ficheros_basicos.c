#include "ficheros_basico.h"


int tamMB(unsigned int nbloques)
{ // Calcula el tamaño en bloques necesario para el mapa de bits.
    int MBsize;
    MBsize = (nbloques / 8);
    if((nbloques % 8)){
        MBsize++;
    }
    MBsize = MBsize / BLOCKSIZE;
    if(MBsize % BLOCKSIZE)

        MBsize++;
    }
    return MBsize;
}

int tamAI(unsigned int ninodos){ // Calcula el tamaño en bloques del array de inodos.
    int AIsize = (ninodos * INODOSIZE) / BLOCKSIZE;
    if ((ninodos * INODOSIZE) % BLOCKSIZE){
       AIsize++;
    }
    return AIsize;
}

int initSB(unsigned int nbloques, unsigned int ninodos){ // Inic.





}

int initMB()
{ // forcializa el mapa de bits.
}

int initAI(){
SB.totinodos = ninodos
struct inodo inodos [BLOCKSIZE/INODOSIZE]
...
contInodos := SB.posPrimerInodoLibre + 1;  //si hemos inicializado SB.posPrimerInodoLibre = 0
for (i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) { //para cada bloque del AI
   for (j = 0; j < BLOCKSIZE / INODOSIZE;  j++) {  //para cada inodo del AI
       inodos[j].tipo := ‘l’;  //libre
       if (contInodos < SB.totInodos) {  //si no hemos llegado al último inodo
               inodos[j].punterosDirectos[0] := contInodos;  //enlazamos con el siguiente
               contInodos++;}
        else{ //hemos llegado al último inodo
              inodos[j].punterosDirectos[0] := UINT_MAX;
               //hay que salir del bucle, el último bloque no tiene por qué estar completo !!!
        }
   }
}
}
