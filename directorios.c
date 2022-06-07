//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "directorios.h"


//static struct UltimaEntrada UltimaEntradaEscritura;
/*
*Función: extraer_camino
------------------------------------------------------
*params:const char *camino, char *inicial, char *final, char *tipo
*
*Dada una cadena de caracteres camino (que comience por '/'), separa su contenido en dos.
*
*Devuelve en la variable 'tipo' si el archivo cuya ruta es la que devuelve 'inicial' es un fichero o un directorio.
*/
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
    if (camino[0]!='/'){ //si no empieza por '/' es incorrecto
        return EXIT_FAILURE_1;
    }
    //encontrar segundo carácter '/' 
    char *barra= strchr(camino+1,'/');
     
    if (barra){ //se ha encontrado segunda barra, por lo que dividimos 
        
        strncpy(inicial, (camino + 1), (strlen(camino) - (strlen(barra) + 1)));
        strcpy(final, barra);

        if(final[0]=='/'){
            strcpy(tipo,"d");//es un directorio
        }else{
            strcpy(tipo,"f");//es un fichero
        }

    }else{
        strcpy(tipo,"f");//es un fichero
        strcpy(inicial,camino+1);
        strcpy(final,"");//final=""
    }
    return EXIT_SUCCESS;
}

void mostrar_error_buscar_entrada(int error) {
   //casos de error
   switch (error) {
   case -1: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
   case -2: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
   case -3: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
   case -4: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
   case -5: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
   case -6: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
   case -7: fprintf(stderr, "Error: No es un directorio.\n"); break;
   }
}

 /**
 *  Función: buscar_entrada
 * ---------------------------------------------------------------------
 * params: const char *camino_parcial, unsigned int *p_inodo_dir, 
 * unsigned int *p_inodo,unsigned int *p_entrada, char reservar, unsigned char permisos
 *
 * Esta función nos buscará una determinada entrada (la parte *inicial 
 * del *camino_parcial que nos devuelva extraer_camino()) entre las entradas 
 * del inodo correspondiente a su directorio padre (identificado con *p_inodo_dir). 
 *
 * Devuelve EXIT_SUCCESS en caso de que todo haya ido bien o EXIT_FAILURE_1 en caso de error 
 * (con su correspondiente mensaje de error)
 */

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
    //Declaraciones
    struct superbloque SB;
    struct entrada entrada;
    struct inodo inodo_dir;
    struct entrada buffer_lectura[BLOCKSIZE / sizeof(struct entrada)]; //buffer entradas
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int numEntradas = BLOCKSIZE / sizeof(struct entrada); //tamaño buffer
    int num_entradas_inodo;
    int num_entrada_inodo = 0; //variable que itera el buffer y que cuenta el numero de entrada

    //leemos el superbloque
    if (bread(posSB, &SB) == EXIT_FAILURE_1){
        fprintf(stderr, "Error en el bread de la función buscar_entrada \n");
        return EXIT_FAILURE_1;
    }

    //Comrobamos si es el directorio raiz
    if (!strcmp(camino_parcial, "/")){
        *p_inodo = SB.posInodoRaiz; // nuestra raiz siempre estará asociada al inodo 0
        *p_entrada = 0;
        return EXIT_SUCCESS;
    }
    
    memset(inicial, 0, sizeof(inicial));
    memset(final, 0, sizeof(final));

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == EXIT_FAILURE_1){
        return ERROR_CAMINO_INCORRECTO;
    }
    
