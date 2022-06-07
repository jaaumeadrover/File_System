Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
Pràctica Final SO2
Participantes: Edu Sánchez, Joan Balaguer, Jaume Adrover

ENTREGA 1:
En general no hay muchas observaciones a destacar, hemos intentado la visualización de mensajes
por pantalla, con una constante muy útil que nos proporciona el nombre del método actual desde el
que se llama("__func__").Además, no hemos sabido optimizar el algoritmo iterativo, ya que utilizábamos
un switch para poder avanzar los bloques según el respectivo nivel y no se optimiza el número de lecturas.

ENTREGA 2:
Una mejora es, en buscar_entrada y mi_dir, utilizar un buffer de entradas de tamaño BLOCKSIZE y 
explorar éstas en memoria (en vez de leer 1 sola entrada cada vez), y así no tener que acceder al dispositivo para cada una de ellas.

ENTREGA 3:
1.Hemos realizado la granulación en mi_write_f() y en mi_read_f para que sólo afecte a la porcion de código donde se actualiza la información
del inodo (leyendo primero de nuevo el inodo y escribiéndolo justo a continuación).
2.Hemos utilizado un buffer de tamaño NUMPROCESOS * (sizeof struct entrada) para leer de golpe todas las entradas.
3.Hemos utilizado un buffer de registros de N=256 para no leer cada registro de uno en uno.