#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h> // Necesaria para usar la API POSIX
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "claves.h"
#include "mensajes.h"

#define SERVER_QUEUE "/SERVIDOR" // Nombre de la cola del servidor

// Función auxiliar:
// Envía la petición al servidor, espera la respuesta y la devuelve
int send_recv(struct peticion *req, struct respuesta *res) {
    
    // 1. Conectar
    // Declarar descriptores, nombre cola y atributos
    mqd_t q_server, q_client;
    char client_queue[256];
    
    /*
    He activado los atributos y se los paso al crear la cola porque sino falla porque linux da un tamannio
    inicial que no nos vale para mandar nuestras structs 
    */
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct respuesta);
    attr.mq_curmsgs = 0;

    // Inventamos un nombre de cola para el cliente, único e identificativo
    sprintf(client_queue, "/CLIENTE_%d", getpid());
    strcpy(req->q_name, client_queue); // Le decimos al servidor dónde contestar

    // Abrimos las colas
    q_client = mq_open(client_queue, O_CREAT | O_RDONLY, 0700, &attr);
    if (q_client == -1) {
        perror("Error creando la cola del cliente en el proxy");
        return -2;
    }
    
    q_server = mq_open("/SERVIDOR", O_WRONLY);

    if (q_server == -1) {
        perror("Error abriendo la cola del servidor en proxy");
        mq_close(q_client);
        mq_unlink(client_queue);
        return -2;
    }

    // 2. Enviar y recibir
    if (mq_send(q_server, (char *)req, sizeof(struct peticion), 0) == -1) {
        perror("Error al enviar el mensaje");
        mq_close(q_server);
        mq_close(q_client);
        mq_unlink(client_queue);

        return -2;
    }
    
    if (mq_receive(q_client, (char *)res, sizeof(struct respuesta), NULL) == -1) {
        perror("Error al recibir el mensaje");
        mq_close(q_server);
        mq_close(q_client);
        mq_unlink(client_queue);

        return -2;
    }


    // 3. Limpiar las colas
    mq_close(q_server);
    mq_close(q_client);
    mq_unlink(client_queue);

    return 0;
}


int destroy(void) {
    struct peticion req;
    struct respuesta res;

    // 1. Empaquetar la petición
    req.op = OP_INIT;

    // 2. Enviar y recibir
    if (send_recv(&req, &res) == -2) return -2;

    return res.resultado;
}


int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
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
    struct peticion req;
    struct respuesta res;

    // 1. Empaquetar la petición
    req.op = OP_EXIST;
    strcpy(req.key, key);

    // 2. Enviar y recibir
    if (send_recv(&req, &res) == -2) return -2;

    return res.resultado;
}