#if DEBUGN7
    fprintf(stderr, "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, (int)reservar);
#endif

    if(leer_inodo(*p_inodo_dir, &inodo_dir)==EXIT_FAILURE_1){   //leemos inodo
        return EXIT_FAILURE_1;
    }

    if ((inodo_dir.permisos & 4) != 4){ //tiene permisos de lectura?
        return ERROR_PERMISO_LECTURA;
    }

    // inicializamos  buffer de lectura con 0
    memset(buffer_lectura, 0, BLOCKSIZE);
    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    
    num_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada); //entradas en un inodo

    if (num_entradas_inodo > 0){
        mi_read_f(*p_inodo_dir, buffer_lectura, 0, BLOCKSIZE);
        entrada = buffer_lectura[0];
    }


      while ((num_entrada_inodo < num_entradas_inodo) && strcmp(inicial, entrada.nombre) != 0){ //buscamos entrada que coincide con inicial
        num_entrada_inodo++;
        if ((num_entrada_inodo % numEntradas) == 0){ //hay que volver a leer del buffer
            memset(buffer_lectura, 0, BLOCKSIZE);
            memset(entrada.nombre, 0, sizeof(entrada.nombre));
            mi_read_f(*p_inodo_dir, buffer_lectura, num_entrada_inodo * sizeof(struct entrada), BLOCKSIZE);
            entrada = buffer_lectura[num_entrada_inodo % numEntradas];
        }
        else{ 
            entrada = buffer_lectura[num_entrada_inodo % numEntradas];
        }
    }
    

    if (strcmp(inicial, entrada.nombre))    { // la entrada no existe
        switch (reservar)
        {
        case 0:
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            break;
        case 1:
            if (inodo_dir.tipo == 'f'){
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            if ((inodo_dir.permisos & 2) != 2){
                return ERROR_PERMISO_ESCRITURA;
            }
            else{
                strcpy(entrada.nombre, inicial);
                if (tipo == 'd')
                {
                    if (!strcmp(final, "/"))
                    {
                        entrada.ninodo = reservar_inodo('d', permisos);
#if DEBUGN7
                        fprintf(stderr, "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n", entrada.ninodo, tipo, permisos, inicial);
#endif
                    }
                    else {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else{
                    entrada.ninodo = reservar_inodo('f', permisos);
#if DEBUGN7
                    fprintf(stderr, "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n", entrada.ninodo, tipo, permisos, inicial);
#endif
                }

                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == EXIT_FAILURE_1){
                    if (entrada.ninodo != -1)
                    { // se había reservado un inodo para la entrada
                        liberar_inodo(entrada.ninodo);
                    }
                    return EXIT_FAILURE_1;
                }
#if DEBUGN7
                fprintf(stderr, "[buscar_entrada()→ creada entrada: %s, %d]\n", entrada.nombre, entrada.ninodo);
#endif
            }
            break;
        }
    }
    if ((strcmp(final, "/") == 0) || strcmp(final, "") == 0){
        if ((num_entrada_inodo < num_entradas_inodo) && (reservar == 1)){
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return EXIT_SUCCESS;
    }
    else{
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return EXIT_SUCCESS;
}


 /**
 *  Función: mi_create
 * ---------------------------------------------------------------------
 * params: const char *camino, unsigned char permisos
 * 
 * Esta función crea un fichero/directorio y su entrada de directorio.
 *
 * Devuelve EXIT_SUCCESS en caso de que todo haya ido bien o EXIT_FAILURE_1 en caso de error 
 * (con su correspondiente mensaje de error)
 */
int mi_creat(const char *camino, unsigned char permisos){
    mi_waitSem();
    //suponemos p_inodo_dir a 0 por simplicidad
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int aux;
    aux = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    if(aux < 0){
        mi_signalSem();
        return aux;
    }
    mi_signalSem();
    return EXIT_SUCCESS;
}


 /**
 *  Función: mi_dir
 * ---------------------------------------------------------------------
 * params: const char *camino, unsigned char permisos
 * 
 * Esta función pone el contenido del directorio en un buffer 
 * de memoria (el nombre de cada entrada puede venir separado por '|' 
 * o por un tabulador) y devuelve el número de entradas. Implica leer de forma secuencial el contenido de un inodo de tipo directorio, con mi_read_f() leyendo sus entradas.
 *
 * Devuelve EXIT_SUCCESS en caso de que todo haya ido bien o EXIT_FAILURE_1 en caso de error 
 * (con su correspondiente mensaje de error)
 */
int mi_dir(const char *camino, char *buffer){

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;   
    int entradas_inodo,entradas_bloque;
    struct inodo inodo;
    int offset=0;
    

    int aux=buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,4); //sin reservar y con permisos de lectura
    if (aux<0){ //error?
        return aux;
    }

    if (leer_inodo(p_inodo,&inodo)==EXIT_FAILURE_1){ //leer inodo
        return EXIT_FAILURE_1;
    }

    if (inodo.tipo!= 'd' || (inodo.permisos & 4) != 4){ //si no es un directorio o no tiene permisos de lectura error
        return EXIT_FAILURE_1;
    }

    entradas_inodo = inodo.tamEnBytesLog / sizeof(struct entrada); //entradas en un inodo
    entradas_bloque=BLOCKSIZE / sizeof(struct entrada);

    struct entrada buffer_lectura[entradas_bloque];
    memset(buffer_lectura, 0, BLOCKSIZE );

    for (int i = 0; i < entradas_inodo; i++){           

        if (i % (entradas_bloque) == 0) //hemos llegado al final del buffer de entradas o leemos por primera vez
            {
               offset += mi_read_f(p_inodo, buffer_lectura, offset, BLOCKSIZE); //leemos entradas
            }       
            
        strcat(buffer, buffer_lectura[i % entradas_bloque].nombre); //concatenamos entradas
        strcat(buffer, "\n");
    }
    return entradas_inodo;
}


/*
*  Función: mi_chmod
------------------------------------------------------
*  params:const char *camino, unsigned cha permisos
*
*  Buscar la entrada *camino con buscar_entrada() para obtener el nº de inodo (p_inodo).
*  Si la entrada existe llamamos a la función correspondiente de ficheros.c pasándole el p_inodo
*
*  Devuelve EXIT_SUCCESS si se ha ejecutado correctamente o -1,-2,-3 según que tipo de error haya salido.
*/
int mi_chmod(const char *camino, unsigned char permisos){
    unsigned int p_inodo_dir=0;
    unsigned int p_inodo=0;
    unsigned int p_entrada=0;
    int aux;
    if ((aux = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos)) < 0){
        return aux;
    }
    if(mi_chmod_f(p_inodo,permisos)){
        return EXIT_FAILURE_1;
    }

    return EXIT_SUCCESS;
}

/*
*Función: mi_stat
------------------------------------------------------
*params:const char *camino, struct STAT *p_stat
*
*Buscar la entrada *camino con buscar_entrada() para obtener el p_inodo. Si la entrada
*existe llamamos a la función correspondiente de ficheros.c pasándole el p_inodo.
*
* Devuelve p_inodo si se ha ejecutado correctamente.O EXIT_FAILURE_1 en caso de error
*/
int mi_stat(const char *camino, struct STAT *p_stat){
    //Declaraciones
    unsigned int p_inodo_dir=0;
    unsigned int p_inodo=0;
    unsigned int p_entrada=0;
    int aux=buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,p_stat->permisos);

    if(aux<0){//existe la entrada?
        return aux;
    }

    if(mi_stat_f(p_inodo,p_stat)==EXIT_FAILURE_1){
        return EXIT_FAILURE_1;
    }
    
    return p_inodo;
}

