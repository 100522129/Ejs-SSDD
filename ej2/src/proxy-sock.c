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
#include "cJSON.h"

/* Notas Importantes:
    - El servidor escucha siempre por el mismo puerto
    - Ya no se envian structs, ahora se envian jsons
*/

char* crear_json (struct peticion req){
    // Comprobacion basica
    if (req.op < 0 || req.op > 5){
        return NULL;
    }

    // 1. Creo el JSON
    cJSON *obj = cJSON_CreateObject();

    if (req.op == 0){ // EL tipo de operacion es destroy
        // Relleno el JSON
        cJSON_AddNumberToObject(obj, "op", OP_INIT);
    }

    else if (req.op == 1){ // La operacion es set_value
        // Relleno el JSON
        cJSON_AddNumberToObject(obj, "op", OP_SET);
        cJSON_AddStringToObject(obj, "key", req.key);
        cJSON_AddStringToObject(obj, "value1", req.value1);
        cJSON_AddNumberToObject(obj, "nvalue2", req.N_value2);

        // Meto el array de floats V_value2
        cJSON *array = cJSON_CreateArray();
        for (int i = 0; i < req.N_value2; i++){
            cJSON_AddItemToArray(array, cJSON_CreateNumber(req.V_value2[i]));
        }
        cJSON_AddItemToObject(obj, "vvalue2", array);

        // Meto el value3
        cJSON *paquete = cJSON_CreateObject();
        cJSON_AddNumberToObject(paquete, "x", req.value3.x);
        cJSON_AddNumberToObject(paquete, "y", req.value3.y);
        cJSON_AddNumberToObject(paquete, "z", req.value3.z);
        cJSON_AddItemToObject(obj, "value3", paquete);
    }
   
    else if (req.op == 2){ // La operacion es get_value
        cJSON_AddNumberToObject(obj, "op", OP_GET);
        cJSON_AddStringToObject(obj, "key", req.key);
    }

    else if (req.op == 3){ // La operacion es modify_value
        // Relleno el JSON
        cJSON_AddNumberToObject(obj, "op", OP_MODIFY);
        cJSON_AddStringToObject(obj, "key", req.key);
        cJSON_AddStringToObject(obj, "value1", req.value1);
        cJSON_AddNumberToObject(obj, "nvalue2", req.N_value2);

        // Meto el array de floats V_value2
        cJSON *array = cJSON_CreateArray();
        for (int i = 0; i < req.N_value2; i++){
            cJSON_AddItemToArray(array, cJSON_CreateNumber(req.V_value2[i]));
        }
        cJSON_AddItemToObject(obj, "vvalue2", array);

        // Meto el value3
        cJSON *paquete = cJSON_CreateObject();
        cJSON_AddNumberToObject(paquete, "x", req.value3.x);
        cJSON_AddNumberToObject(paquete, "y", req.value3.y);
        cJSON_AddNumberToObject(paquete, "z", req.value3.z);
        cJSON_AddItemToObject(obj, "value3", paquete);
    }

    else if (req.op == 4){ // La operacion es delete_key
        cJSON_AddNumberToObject(obj, "op", OP_DELETE);
        cJSON_AddStringToObject(obj, "key", req.key);
    }

    else if (req.op == 5){ // La operacon es exist
        cJSON_AddNumberToObject(obj, "op", OP_EXIST);
        cJSON_AddStringToObject(obj, "key", req.key);
    }

    // Paso el JSON a char* 
    char* request_json = cJSON_PrintUnformatted(obj);

    // Liberamos memoria
    cJSON_Delete(obj);

    return request_json;
}

struct respuesta traducir_json_response (char* json_response){
    // 1. Parseo el JSON con cJSON

    // 2. Relleno y devuelvo la struct respuesta
    return;
}


// Función auxiliar:
// Envía la petición al servidor, espera la respuesta y la devuelve
int send_recv(char* request, char* response) {

    // 1. Leer IP_TUPLAS y PORT_TUPLAS del entorno

    // 2. Resolver el host con getaddrinfo

    // 3. Crear socket y hacer connect

    // 4. Enviar los 4 Bytes de longitud + el JSON request

    // 5. Recibir los 4 Bytes de longitud + el JSON response

    // 6. Cerrar el socket

    return 0;
}

