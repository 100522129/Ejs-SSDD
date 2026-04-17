#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "claves.h"

int main() {
    int mi_pid = getpid();
    printf("[CLIENTE %d] Iniciando prueba de estrés...\n", mi_pid);

    int errores = 0;
    int num_pruebas = 100; // Cuántas tuplas insertará este cliente

    struct Paquete p = { mi_pid, mi_pid*2, mi_pid*3 };
    float v[] = {1.1f, 2.2f};
    char mi_valor[256];
    sprintf(mi_valor, "Valor del cliente %d", mi_pid);

    // 1. Inserción Masiva
    for (int i = 0; i < num_pruebas; i++) {
        char key[256];
        sprintf(key, "clave_%d_%d", mi_pid, i); // Clave única: ej. clave_1234_5
        
        if (set_value(key, mi_valor, 2, v, p) != 0) {
            printf("[CLIENTE %d] Error al insertar %s\n", mi_pid, key);
            errores++;
        }
    }

    // 2. Comprobación Masiva
    for (int i = 0; i < num_pruebas; i++) {
        char key[256];
        sprintf(key, "clave_%d_%d", mi_pid, i);

        char v1_out[256];
        int n_out;
        float v2_out[32];
        struct Paquete p_out;

        if (get_value(key, v1_out, &n_out, v2_out, &p_out) != 0) {
             printf("[CLIENTE %d] Error al recuperar %s\n", mi_pid, key);
             errores++;
        } else {
            // Verificamos que los datos no se hayan mezclado con los de otro cliente
            if (p_out.x != mi_pid || strcmp(v1_out, mi_valor) != 0) {
                printf("[CLIENTE %d] DATOS CORRUPTOS en %s\n", mi_pid, key);
                errores++;
            }
        }
    }

    if (errores == 0) {
        printf("[CLIENTE %d] ÉXITO: %d tuplas insertadas y validadas sin interferencias.\n", mi_pid, num_pruebas);
        return 0;
    } else {
        printf("[CLIENTE %d] FALLO: Se registraron %d errores.\n", mi_pid, errores);
        return -1;
    }
}