//Autores: Joan Balaguer, Eduardo Sánchez, Jaume Adrover
#include "simulacion.h"

#define DEBUGN11_v1 1
#define DEBUGN11_v2 0
int acabados=0;

void reaper(){
  pid_t ended;
  signal(SIGCHLD, reaper);
  while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
     acabados++;
#if DEBUGN11_v2
        fprintf(stderr, "Método simulación.c: Acabado proceso PID %d, total acabados: %d.\n", ended, acabados);
#endif
  }
}

int main(int argc,char const *argv[]){
    //asociar la señal SIGCHLD al enterrador
    signal(SIGCHLD, reaper);

    //comprobar sintaxis
    if(argc!=2){
        fprintf(stderr,"Error sintaxis: ./simulacion <disco>");
        return EXIT_FAILURE_1;
    }

    //montar el dispositivo
    if (bmount(argv[1]) == EXIT_FAILURE_1){
        fprintf(stderr,"Error al montar el dispositivo");
        return EXIT_FAILURE_1;
    }

    //Guardar tiempo dentro de una variable
    time_t t;
    time(&t);
    char camino[21] = ""; //longitud camino
    struct tm *tm = localtime(&t);
    sprintf(camino, "/simul_%d%02d%02d%02d%02d%02d/",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);


    if ((mi_creat(camino, 6)) < 0){
        fprintf(stderr, "./simulación.c: Error al crear el fichero del camino\n");
        return EXIT_FAILURE_1;
    }
    fprintf(stderr, "Directorio creado\n");

    for (int n = 1; n <= NUMPROCESOS; n++){
        pid_t pid = fork();
        if (pid == 0){
            bmount(argv[1]); //Montar el disco para el proceso hijo
            char nombredir[40];
            sprintf(nombredir, "%sproceso_%d/", camino, getpid());
            if (mi_creat(nombredir, 6) < 0){
                fprintf(stderr, "./simulación.c: Error al crear el fichero del proceso\n");
                bumount();
                exit(0);
            }
            //Creamos el fichero
            char nfichero[50];
            sprintf(nfichero, "%sprueba.dat", nombredir);
            if (mi_creat(nfichero, 6) < 0){
                fprintf(stderr, "./simulación.c: Error al crear el fichero del proceso\n");
                bumount();
                exit(0);
            }
#if DEBUGN11_v2
            fprintf(stderr, "Fichero del proceso %i creado\n", n);
#endif 
            srand(time(NULL) + getpid()); 
            for (int escritura = 1; escritura <= NUMESCRITURAS; escritura++){
                struct REGISTRO reg;
                reg.fecha = time(NULL);
                reg.pid = getpid();
                reg.nEscritura = escritura;
                reg.nRegistro = rand() % REGMAX;
                mi_write(nfichero, &reg, reg.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
#if DEBUGN11_v2
                fprintf(stderr, "[simulación.c → Escritura %i en %s]\n", escritura, nfichero);
#endif
                usleep(50000); //0,5 s

            }
#if DEBUGN11_v1
            fprintf(stderr, "[Proceso %d: Completadas %d escrituras en %s]\n", n, NUMESCRITURAS, nfichero);
#endif
            bumount();
            exit(0);
        }
        usleep(150000); //0,15 s
    }

    //mientras queden procesos por acabar
    while (acabados < NUMPROCESOS){
        pause();
    }

    //desmontar dispositivo
    if (bumount() < 0){
        fprintf(stderr, "Error al desmontar el dispositivo\n");
        exit(0);
    }

    fprintf(stderr, "Procesos terminados: %d\n", acabados);

    exit(0);
}