#include <mqueue.h>
#include <fcntl.h>           /* Para las constantes O_CREAT, O_RDONLY, etc. */
#include <sys/stat.h>        /* Para las constantes de permisos (0644) */
#include <stdio.h>
#include <stdlib.h>
#include "mensajes.h"
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "claves.h"
#include "pool.h"

struct hilo_trabajador *pool = NULL; // Puntero global para no tener que crear una estructura nueva que necesitaria el hilo para pasarle su id y su pool


int ejecutar_funcion (struct hilo_trabajador* datos){
    // 1. Se crea una copia local de la peticion
    struct peticion peticion = (datos->tarea);
    
    // 2. Se crea la respuesta que se le mandara al proxy
    struct respuesta* respuesta = malloc(sizeof(struct respuesta));
    if (respuesta == NULL){
        printf("Hola, soy el hilo [%ld] y no he podido generar la respuesta\n", datos->thread_id);
        return -1;
    }
    int codigo_respuesta;
    // 3. Se lanza la funcion del .so correspondiente a cada peticion 
    switch (peticion.op) {
        case OP_INIT: 
            respuesta->resultado = destroy();
            break;

        case OP_SET:
            codigo_respuesta = set_value(peticion.key,
                peticion.value1, 
                peticion.N_value2, 
                peticion.V_value2, 
                peticion.value3
            );
            respuesta->resultado = codigo_respuesta;
            // El resto de datos de la estructura se deja con los valores basura ya que no se usan
            break;

        case OP_GET:
            codigo_respuesta = get_value(
                peticion.key, 
                respuesta->value1, 
                &respuesta->N_value2, 
                respuesta->V_value2, 
                &respuesta->value3
            );
            respuesta->resultado = codigo_respuesta;
            break;

        case OP_MODIFY:
            codigo_respuesta = modify_value(peticion.key, 
                peticion.value1, 
                peticion.N_value2, 
                peticion.V_value2, 
                peticion.value3);
            respuesta->resultado = codigo_respuesta;
            break;

        case OP_DELETE:
            codigo_respuesta = delete_key(peticion.key);
            respuesta->resultado = codigo_respuesta;
            break;

        case OP_EXIST:
            codigo_respuesta = exist(peticion.key);
            respuesta->resultado = codigo_respuesta;
            break;
    }
    
    // 4. Se abre la cola del proxy 
    mqd_t q_proxy = mq_open(peticion.q_name, O_WRONLY);
    if (q_proxy == -1){
        printf("Soy el Hilo [%ld] y no he podido abrir la cola del proxy\n", datos->thread_id);
        mq_close(q_proxy);
        free(respuesta);
        return -1;
    }

    // 5. Se manda la respuesta al proxy via cola
    if (mq_send(q_proxy, (char*)respuesta, sizeof(*respuesta), 0) == -1){
        printf("Soy el Hilo [%ld] y no he podido escribir en la cola del proxy\n", datos->thread_id);
        mq_close(q_proxy);
        free(respuesta);
        return -1;
    }
    
    // 6. Limpieza
    mq_close(q_proxy);
    free(respuesta);

    // 7. El hilo vuelve al pool
    return 0;
    
}


void* trabajador (void* arg){ // Me llega un puntero void
    // 1. Se transforma puntero void* -> int*
    int* puntero_id = (int*)arg;

    // 2. Se desreferencia el puntero y se obtiene el int
    int id = *(puntero_id);

    // 3. Se guardan los datos unicos para cada hilo
    struct hilo_trabajador *mis_datos = &pool[id];

    // 4. Inicia la logica de los trabajadores
    while (1){
        // 4.1 Espera a que el semaforo este en verde
        sem_wait(&mis_datos->semaforo);

        // 4.2 Se ejecuta la funcion del hilo trabajador
        printf("Hilo %ld trabajando...\n", mis_datos->thread_id);
        if (ejecutar_funcion(mis_datos) == -1){
            fprintf(stderr, "[!] Hilo %ld falló al contestar al cliente. Retomando servicio...\n", mis_datos->thread_id);
        }
        printf("Hilo %ld ha finalizado su trabajo\n", mis_datos->thread_id);
        // 4.3 Se notifica al hilo jefe que el hilo queda libre
        pthread_mutex_lock(&mis_datos->mutex);
        mis_datos->ocupado = 0;
        pthread_mutex_unlock(&mis_datos->mutex);
    }

}

