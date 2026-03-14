#ifndef POOL_H
#define POOL_H

#include <pthread.h>
#include <semaphore.h>
#include "mensajes.h" // Necesario para 'struct peticion'

// Estructura necesaria para crear el pool de hilos
struct hilo_trabajador { 
    pthread_t thread_id;
    int ocupado;
    struct peticion tarea;
    int tareas_ejecutadas;
    sem_t semaforo;        // Empieza en 0
    pthread_mutex_t mutex; // Para que no haya condiciones de carrera al acceder a 'ocupado'
};

#endif // POOL_H