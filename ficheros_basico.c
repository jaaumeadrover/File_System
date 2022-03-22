#include "ficheros_basico.h"

int tamMB(unsigned int nbloques){ // Calcula el tamaño en bloques necesario para el mapa de bits.
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

int tamAI(unsigned int ninodos){ // Calcula el tamaño en bloques del array de inodos.
    int AIsize = ((ninodos * INODOSIZE)/ BLOCKSIZE);
    if ((ninodos * INODOSIZE)%BLOCKSIZE){
        AIsize++;
    }
    return AIsize;

}

int initSB(unsigned int nbloques, unsigned int ninodos){ // Inic.
    struct superbloque SB;
    //inicialización
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


int initMB(){ 
//DECLARACIONES ATRIBUTOS
unsigned char bufferMB[BLOCKSIZE];
struct superbloque SB;

//inicializamos a 0 todo el buffer
memset(bufferMB, 0, BLOCKSIZE);

//leemos el superbloque
if(bread(posSB,&SB)==-1){
    return EXIT_FAILURE;
}


//Volcamos todo el contenido del buffer a memoria
for(int i=SB.posPrimerBloqueMB;i<=SB.posUltimoBloqueMB;i++){
    bwrite(i,bufferMB);
}

//Reservamos todos los bloques correspondientes a los metadatos
for (unsigned int i = posSB; i < SB.posPrimerBloqueDatos; i++){
    reservar_bloque();
}

    return EXIT_SUCCESS;
}

/*

*/
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
    //lectura para inicializar SB con datos del super bloque
    if(bread(posSB,&SB)==-1){
        return EXIT_FAILURE;
    }
    //inicializar variables
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    //lectura bloque indicado
    if (bread(nbloqueabs,&bufferMB)==-1){
        return EXIT_FAILURE;
    }

    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128;    // 10000000
    mascara >>= posbit;   // desplazamiento de bits a la derecha   
    
    if(bit){
        bufferMB[posbyte]|= mascara ;
    }else {
        bufferMB[posbyte] &= ~mascara;  // operadores AND y NOT para bits
    }
    //escritura
    if(bwrite(nbloqueabs,bufferMB)==-1){
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

char leer_bit(unsigned int nbloque){
    struct superbloque SB;
    //leemos el superbloque
    if(bread(posSB,&SB)==-1){
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }

    //declaración variables
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    //lectura bloque determinado
    if (bread(nbloqueabs,&bufferMB)==-1){
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
    //localizar byte concreto dentro del bloque
    posbyte = posbyte % BLOCKSIZE;

    //máscara + desplazamiento de bits
    unsigned char mascara = 128; 
    mascara >>= posbit;          // mover bits a la derecha
    mascara &= bufferMB[posbyte]; // AND
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha
    return mascara;
}



 int reservar_bloque(){
     /*
     unsigned char bufferMB[BLOCKSIZE];
     unsigned char bufferAux[BLOCKSIZE];
     struct superbloque SB;
     int nbloqueabs;
     //leemos superbloque
     if(bread(posSB,&SB)==-1){
         fprintf(stderr, "Error: lectura incorrecta");
         return EXIT_FAILURE;
     }

     //inicializamos buffer auxiliar
     memset(bufferAux, 255, BLOCKSIZE);
     if(SB.cantBloquesLibres>0){
         //buscamos primer bloque libre
         for(int i=SB.posPrimerBloqueMB;i<SB.posUltimoBloqueMB;i++){
             if(bread(nbloqueabs, bufferMB)==-1){
                 fprintf(stderr, "Error: lectura incorrecta");
                 return EXIT_FAILURE;
             }
             if(memcmp(bufferMB,bufferAux,BLOCKSIZE)<0){
                 
             }
             
         }
     }else{
         printf("NO QUEDAN BLOQUES LIBRES!!!");
     }
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
    while (bufferMB[posbyte] == 255)
    {
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

    if (escribir_bit(nbloque, 1) == EXIT_FAILURE){
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
 

 int liberar_bloque(unsigned int nbloque){
     struct superbloque SB;
     //lectura superbloque
     if(bread(posSB,&SB) == -1){
         fprintf(stderr, "Error: lectura incorrecta");
         return EXIT_FAILURE;
     }
     //Escribimos 0 en el MB y aumentamos nbloques libres
     escribir_bit(nbloque,0);
     SB.cantBloquesLibres++;

    //Actualizamos el Superbloque
     if(bwrite(posSB,&SB) == -1){
         fprintf(stderr, "Error: lectura incorrecta");
         return EXIT_FAILURE;
     }

     //Devolvemos el número de bloque liberado
     return nbloque;
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
    return EXIT_SUCCESS;
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
    //int posInodoSolici = ninodo%(BLOCKSIZE/INODOSIZE);
    //leemos el inodo del bloque correspondiente del disco
    int e = bread(primerBloqInodo, inodos);
    if (e < 0){
        fprintf(stderr, "Error: lectura incorrecta");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos){
    //Leer el superbloque del dispositivo virtual
    struct superbloque SB;
    if (bread(posSB, &SB) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }

    //Compruebamos si hay bloques libres en el disco.
    if (SB.cantBloquesLibres == 0){
        return EXIT_FAILURE;
    }

    //Actualizar la lista enlazada de inodos libres
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre++;
    SB.cantInodosLibres--;

    struct inodo inodoAUX;
    //Inicialización
    inodoAUX.tipo = tipo;
    inodoAUX.permisos = permisos;
    inodoAUX.nlinks = 1;
    inodoAUX.tamEnBytesLog = 0;
    inodoAUX.atime = time(NULL);
    inodoAUX.mtime = time(NULL);
    inodoAUX.ctime = time(NULL);
    inodoAUX.numBloquesOcupados = 0;
    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            inodoAUX.punterosIndirectos[j] = 0;
        }
        inodoAUX.punterosDirectos[i] = 0;
    }

    //Escribir el inodo inicializado en la posición del que era el primer inodo libre
    if (escribir_inodo(posInodoReservado, inodoAUX) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }

    if (bwrite(posSB, &SB) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
    //Escribimos el superbloque actualizado
    return EXIT_SUCCESS;
}

// int obtener_nRangoBL (struct inodo *inodo, unsigned int nblogico, unsigned int *ptr){
//       if (nblogico<DIRECTOS)  {    
//         *ptr=inodo->punterosDirectos[nblogico];     
//         return 0; } 
//     else if (nblogico<INDIRECTOS0) {         
//         *ptr=inodo->punterosIndirectos[0];        
//         return 1;  }   
//     else if (nblogico<INDIRECTOS1){            
//         *ptr=inodo->punterosIndirectos[1];            
//         return 2; }       
//     else if (nblogico<INDIRECTOS2) {             
//         *ptr=inodo->punterosIndirectos[2];               
//         return 3; }          
//     else {          
//        *ptr=0;            
//         error("Bloque lógico fuera de rango");        
//         return -1;  }
// }

// int obtener_indice (unsigned int nblogico, int nivel_punteros){
//     if (nblogico < DIRECTOS){ 
//         return nblogico;   //ej nblogico=8
//     }else if (nblogico < INDIRECTOS0)  {
//         return nblogico - DIRECTOS;   //ej nblogico=204
//     }else if (nblogico < INDIRECTOS1){    //ej nblogico=30.004        
//         if (nivel_punteros == 2){ 
//             return (nblogico - INDIRECTOS0) / NPUNTEROS;}          
//         else if (nivel_punteros==1){ 
//             return (nblogico - INDIRECTOS0) % NPUNTEROS;}                   
//     }else if (nblogico < INDIRECTOS2){    //ej nblogico=400.004           
//         if (nivel_punteros == 3){ 
//             return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);}              
//          else if (nivel_punteros == 2){       
//             return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;}            
//         else if (nivel_punteros == 1){
//             return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;}        
//   }          
// }


// int traducir_bloque_inodo(int ninodo, int nblogico, char reservar){
      
// }        