int main(){
    // 1. Se crea la cola POSIX del servidor
    mqd_t q_server; // Este es el ID de la cola
    struct mq_attr attr;

    // 2. Se configuran los atributos de la cola
    attr.mq_flags = 0; // Comportamiento normal (bloqueante)
    attr.mq_maxmsg = 10; // Limite de mensajes en la cola a la vez
    attr.mq_msgsize = sizeof(struct peticion); // El tamannio de cada mensaje es exactamente el de cada peticion
    attr.mq_curmsgs = 0;

    // 3. Se inicializa la cola
    char* nombre_cola = "/SERVIDOR";
    q_server = mq_open(nombre_cola, O_CREAT | O_RDONLY, 0777, &attr); // Cambiar permisos mas adelante
    if (q_server == (mqd_t)-1) { // Hago el cast por si acaso
        perror("Error al crear la cola POSIX del servidor");
        exit(-1);
    }
    printf("[SERVIDOR] Cola '/SERVIDOR' inicializada correctamente. Esperando clientes...\n");

    // 4. Se define el contenedor para almacenar cada peticion
    struct peticion peticion_actual; // Aqui vamos a guardar los datos que saquemos de la cola

    // 5. Se crea el pool de hilos con malloc para poder acceder desde fuera de la funcion  
    long NUM_THREADS = sysconf(_SC_NPROCESSORS_ONLN); // 5.1 Se define el numero maximo de hilos del PC
    pool = malloc(sizeof(struct hilo_trabajador) * NUM_THREADS);
    if (!pool){
        perror("Error al crear el pool de hilos");
        exit(-1);
    }
    // Este array es VITAL. Cada casilla guarda el número de ID de forma permanente. Si al hilo le paso la variable i del bucle se tensa
    int threads_IDs [NUM_THREADS];

    // 6. Se inicializa el pool de hilos
    for (size_t i = 0; i < NUM_THREADS; i++){
        // 6.1 Se inicializa el estado del hilo
        pool[i].ocupado = 0; // Inicialmente esta libre

        // 6.2 Inicalizar a 0 el semaforo del hilo
        if (sem_init(&pool[i].semaforo, 0, 0) != 0){
            perror("Error inicializando semáforo");
            exit(-1);
        }
        
        // 6.3 Se inicializa el mutex
        pthread_mutex_init(&pool[i].mutex, NULL);

        // 6.4 Se guarda el ID del hilo en el array
        threads_IDs [i] = i;

        // 6.5 Se crea el hilo y se manda a trabajar
        if (pthread_create(&pool[i].thread_id, NULL, trabajador, &threads_IDs[i]) != 0){
            perror("Error creando hilo");
            exit(-1);
        }
    }
    printf("[JEFE] Pool de %ld hilos creado e inicializado.\n", NUM_THREADS);

    // ------------------------------
    // Empieza la logica del servidor 
    // ------------------------------
    
    while (1){
        // 1. Se lee de la cola POSIX y se almacena el mensaje en la variable peticion_actual
        if (mq_receive(q_server, (char*)&peticion_actual, sizeof(struct peticion), NULL) == -1){
            perror("Error al recibir la peticion");
            continue;
        }
        // 2. Se pasa la peticion a un hilo trabajador
        int asignado = 0;
        while (!asignado){
            for (int i = 0; i < NUM_THREADS; i++){
                pthread_mutex_lock(&pool[i].mutex);
                if (pool[i].ocupado == 0){
                    pool[i].ocupado = 1;
                    pool[i].tarea = peticion_actual;
                    pthread_mutex_unlock(&pool[i].mutex);
                    sem_post(&pool[i].semaforo);
                    asignado = 1;
                    break;
                }
                pthread_mutex_unlock(&pool[i].mutex);
            }
            if (asignado == 0){
                usleep(1000);
            } 
        }
    }

    // Limpieza
    mq_close(q_server);
    mq_unlink("/SERVIDOR");
    free(pool);
}