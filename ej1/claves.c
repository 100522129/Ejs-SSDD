#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "claves.h"

// Definimos el nodo de la lista enlazada
struct NodoTupla {
    char key[256]; // Clave
    // Valores asociados a la clave:
    char value1[256]; 
    int N_value2; // Longitud del vector V_value2 (1 <= N <= 32)
    float V_value2[32]; // Vector con los elementos
    struct Paquete value3; // Paquete esta compuesto por: int x,y,z

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
    // Esclavo = Pablo Pino 
    // Rey = Pablo Perez 👑
// -----------------------------

int destroy(void) { // Esclavo
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
// Falta ver que hacemos cuadno hay un error al insertar un nodo
int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) { // Rey

    // 1. Se comprueba que el valor N_value2 este dentro del rango
    if (N_value2 < 1 || N_value2 > 32) {
        return -1;
    }

    // 2. Se comprueba que las cadenas de chars acaban en \0
    if (strnlen(key, 256) == 256 || strnlen(value1, 256) == 256) {
        return -1;
    }

    // 3. Se bloquea la lista
    pthread_mutex_lock (&almacen.mutex_lista);

    // 4. Se comprueba que no exsita la clave
    if (exist(key) == 0){
        pthread_mutex_unlock (&almacen.mutex_lista);
        return -1;
    }

    // 5. Se crea el nodo (se usa malloc para que sobreviva fuera de la funcion, lo dejamos en el Heap)
    struct NodoTupla* nodo = malloc (sizeof(struct NodoTupla));

    // 6. Se insertan los valores en el nodo
    strcpy(nodo->key, key);
    strcpy(nodo->value1, value1);
    nodo->N_value2 = N_value2;
    memcpy(nodo->V_value2, V_value2, N_value2 * sizeof(float));
    nodo->value3 = value3;
    nodo->next = NULL; // Cuando se inserte en la lista cambiara su valor

    // 7. Se inserta el nodo en la SList
    if (insert_node(nodo) == -1){
        free (nodo);
        pthread_mutex_unlock (&almacen.mutex_lista);    
        return -1;
    }

    // 8. Liberar y salir
    pthread_mutex_unlock (&almacen.mutex_lista);
    return 0;

}

int insert_node (struct NodoTupla* nodo){ // Rey
    // 1. Se comprueba que el nodo sea valido
    if (nodo == NULL) {
        return -1;
    }
    // Caso 1: Lista vacia
    if (almacen.size == 0){
        almacen.head = nodo;
        almacen.size +=1;
        return 0;
    }
    // Caso 2: Lista no vacia (insertamos al principio por eficiencia)
    nodo->next = almacen.head;
    almacen.head = nodo;
    almacen.size +=1;
    return 0; // Se puede refactorizar pero de momento se deja asi por legibilidad
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) { // Esclavo
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

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) { // Rey
    // 1. Se comprueba que las cadenas de chars acaban en \0
    if (strnlen(key, 256) == 256 || strnlen(value1, 256) == 256) {
        return -1;
    }
    // 2. Se comprueba que el valor N_value2 este dentro del rango
    if (N_value2 < 1 || N_value2 > 32) {
        return -1;
    }
    // 3. Bloqueo de la lista
    pthread_mutex_lock(&almacen.mutex_lista);

    // 4. Se busca el nodo a modificar
    struct NodoTupla* nodo = find_node(key); // IMPORTANTE PUNTERO para no hacer una copia local del nodo
    if (nodo == NULL) {
        return -1;
    }

    // 5. Se modifican los valores del nodo
    strcpy (nodo->key, key);
    strcpy (nodo->value1, value1);
    nodo->N_value2 = N_value2;
    memcpy (nodo->V_value2, V_value2, N_value2 * sizeof(float));
    nodo->value3 = value3;

    // 6. Liberar y salir
    pthread_mutex_unlock(&almacen.mutex_lista);
    return 0;
}

int delete_key(char *key) { // Esclavo
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
    return 1;
}

int exist(char *key) { // Rey
    if (find_node (key)){
       return 0; // Retorna 0 si existe
    }
    return -1; // Retorna 1 si no existe
}
