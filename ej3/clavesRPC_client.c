#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"
#include "clavesRPC.h"

static CLIENT *conectar(void) {
    char *ip = getenv("IP_TUPLAS"); // Leo la IP del servidor de la variable de entorno
    if (ip == NULL) {
        fprintf(stderr, "Error: IP_TUPLAS no definida\n");
        return NULL;
    }
    // Me conecto al servidor RPC usando TCP (CLAVES_PROG y CLAVES_VERS identifican el servicio)
    CLIENT *clnt = clnt_create(ip, CLAVES_PROG, CLAVES_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(ip);
        return NULL;
    }
    return clnt;
}

int destroy(void) {
    CLIENT *clnt = conectar();
    if (clnt == NULL) return -1; // Si no puedo conectar, error

    int resultado;
    // Llamo a la funcion RPC. Le paso &resultado para que el servidor
    // escriba ahi su respuesta (0 = exito, -1 = error)
    enum clnt_stat status = destroy_rpc_1(&resultado, clnt);

    if (status != RPC_SUCCESS) { // Si la llamada RPC falla
        clnt_perror(clnt, "destroy_rpc_1");
        clnt_destroy(clnt);
        return -1;
    }

    clnt_destroy(clnt); // Cierro la conexion
    return resultado;   // Devuelvo lo que me dijo el servidor
}

int exist(char *key) {
    if (key == NULL) return -1; // Valido el parametro antes de conectar

    CLIENT *clnt = conectar();
    if (clnt == NULL) return -1;

    int resultado;
    // Le paso la clave y un puntero al resultado
    enum clnt_stat status = exist_rpc_1(key, &resultado, clnt);

    if (status != RPC_SUCCESS) {
        clnt_perror(clnt, "exist_rpc_1");
        clnt_destroy(clnt);
        return -1;
    }

    clnt_destroy(clnt);
    return resultado;
}

int delete_key(char *key) {
    if (key == NULL) return -1;

    CLIENT *clnt = conectar();
    if (clnt == NULL) return -1;

    int resultado;
    enum clnt_stat status = delete_rpc_1(key, &resultado, clnt);

    if (status != RPC_SUCCESS) {
        clnt_perror(clnt, "delete_rpc_1");
        clnt_destroy(clnt);
        return -1;
    }

    clnt_destroy(clnt);
    return resultado;
}

int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    if (key == NULL || value1 == NULL || V_value2 == NULL) return -1; // Valido parametros antes de conectar
    if (N_value2 < 1 || N_value2 > 32) return -1;

    CLIENT *clnt = conectar();
    if (clnt == NULL) return -1;

    // Empaqueto los argumentos en la estructura XDR que espera rpcgen
    SetModArgs args;
    args.key = key;
    args.value1 = value1;
    args.N_value2 = N_value2;
    args.V_value2.V_value2_val = V_value2; // V_value2 es un array dinamico XDR: necesito el puntero y la longitud
    args.V_value2.V_value2_len = N_value2;
    args.value3.x = value3.x; // Convierto el Paquete de claves.h al PaqueteRPC de clavesRPC.h
    args.value3.y = value3.y;
    args.value3.z = value3.z;

    int resultado;
    enum clnt_stat status = set_rpc_1(args, &resultado, clnt);

    if (status != RPC_SUCCESS) {
        clnt_perror(clnt, "set_rpc_1");
        clnt_destroy(clnt);
        return -1;
    }

    clnt_destroy(clnt);
    return resultado;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    if (key == NULL || value1 == NULL || V_value2 == NULL) return -1;
    if (N_value2 < 1 || N_value2 > 32) return -1;

    CLIENT *clnt = conectar();
    if (clnt == NULL) return -1;

    // Igual que set_value pero llamo a modify_rpc_1
    // El servidor sabra que tiene que modificar una tupla existente
    SetModArgs args;
    args.key = key;
    args.value1 = value1;
    args.N_value2 = N_value2;
    args.V_value2.V_value2_val = V_value2;
    args.V_value2.V_value2_len = N_value2;
    args.value3.x = value3.x;
    args.value3.y = value3.y;
    args.value3.z = value3.z;

    int resultado;
    enum clnt_stat status = modify_rpc_1(args, &resultado, clnt);

    if (status != RPC_SUCCESS) {
        clnt_perror(clnt, "modify_rpc_1");
        clnt_destroy(clnt);
        return -1;
    }

    clnt_destroy(clnt);
    return resultado;
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    if (key == NULL || value1 == NULL || N_value2 == NULL || V_value2 == NULL || value3 == NULL) {
        return -1; // Valido todos los punteros de salida
    }

    CLIENT *clnt = conectar();
    if (clnt == NULL) return -1;

    GetResult resultado;
    memset(&resultado, 0, sizeof(GetResult)); // Inicializo a cero para que RPC pueda deserializar correctamente
    enum clnt_stat status = get_rpc_1(key, &resultado, clnt);

    if (status != RPC_SUCCESS) {
        clnt_perror(clnt, "get_rpc_1");
        clnt_destroy(clnt);
        return -1;
    }

    int ret = resultado.resultado; // 0 = clave encontrada, -1 = no existe

    if (ret == 0) { // Solo copio los datos si la clave existia
        strncpy(value1, resultado.value1, 255);
        value1[255] = '\0'; // Garantizo que el string termina en \0

        *N_value2 = resultado.N_value2;

        memcpy(V_value2, resultado.V_value2.V_value2_val, resultado.N_value2 * sizeof(float)); // Copio el array de floats

        value3->x = resultado.value3.x; // Convierto el PaqueteRPC de vuelta al Paquete de claves.h
        value3->y = resultado.value3.y;
        value3->z = resultado.value3.z;
    }

    clnt_destroy(clnt);
    return ret;
}