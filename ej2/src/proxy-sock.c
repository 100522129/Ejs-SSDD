#include <stdio.h>       // sprintf, printf
#include <stdlib.h>      // getenv, atoi
#include <string.h>      // strlen, memset, strcmp
#include <unistd.h>      // close, read, write
#include <sys/socket.h>  // socket, connect, send, recv
#include <netinet/in.h>  // struct sockaddr_in
#include <arpa/inet.h>   // htons, inet_pton
#include <netdb.h>       // getaddrinfo (para resolver nombres de dominio)
#include "claves.h"
#include "mensajes.h"


/* Notas Importantes:
    - El servidor escucha siempre por el mismo puerto
    - Ya no se envian structs, ahora se envian jsons
*/


// Función auxiliar:
// Envía la petición al servidor, espera la respuesta y la devuelve
int send_recv(char* request, char* response) {
    
    // 1. Conectar
    

    // 2. Enviar y recibir (usamos JSONS)


    // 3. Limpieza
    
    return 0;
}

int destroy(void) {
    /*
    * Request:
    * {
    *     "op": "DESTROY"
    * }
    * Response:
    * {
    *     "result": 0    (ok)
    *     "result": -1   (error)
    * }
    */
    struct peticion req;
    struct respuesta res;

    // 1. Empaquetar la petición
    req.op = OP_INIT;

    // 2. Enviar y recibir
    if (send_recv(&req, &res) == -2) return -2;

    return res.resultado;
}

int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    /*
    * Request:
    * {
    *     "op":      "SET",
    *     "key":     "<clave>",
    *     "value1":  "<string[256]>",
    *     "nvalue2": <int[1-32]>,
    *     "vvalue2": [<float>, <float>, ...],
    *     "value3":  { <campos de Paquete> }
    * }
    * Response:
    * {
    *     "result": 0    (ok)
    *     "result": -1   (clave ya existe o N_value2 fuera de rango)
    * }
    */



    struct peticion req;
    struct respuesta res;

    // 1. Empaquetar la petición
    req.op = OP_SET;

    // Comprobar de los valores de entrada
    if (N_value2 < 1 || N_value2 > 32) return -1;
    if (strnlen(key, 256) == 256 || (value1 != NULL && strnlen(value1, 256) == 256)) return -1;
    
    strcpy(req.key, key);
    strcpy(req.value1, value1);
    req.N_value2 = N_value2;
    memcpy(req.V_value2, V_value2, N_value2 * sizeof(float));
    req.value3 = value3;

    // 2. Enviar y recibir
    if (send_recv(&req, &res) == -2) return -2;

    return res.resultado;
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    /*
    * Request:
    * {
    *     "op":  "GET",
    *     "key": "<clave>"
    * }
    * Response:
    * {
    *     "result":  0,
    *     "value1":  "<string[256]>",
    *     "nvalue2": <int>,
    *     "vvalue2": [<float>, <float>, ...],
    *     "value3":  { <campos de Paquete> }
    * }
    * {
    *     "result": -1   (clave no existe)
    * }
    */




    // Vital para que no falle si se pasa un NULL
    if (key == NULL) {
        return -1;
    }
    struct peticion req;
    struct respuesta res;

    // 1. Empaquetar la petición
    req.op = OP_GET;
    strcpy(req.key, key);
    
    // 2. Enviar y recibir
    if (send_recv(&req, &res) == -2) return -2;

    // 3. Desempaquetar la respuesta (si la clave existe)
    if (res.resultado == 0) {
        strcpy(value1, res.value1);

        *N_value2 = res.N_value2;
        memcpy(V_value2, res.V_value2, res.N_value2 * sizeof(float));
        *value3 = res.value3;
    }

    // 4. Devolver el resultado de la respuesta
    return res.resultado;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {

    /*
    * Request:
    * {
    *     "op":      "MODIFY",
    *     "key":     "<clave>",
    *     "value1":  "<string[256]>",
    *     "nvalue2": <int[1-32]>,
    *     "vvalue2": [<float>, <float>, ...],
    *     "value3":  { <campos de Paquete> }
    * }
    * Response:
    * {
    *     "result": 0    (ok)
    *     "result": -1   (clave no existe o N_value2 fuera de rango)
    * }
    */
    struct peticion req;
    struct respuesta res;

    // 1. Empaquetar la petición
    req.op = OP_MODIFY;

    // Comprobar los valores de entrada
    if (N_value2 < 1 || N_value2 > 32) return -1;
    if (strnlen(key, 256) == 256 || (value1 != NULL && strnlen(value1, 256) == 256)) return -1;
    
    strcpy(req.key, key);
    strcpy(req.value1, value1);
    req.N_value2 = N_value2;
    memcpy(req.V_value2, V_value2, N_value2 * sizeof(float));
    req.value3 = value3;
    
    // 2. Enviar y recibir
    if (send_recv(&req, &res) == -2) return -2;

    return res.resultado;
}

int delete_key(char *key) {

    /*
    * Request:
    * {
    *     "op":  "DELETE",
    *     "key": "<clave>"
    * }
    * Response:
    * {
    *     "result": 0    (ok)
    *     "result": -1   (clave no existe)
    * }
    */

    // Vital para que no falle si se pasa un NULL
    if (key == NULL) {
        return -1;
    }
    struct peticion req;
    struct respuesta res;

    // 1. Empaquetar la petición
    req.op = OP_DELETE;
    strcpy(req.key, key);

    // 2. Enviar y recibir
    if (send_recv(&req, &res) == -2) return -2;

    return res.resultado;
}

int exist(char *key) {

    /*
    * Request:
    * {
    *     "op":  "EXIST",
    *     "key": "<clave>"
    * }
    * Response:
    * {
    *     "result": 1    (existe)
    *     "result": 0    (no existe)
    *     "result": -1   (error de comunicaciones)
    * }
    */

    // Vital para que no falle si se pasa un NULL
    if (key == NULL) {
        return -1;
    }
    
    struct peticion req;
    struct respuesta res;

    // 1. Empaquetar la petición
    req.op = OP_EXIST;
    strcpy(req.key, key);

    // 2. Enviar y recibir
    if (send_recv(&req, &res) == -2) return -2;

    return res.resultado;
}
