//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "bloques.h"
#include "semaforo_mutex_posix.h"

static sem_t *mutex;
static unsigned int inside_sc = 0;

static int descriptor = 0;//variable global estática descriptor fichero

/**
 *  Función: bmount:
 * ---------------------------------------------------------------------
 * params: const char *camino
 * 
 * Monta el dispositivo virtual.
 * 
 * Devuelve 0 (o EXIT_SUCCESS) si se ha cerrado el fichero correctamente,
 * o -1 (o EXIT_FAILURE) en caso contrario.
 */
int bmount(const char *camino){
    //nivel 12
    if(descriptor>0){
        close(descriptor);
    }

      if (mutex==0) { 
       mutex = initSem(); 
       if (mutex == SEM_FAILED) {
           return -1;
       }
   }

    umask(000);
    descriptor = open(camino, O_RDWR|O_CREAT, 0666);
    if (descriptor == EXIT_FAILURE_1){
        perror("intento fallido de abrir el fichero.");
        return EXIT_FAILURE_1;
    }
    return descriptor;
}
/*
* Función bumount():
* ---------------------------------------------
* params:
* 
* Desmonta el dispositivo virtual. Básicamente 
* llama a la función close() para liberar el descriptor de ficheros.
* 
* Devuelve 0 (o EXIT_SUCCESS) si se ha cerrado el 
* fichero correctamente, o -1 (o EXIT_FAILURE) en caso contrario.
*/
int bumount(){
    descriptor=close(descriptor);
    deleteSem();
    if (close(descriptor) == EXIT_FAILURE_1){
        return EXIT_FAILURE_1;
    }
        return EXIT_SUCCESS;
}

/*
* Función bwrite():
* ---------------------------------------------
* params: unsigned int nbloque, const void *buf
* 
* Escribe 1 bloque en el dispositivo virtual, 
* en el bloque físico especificado por nbloque.
* 
* Devuelve el numero de bytes que se han escrito en el disco.
*/
int bwrite(unsigned int nbloque, const void *buf){
    int bytes;
    
    //Posicionamos el puntero
    if(lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) == -1){
        //Error al posicionar el puntero
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno)); 
        return EXIT_FAILURE_1; //-1
    }
    //Volcamos el contenido del buffer
    bytes = write(descriptor, buf, BLOCKSIZE);
    //Control de errores
    if(bytes < 0){
        perror("Escritura incorrecta");
        return EXIT_FAILURE_1;
    }
    return bytes;
    
   
}
/*
* Función bread():
* -------------------------------------
* params: unsigned int nbloque, void *buf.
* 
* Lee un bloque en el dispositivo virtual, en el bloque físico 
* especificado por nbloque.
*
* Devuelve el nº de bytes que ha podido leer (si ha ido bien, 
* será BLOCKSIZE), o -1 (o EXIT_FAILURE) si se produce un error.
*/
int bread(unsigned int nbloque, void *buf){
    int nbytes;
    //Posicionamiento del puntero
    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET)==-1){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno)); 
        return EXIT_FAILURE_1;
    }
    //Lectura
    nbytes = read(descriptor, buf, BLOCKSIZE);
    if (nbytes < 0){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno)); 
        return EXIT_FAILURE_1;
    }
    return nbytes;//devolvemos número de bytes leídos
}

/**
 * Función mi_waitSem():
 * ---------------------------------------------------------------------
 * params: 
 * 
 * Función para llamar al wait del semaforo.
 * 
 * 
 */
void mi_waitSem() {
   if (!inside_sc) { // inside_sc==0
       waitSem(mutex);
   }
   inside_sc++;
}

/**
 * Función mi_signalSem():
 * ---------------------------------------------------------------------
 * params: 
 * 
 * Función para llamar al wait del semaforo.
 * 
 * 
 */
void mi_signalSem(){
    inside_sc--;
    if (!inside_sc){
        signalSem(mutex);
    }
}
