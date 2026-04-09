#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "claves.h"
#include "mensajes.h"
#include "cJSON.h"

/* Notas Importantes:
    - El servidor escucha siempre por el mismo puerto
    - Ya no se envian structs, ahora se envian jsons
*/

char* crear_json (struct peticion req){ // Falta tratar errores y refactorizar
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

struct respuesta traducir_json_response (char* json_response){ // Falta tratar errores
    // 1. Parseo el JSON con cJSON
    cJSON *obj = cJSON_Parse(json_response);

    // 2. Relleno los campos de la respuesta
    struct respuesta respuesta_servidor;
    respuesta_servidor.resultado = cJSON_GetObjectItem(obj, "result")->valueint;

    // 2.1 Miro si la operacion es get_value (es la unica que tiene mas campos)
    if (cJSON_GetArraySize(obj)> 1){
        // Relleno value1
        strcpy(respuesta_servidor.value1, cJSON_GetObjectItem(obj, "value1")->valuestring);
        // Relleno nvalue2
        respuesta_servidor.N_value2 = cJSON_GetObjectItem(obj, "nvalue2")->valueint;
        // Relleno vvalue2 (array de floats)
        cJSON *array = cJSON_GetObjectItem(obj, "vvalue2");
        for (int i = 0; i < respuesta_servidor.N_value2; i++){
            respuesta_servidor.V_value2[i] = (float)cJSON_GetArrayItem(array, i)->valuedouble;
        }
        // Relleno value3 (struct paquete)
        cJSON *value3 = cJSON_GetObjectItem(obj, "value3");
        respuesta_servidor.value3.x = cJSON_GetObjectItem(value3, "x")->valueint;
        respuesta_servidor.value3.y = cJSON_GetObjectItem(value3, "y")->valueint;
        respuesta_servidor.value3.z = cJSON_GetObjectItem(value3, "z")->valueint;
    }
    // 3. Devuelvo la struct respuesta
    cJSON_Delete(obj);
    return respuesta_servidor;
}

// Función auxiliar: Envía la petición al servidor, espera la respuesta y la devuelve
int send_recv(char* request, char* response) {

    // 1. Leer IP_TUPLAS y PORT_TUPLAS del entorno
    char* ip_servidor = getenv("IP_TUPLAS");
    if (ip_servidor == NULL){
        perror("Error al leer la IP del servidor");
        return -2;
    }
    char* puerto_servidor = getenv("PORT_TUPLAS");
    if (puerto_servidor == NULL){
        perror("Error al leer el puerto del servidor");
        return -2;
    }

    // 2. Resolver el host con getaddrinfo
    struct addrinfo *res; // es la salida, getaddrinfo escribe ahi
    struct addrinfo hints; // es la entrada, para decir el tipo de conexion que queremos (IPv4, TCP)

    // 2.1 Quito la basura que pueda haber al declarar la variable hints
    memset(&hints, 0, sizeof(hints));
    
    // 2.2 Indico la informacion de la conexion
    hints.ai_family = AF_INET; // Significa IPv4
    hints.ai_socktype = SOCK_STREAM; // Significa TCP

    // 2.3 LLamo a getaddrinfo (getaddrinfo necesita un puntero al puntero res)
    if (getaddrinfo(ip_servidor, puerto_servidor, &hints, &res) !=0){
        perror("Error al usar getaddrinfo");
        return -2;
    }

    // 3. Crear socket y hacer connect (ya tengo toda la info en res)

    // 3.1 Para crear el socket necesito el tipo de direccion, tipo de socket y protocolo
    int sock = socket(res->ai_family, res->ai_socktype, 0);
    if (sock == -1){
        perror("Error al crear el socket");
        freeaddrinfo(res);
        return -2;
    }

    // 3.2 Me conecto al servidor (necesito su IP y PUERTO, viene ya todo en ai_addr)
    if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Error al conectar");
        close(sock);
        freeaddrinfo(res);
        return -2;
    }

    // 4. Enviar los 4 Bytes de longitud + el JSON request

    // 4.1 Calculo los bytes de longitud para que el servidor sepa los bytes que tiene que leer del JSON
    int longitud_request = strlen(request);

    // 4.2 Para que el servidor lo entienda lo tengo que transformar a big-endian (formato de red)
    uint32_t longitud_request_final = htonl(longitud_request);
    
    // 4.3 Envio la longitud de la request
    send(sock, &longitud_request_final, sizeof(longitud_request_final), 0);

    // 4.4 Envio la request
    send(sock, request, longitud_request, 0);

    // 5. Recibir los 4 Bytes de longitud + el JSON response

    // 5.1 Primero recibo la longitud de la response
    uint32_t longitud_response;
    recv(sock, &longitud_response, sizeof(longitud_response), 0);

    // 5.2 Convierto la longitud a formato local
    int longitud_response_final = ntohl(longitud_response);

    // 5.3 Recibo el JSON
    recv(sock, response, longitud_response_final, 0);

    // 6. Cerrar el socket
    close(sock);
    freeaddrinfo(res);
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
    if (send_recv(json_request, res) == -2){
        free(json_request);
        return -2;
    } 

    free(json_request);

    // 4. Leo el JSON de la response y lo paso a struct response
    struct respuesta respuesta_final = traducir_json_response(res);

    return respuesta_final.resultado;
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
    if (send_recv(json_request, res) == -2){
        free(json_request);
        return -2;
    } 

    free(json_request);

    // 5. Leo el JSON de la response y lo paso a struct response
    struct respuesta respuesta_final = traducir_json_response(res);
    return respuesta_final.resultado;
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
    
    // 3. Envio la request en formato JSON y recibo la response en formato JSON
    if (send_recv(json_request, res) == -2){
        free(json_request);
        return -2;
    } 

    free(json_request);
    
    // 4. Paso el JSON de la respones a struct respuesta
    struct respuesta respuesta_final = traducir_json_response(res);

    // 5. Desempaquetar la respuesta (si la clave existe)
    if (respuesta_final.resultado == 0) {
        strcpy(value1, respuesta_final.value1);
        *N_value2 = respuesta_final.N_value2;
        memcpy(V_value2, respuesta_final.V_value2, respuesta_final.N_value2 * sizeof(float));
        *value3 = respuesta_final.value3;
    }

    // 6. Devolver el resultado de la respuesta
    return respuesta_final.resultado;
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
    if (send_recv(json_request, res) == -2){
        free(json_request);
        return -2;
    }

    free(json_request);

    // 4. Paso el JSON de la respones a una struct respuesta
    struct respuesta respuesta_final = traducir_json_response(res);
    return respuesta_final.resultado;
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

    // 3. Envio la request en formato JSON y recibo la response en formato JSON
    if (send_recv(json_request, res) == -2){
        free(json_request);
        return -2;
    } 

    free(json_request);

    // 4. Paso la response JSON a struct respuesta
    struct respuesta respuesta_final = traducir_json_response(res);
    return respuesta_final.resultado;
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
    if (send_recv(json_request, res) == -2){
        free(json_request);
        return -2;
    } 

    free(json_request);
    
    // 4. Paso el JSON de la response a struct respuesta
    struct respuesta respuesta_final = traducir_json_response(res);
    return respuesta_final.resultado;
}