/*
*Función: mi_write
------------------------------------------------------
* params:const char *camino, const void *buf, unsigned int offset, unsigned int nbytes
*
* Escribe contenido en un fichero. 
*
* Devuelve el numero de bytes escritos
*/
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
    //DECLARACIONES
    unsigned int p_inodo=0;
    unsigned int p_inodo_dir=0;
    unsigned int p_entrada=0;
    int aux=0;
    int bytes_escritos=0;

    aux=buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,4);
    if(aux<0){
        return aux;
    }
    bytes_escritos+=mi_write_f(p_inodo,buf,offset,nbytes);

    return bytes_escritos;
}

/**
 Función: mi_read
------------------------------------------------------
* params:const char *camino, void *buf, unsigned int offset, unsigned int nbytes
*
* Lee los nbytes del fichero indicado por el camino pasado por parámetro, a partir de
* offset y copiarlos en el buffer buf.
*
* Devuelve el numero de bytes leidos
*/
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes){
    //DECLARACIONES
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int aux = 0;
    int bytes_leidos = 0;

    // Realiza la lectura del archivo.
    aux=buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,4);
    if(aux<0){
        return aux;
    }

    bytes_leidos = mi_read_f(p_inodo, buf, offset, nbytes);

    return bytes_leidos;
        
}
/** 
  Función: mi_link
------------------------------------------------------
* params: const char *camino1, const char *camino2
*
* Crea el enlace de una entrada de directorio camino2 
* al inodo especificado por otra entrada de directorio camino1 .
*
* Devuelve EXIT_FAILURE_1 en caso de error y EXIT_SUCCES en caso de que
* tdo haya ido bien
*/
int mi_link(const char *camino1, const char *camino2){
    mi_waitSem();
    //DECLARACIONES
    unsigned int p_inodo_dir = 0,p_inodo_dir2=0;
    unsigned int p_inodo = 0,p_inodo2=0;
    unsigned int p_entrada = 0,p_entrada2=0;
    struct inodo inodo,inodo2;
    int aux=0;
    struct entrada entrada;

    //CAMINO 1
    aux=buscar_entrada(camino1,&p_inodo_dir,&p_inodo,&p_entrada,0,4);
    if(aux<0){
        mostrar_error_buscar_entrada(aux);
        mi_signalSem();
        return aux;
    }

    if(leer_inodo(p_inodo, &inodo)==EXIT_FAILURE_1){   //leemos inodo
        mi_signalSem();
        return EXIT_FAILURE_1;
    }

    if (inodo.tipo!= 'f' || (inodo.permisos & 4) != 4){ //si no es un directorio o no tiene permisos de lectura error
        mi_signalSem();
        return EXIT_FAILURE_1;
    }


    //CAMINO 2
    aux=buscar_entrada(camino2,&p_inodo_dir2,&p_inodo2,&p_entrada2,1,6);
    if(aux<0){
        mostrar_error_buscar_entrada(aux);
        mi_signalSem();
        return aux;
    }

    if(leer_inodo(p_inodo2, &inodo2)==EXIT_FAILURE_1){   //leemos inodo
        fprintf(stderr,"ERROR LECTURA: método %s",__func__);
        mi_signalSem();
        return EXIT_FAILURE_1;
    }

    if (inodo2.tipo!= 'f' || (inodo2.permisos & 4) != 4){ //si no es un directorio o no tiene permisos de lectura error
        mi_signalSem();
        return EXIT_FAILURE_1;
    }

    memset(&entrada, 0, sizeof(struct entrada));

    aux= mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada));

    if(aux<0){
        mi_signalSem();
        return aux;
    }

    entrada.ninodo=p_inodo;

    aux= mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada));

    if(aux<0){
        mi_signalSem();
        return aux;
    }
    
    if(liberar_inodo(p_inodo2)==EXIT_FAILURE_1){
        fprintf(stderr,"Error al liberar_inodo() en el método %s",__func__);
        mi_signalSem();
        return EXIT_FAILURE_1;}

    inodo.nlinks++;
    inodo.ctime=time(NULL);

    if(escribir_inodo(p_inodo,inodo)==EXIT_FAILURE_1){
        mi_signalSem();
        return EXIT_FAILURE_1;
    }
    mi_signalSem();
    return EXIT_SUCCESS;
}


