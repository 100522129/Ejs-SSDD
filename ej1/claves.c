#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "claves.h"

// Definimos el nodo de la lista enlazada
struct NodoTupla {
    char key[256];
    char value1[256];
    int N_value2;
    float V_value2[32];
    struct Paquete value3;

    struct NodoTupla *next; // Puntero al siguiente elemento de la lista enlazada
};

// Definimos la estructura de la lista enlazada
struct slist{
    // Puntero a la cabeza de la lista
    struct NodoTupla *head;
    // Mutex para que las operaciones sean atómicas
    pthread_mutex_t mutex_lista;
    // Tamaño de la lista
    int size;
};

struct slist almacen = { NULL, 0, PTHREAD_MUTEX_INITIALIZER };

// Función auxiliar para buscar un nodo (dada la clave)
struct NodoTupla* find_node(const char *key) {
   
    struct NodoTupla *actual = almacen.head;
   
    while (actual != NULL) {
        if (strcmp(actual->key, key)  == 0) {
            return actual; // El nodo actual es el buscado
        }
        actual = actual->next;
    }

    return NULL; // El nodo buscado no existe
}


// -----------------------------
//  IMPLEMENTACIÓN DE FUNCIONES
// -----------------------------

int destroy(void) {
    pthread_mutex_lock(&almacen.mutex_lista);

    struct NodoTupla *actual = almacen.head;
    struct NodoTupla *next;

    // Destruimos todas las tuplas almacenadas
    while (actual != NULL) {
        next = actual->next;
        free(actual);
        actual = next;
    }

    almacen.head = NULL;
    almacen.size = 0;
    pthread_mutex_unlock(&almacen.mutex_lista);

    return 0;
}


int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) { // Siervo
    
    return 0;
}


int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    pthread_mutex_lock(&almacen.mutex_lista);

    struct NodoTupla *node = find_node(key);

    if (node == NULL) {
        pthread_mutex_unlock(&almacen.mutex_lista);
        return -1; // No existe dicho elemento
    }

    strcpy(value1, node->value1);
    *N_value2 = node->N_value2;
    for (int i = 0; i < node->N_value2; i++) {
        V_value2[i] = node->V_value2[i];
    }
    *value3 = node->value3;

    pthread_mutex_unlock(&almacen.mutex_lista);
    return 0;
}


int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) { // Siervo

    return 0;
}


int delete_key(char *key) {
    pthread_mutex_lock(&almacen.mutex_lista);

    struct NodoTupla *actual = almacen.head;
    struct NodoTupla *prev = NULL;

    while (actual != NULL) {
        if (strcmp(actual->key, key) == 0) {
            if (prev == NULL) {
                almacen.head = actual->next;
            } else {
                prev->next = actual->next;
            }

            free(actual);
            almacen.size--;
            pthread_mutex_unlock(&almacen.mutex_lista);
            return 0;
        }
        prev = actual;
        actual = actual->next;
    }

    // En caso de que la clave no exista se devuelve -1
    pthread_mutex_unlock(&almacen.mutex_lista);
    return -1;
}


int exist(char *key) { // Siervo

    return 0;
}