#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "claves.h"
#include "mensajes.h"
#include "cJSON.h"

// ============================
// === Funciones Auxiliares ===
// ============================

// Recibe n bytes, rellenando el buffer en varios recv si hace falta
int recv_all(int sock, void *buf, int n){
    int recibido = 0;
    char *p = (char *)buf;
    while (recibido < n) { // Lee hasta haber recibido todo
        int r = recv(sock, p + recibido, n - recibido, 0);
        if (r <= 0) return -1; // Error o conexión cerrada
        recibido += r; // Suma los bytes leídos
    }
    return 0;
}


// Envía exactamente n bytes
int send_all(int sock, void *buf, int n) {
    int enviado = 0;
    char *p = (char *)buf;
    while (enviado < n) { // Sigue hasta haber enviado todo
        int s = send(sock, p + enviado, n - enviado, 0);
        if (s <= 0) return -1; // Error o conexión cerrada
        enviado += s; // Suma los bytes leídos
    }
    return 0;
}


// =============================================
// Parsear la petición JSON -> struct peticion
// =============================================

struct peticion parsear_json_request(const char *json_str) {
    struct peticion req;
    memset(&req, 0, sizeof(req));

    cJSON *obj = cJSON_Parse(json_str);
    if (obj == NULL) { // Caso de error
        req.op = -1;
        return req;
    }

    cJSON *op_item = cJSON_GetObjectItem(obj, "op");
    if (op_item == NULL) { // Caso de error
        req.op = -1;
        cJSON_Delete(obj);
        return req;
    }

    req.op = op_item->valueint;

    // Según la operación, extraemos los campos necesarios
    if (req.op == OP_SET || req.op == OP_MODIFY) {
        cJSON *key     = cJSON_GetObjectItem(obj, "key");
        cJSON *value1  = cJSON_GetObjectItem(obj, "value1");
        cJSON *nvalue2 = cJSON_GetObjectItem(obj, "nvalue2");
        cJSON *vvalue2 = cJSON_GetObjectItem(obj, "vvalue2");
        cJSON *value3  = cJSON_GetObjectItem(obj, "value3");
 
        if (key) strncpy(req.key, key->valuestring, 255);
        if (value1) strncpy(req.value1, value1->valuestring, 255);
        if (nvalue2) req.N_value2 = nvalue2->valueint;

        if (vvalue2 && cJSON_IsArray(vvalue2)) {
            for (int i = 0; i < req.N_value2; i++) {
                cJSON *item = cJSON_GetArrayItem(vvalue2, i);
                if (item) req.V_value2[i] = (float)item->valuedouble;
            }
        }
 
        if (value3) {
            cJSON *x = cJSON_GetObjectItem(value3, "x");
            cJSON *y = cJSON_GetObjectItem(value3, "y");
            cJSON *z = cJSON_GetObjectItem(value3, "z");
            if (x) req.value3.x = x->valueint;
            if (y) req.value3.y = y->valueint;
            if (z) req.value3.z = z->valueint;
        }
    }

    else if (req.op == OP_GET || req.op == OP_DELETE || req.op == OP_EXIST) {
        cJSON *key = cJSON_GetObjectItem(obj, "key");
        if (key) strncpy(req.key, key->valuestring, 255);
    }

    // OP_INIT (destroy) no tiene campos adicionales, no hay que comprobarlo
    cJSON_Delete(obj);
    return req;
}


// =================================================
// Construir la respuesta JSON <- struct respuesta
// ==================================================

// es_get_value: 1 si hay que incluir los campos de value1, nvalue2, vvalue2, value3
char* construir_json_response(struct respuesta res, int es_get_value) {
    cJSON *obj = cJSON_CreateObject();

    cJSON_AddNumberToObject(obj, "result", res.resultado);

    if (es_get_value && res.resultado == 0) {
        cJSON_AddStringToObject(obj, "value1", res.value1);
        cJSON_AddNumberToObject(obj, "nvalue2", res.N_value2);
 
        cJSON *array = cJSON_CreateArray();
        for (int i = 0; i < res.N_value2; i++) {
            cJSON_AddItemToArray(array, cJSON_CreateNumber(res.V_value2[i]));
        }
        cJSON_AddItemToObject(obj, "vvalue2", array);
 
        cJSON *paquete = cJSON_CreateObject();
        cJSON_AddNumberToObject(paquete, "x", res.value3.x);
        cJSON_AddNumberToObject(paquete, "y", res.value3.y);
        cJSON_AddNumberToObject(paquete, "z", res.value3.z);
        cJSON_AddItemToObject(obj, "value3", paquete);
    }

    char *json_str = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);
    return json_str;
}


// ==========================================================
// HILO: gestiona la conexión de un cliente de inicio a fin
// ==========================================================