/** 
  Función: mi_unlink
------------------------------------------------------
* params: const char *camino1, const char *camino2
*
* Función de la capa de directorios que borra la entrada 
* de directorio especificada (no hay que olvidar actualizar 
* la cantidad de enlaces en el inodo) y, en caso de que fuera el 
* último enlace existente, borrar el propio fichero/directorio.
* Es decir que esta función nos servirá tanto para borrar un enlace 
* a un fichero como para eliminar un fichero o directorio que no contenga enlaces.
*
* Devuelve EXIT_FAILURE_1 en caso de error y EXIT_SUCCES en caso de que
* tdo haya ido bien
*/
 int mi_unlink(const char *camino){
    mi_waitSem();
    //DECLARACIONES
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    struct inodo inodo;
    struct  inodo inodo_dir;
    struct entrada entrada;
    int nEntradas=0;
    int aux=0;
    int liberados=0;

    aux=buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,4);
    if(aux<0){
        mostrar_error_buscar_entrada(aux);
        mi_signalSem();
        return aux; //error
    }

    if(leer_inodo(p_inodo,&inodo)==EXIT_FAILURE_1){
        fprintf(stderr,"Error lectura en el método %s()",__func__);
        mi_signalSem();
        return EXIT_FAILURE_1; //error
    }

    //si se trata de un directorio y no esta vacio
    if((inodo.tipo=='d') && (inodo.tamEnBytesLog >0 )){
        fprintf(stderr,"Error: el directorio %s no está vacio.\n",camino);
        mi_signalSem();
        return EXIT_FAILURE_1; //error
    }else{
        
        if(leer_inodo(p_inodo_dir,&inodo_dir)==EXIT_FAILURE_1){ //leemos inodo
            fprintf(stderr,"Error lectura en el método %s()",__func__);
            mi_signalSem();
            return EXIT_FAILURE_1; //error
        }
        
        nEntradas=(inodo_dir.tamEnBytesLog/sizeof(struct entrada)); //calculamos num entradas
        if(nEntradas!=p_entrada){
            if(mi_read_f(p_inodo_dir, &entrada, (nEntradas - 1) * sizeof(struct entrada), sizeof(struct entrada))==EXIT_FAILURE_1){
                fprintf(stderr,"Error lectura en el método: %s()",__func__);
                mi_signalSem();
                return EXIT_FAILURE_1;
            }
            if(mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada))==EXIT_FAILURE_1){
                fprintf(stderr,"Error escritura en el método: %s()",__func__);
                mi_signalSem();
                return EXIT_FAILURE_1;
            }
        }
        liberados += mi_truncar_f(p_inodo_dir, (nEntradas - 1) * sizeof(struct entrada));
        inodo.nlinks--;

        if (inodo.nlinks == 0){
            if (liberar_inodo(p_inodo) < 0){ //liberamos inodo
                fprintf(stderr, "error en liberar_inodo de mi_unlink.\n");
                mi_signalSem();
                return EXIT_FAILURE;
            }
        }
        else{          
            inodo.ctime = time(NULL); //actualizamos ctime
            if (escribir_inodo(p_inodo, inodo) == EXIT_FAILURE_1){
                mi_signalSem();
                return EXIT_FAILURE_1;}
        }
    }
    mi_signalSem();
    return liberados;
 }