int destroy(void) {

    struct peticion req;
    char res [4096];

    // 1. Empaqueto la petición
    req.op = OP_INIT;

    // 2. Creo el JSON para la request
    char* json_request = crear_json(req);

    // 3. Envio la request en formato JSON y recibo la response en formato JSON
    if (send_recv(json_request, res) == -2) return -2;

    free(json_request);

    // 4. Leo el JSON de la response y lo paso a struct response

    return;
}

int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {

    struct peticion req;
    char res [4096];

    // 1. Empaquetar la petición
    req.op = OP_SET;

    // 2. Comprobar de los valores de entrada
    if (N_value2 < 1 || N_value2 > 32) return -1;
    if (strnlen(key, 256) == 256 || (value1 != NULL && strnlen(value1, 256) == 256)) return -1;
    
    strcpy(req.key, key);
    strcpy(req.value1, value1);
    req.N_value2 = N_value2;
    memcpy(req.V_value2, V_value2, N_value2 * sizeof(float));
    req.value3 = value3;

    // 3. Paso la request a JSON
    char* json_request = crear_json(req);

    // 4. Envio la request en formato JSON y recibo la response en formato JSON
    if (send_recv(json_request, res) == -2) return -2;

    free(json_request);

    // 5. Leo el JSON de la response y lo paso a struct response

    return;
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {

    // Vital para que no falle si se pasa un NULL
    if (key == NULL) {
        return -1;
    }
    struct peticion req;
    char res [4096];

    // 1. Empaquetar la petición
    req.op = OP_GET;
    strcpy(req.key, key);
    
    // 2. Paso la request a JSON
    char* json_request = crear_json(req);
    
    // 2. Envio la request en formato JSON y recibo la response en formato JSON
    if (send_recv(json_request, res) == -2) return -2;

    free(json_request);
    
    // 3. Paso el JSON de la respones a struct respuesta

    // 4. Desempaquetar la respuesta (si la clave existe)
    if (res.resultado == 0) {
        strcpy(value1, res.value1);

        *N_value2 = res.N_value2;
        memcpy(V_value2, res.V_value2, res.N_value2 * sizeof(float));
        *value3 = res.value3;
    }

    // 5. Devolver el resultado de la respuesta
    return res.resultado;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    struct peticion req;
    char res [4096];

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
    
    // 2. Paso la request a JSON 
    char* json_request = crear_json(req);

    // 3. Envio la request en formato JSON y recibo la response en formato JSON
    if (send_recv(json_request, res) == -2) return -2;

    free(json_request);

    // 4. Paso el JSON de la respones a una struct respuesta
    return;
}

int delete_key(char *key) {

    // Vital para que no falle si se pasa un NULL
    if (key == NULL) {
        return -1;
    }
    struct peticion req;
    char res[4096];

    // 1. Empaquetar la petición
    req.op = OP_DELETE;
    strcpy(req.key, key);

    // 2. Paso la request a JSON
    char* json_request = crear_json(req);

    // 3.  Envio la request en formato JSON y recibo la response en formato JSON
    if (send_recv(json_request, res) == -2) return -2;

    free(json_request);

    // 4. Paso la response JSON a struct respuesta
    return;
}

int exist(char *key) {

    // Vital para que no falle si se pasa un NULL
    if (key == NULL) {
        return -1;
    }
    
    struct peticion req;
    char res [4096];

    // 1. Empaquetar la petición
    req.op = OP_EXIST;
    strcpy(req.key, key);

    // 2. Paso la request a JSON
    char* json_request = crear_json(req);

    // 3. Envio la request en formato JSON y recibo la response en formato JSON
    if (send_recv(json_request, res) == -2) return -2;

    free(json_request);
    
    // 4. Paso el JSON de la response a struct respuesta
    return;
}