void *gestionar_cliente (void *arg) {
    int sock_cliente = *(int *)arg;
    free(arg); // Liberamos el int* creado en el main para pasarlo al hilo
    
    // 1. Recibir los 4 bytes de longitud de la petición JSON
    uint32_t longitud_req_net;
    if (recv_all(sock_cliente, &longitud_req_net, sizeof(longitud_req_net)) == -1) {
        perror("[SERVIDOR] Error al recibir longitud de petición");
        close(sock_cliente);
        return NULL;
    }
    int longitud_req = ntohl(longitud_req_net);

    // 2. Recibir el JSON de la petición
    char *req_json = malloc(longitud_req + 1); // Para luego añádir '\0'
    if (req_json == NULL) {
        perror("[SERVIDOR] Error al reservar memoria para la petición");
        close(sock_cliente);
        return NULL;
    }
    if (recv_all(sock_cliente, req_json, longitud_req) == -1) {
        perror("[SERVIDOR] Error al recibir JSON de petición");
        free(req_json);
        close(sock_cliente);
        return NULL;
    }
    req_json[longitud_req] = '\0'; // !!!!! Es necesario el '\0'??

    // 3. Parsear la petición
    struct peticion req = parsear_json_request(req_json);
    free(req_json);

    if (req.op == -1) {
        perror("[SERVIDOR] JSON de petición inválido");
        close(sock_cliente);
        return NULL;
    }

    // 4. Ejecutar la operación correspondiente (llamada a claves.c)
    struct respuesta res;
    memset(&res, 0, sizeof(res));
    int es_get_value = 0;

    switch (req.op) {
        case OP_INIT:
            res.resultado = destroy();
            break;

        case OP_SET:
            res.resultado = set_value(req.key, req.value1, req.N_value2, req.V_value2, req.value3);
            break;
            
        case OP_GET:
            es_get_value = 1;
            res.resultado = get_value(req.key, res.value1, &res.N_value2, res.V_value2, &res.value3);
            break;
        
        case OP_MODIFY:
            res.resultado = modify_value(req.key, req.value1, req.N_value2, req.V_value2, req.value3);
            break;

        case OP_DELETE:
            res.resultado = delete_key(req.key);
            break;
        
        case OP_EXIST:
            res.resultado = exist(req.key);
            break;

        default: // Caso de error
            fprintf(stderr, "[SERVIDOR] Operación desconocida: %d\n", req.op);
            res.resultado = -1;
            break;
    }

    // 5. Construir el JSON de respuesta
    char *res_json = construir_json_response(res, es_get_value);
    if (res_json == NULL) {
        fprintf(stderr, "[SERVIDOR] Error al construir JSON de respuesta\n");
        close(sock_cliente);
        return NULL;
    }

    // 6. Enviar los 4 bytes de longitud + JSON de respuesta
    int longitud_res = strlen(res_json);
    uint32_t longitud_res_net = htonl(longitud_res);

    if (send_all(sock_cliente, &longitud_res_net, sizeof(longitud_res_net)) == -1 ||
        send_all(sock_cliente, res_json, longitud_res) == -1) {
        perror("[SERVIDOR] Error al enviar respuesta");
    }

    // 7. Limpiar la memoria y cerrar
    free(res_json);
    close(sock_cliente);
    return NULL;
}


// ====================
// === FUNCIÓN MAIN ===
// ====================

int main(int argc, char *argv[]) {
    // 1. Leer el puerto de los argumentos: ./servidor <PUERTO>
    if (argc != 2) {
        fprintf(stderr, "Uso: ./servidor <PUERTO>\n");
        return -1;
    }
    int puerto = atoi(argv[1]);
    
    // Comprobar si es un número válido
    if (puerto <= 0 || puerto > 65535) {
        fprintf(stderr, "[SERVIDOR] Puerto inválido: %s (Rango permitido 1 - 65535)\n", argv[1]);
        return -1;
    }

    // 2. Crear el socket servidor
    int sock_servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_servidor == -1) {
        perror("[SERVIDOR] Error al crear el socket");
        return -1;
    }

    // SO_REUSEADDR: permite reutilizar el puerto inmediatamente tras reiniciar el servidor
    // !!!!! Esto lo dejamos así??
    int opt = 1;
    if (setsockopt(sock_servidor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("[SERVIDOR] Error en setsockopt");
        close(sock_servidor);
        return -1;
    }

    // 3. Configurar la dirección y hacer bind
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(puerto);
    addr.sin_addr.s_addr = INADDR_ANY; // Acepta conexiones en cualquier interfaz

    if (bind(sock_servidor, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("[SERVIDOR] Error en bind");
        close(sock_servidor);
        return -1;
    }

    // 4. Poner el socket en modo escucha
    if (listen(sock_servidor, 32) == -1) {
        perror("[SERVIDOR] Error en listen");
        close(sock_servidor);
        return -1;
    }

    printf("[SERVIDOR] Escuchando en el puerto %d...\n", puerto);

    // 5. Bucle principal: acepta conexiones y lanza un hilo por cliente
    while (1) {
        struct sockaddr_in addr_cliente;
        socklen_t len_cliente = sizeof(addr_cliente);

        int *sock_cliente = malloc(sizeof(int)); // malloc para evitar que haya race condition
        if (sock_cliente == NULL) {
            perror("[SERVIDOR] Error al reservar memoria para el socket del cliente");
            continue;
        }

        *sock_cliente = accept(sock_servidor, (struct sockaddr *)&addr_cliente, &len_cliente);
        if (*sock_cliente == -1) {
            perror("[SERVIDOR] Error en accept");
            free(sock_cliente);
            continue;
        }

        // Lanzamos el hilo con DETACHED para que libere sus recursos automáticamente al terminar
        pthread_t hilo;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create(&hilo, &attr, gestionar_cliente, sock_cliente) != 0) {
            perror("[SERVIDOR] Error al crear hilo");
            close(*sock_cliente);
            free(sock_cliente);
        }

        pthread_attr_destroy(&attr);
    }

    close(sock_servidor);
    return 0;
}