#include <stdio.h>
#include <string.h>
#include "claves.h"

int main() {
    printf("=== INICIANDO PRUEBAS app-cliente.c ===\n");

    // Primero limpiamos el almacén
    destroy();

    // --------------------------------------------------------------------------
    // 1. Insertamos unos datos de ejemplo (ejemplo del enunciado) -> set_value
    // --------------------------------------------------------------------------
    printf("[ Prueba 1 ] Insertando tupla... ");
    char *key = "clave 5";
    char *v1 = "ejemplo de valor 1";
    float v2[] = {2.3, 0.5, 23.45};
    struct Paquete v3;
    v3.x = 10;
    v3.y = 5;
    v3.z = 3;

    int err = set_value(key, v1, 3, v2, v3);
    
    if (err == 0) {
        printf("OK\n");
    } else {
        printf("ERROR\n");
    }
    
    // --------------------------------------------------------------------------
    // 2. Comprobamos que la tupla existe -> exist
    // --------------------------------------------------------------------------
    printf("[ Prueba 2 ] Comprobando si existe la clave... ");
    if (exist(key) == 1) {
        printf("OK\n");
    } else {
        printf("ERROR\n");
    }

    // --------------------------------------------------------------------------
    // 3. Intentamos recuperar los datos -> get_value
    // --------------------------------------------------------------------------
    printf("[ Prueba 3 ] Recuperando datos de la tupla... ");
    char v1_cp[256];
    int n_cp;
    float v2_cp[32];
    struct Paquete v3_cp;

    err = get_value(key, v1_cp, &n_cp, v2_cp, &v3_cp);
    if (err == 0) {
        printf("OK\n");
        printf("   -> Texto: '%s'\n", v1_cp);
        printf("   -> Paquete: x=%d, y=%d, z=%d\n", v3_cp.x, v3_cp.y, v3_cp.z);
    } else {
        printf("ERROR\n");
    }

    // --------------------------------------------------------------------------
    // 4. Modificamos el valor de la clave -> modify_value
    // --------------------------------------------------------------------------
    printf("[ Prueba 4 ] Modificando los datos de la tupla... ");
    char *v1_mod = "valor totalmente nuevo";
    float v2_mod[] = {9.9, 8.8}; // Ahora N será 2
    struct Paquete v3_mod;
    v3_mod.x = 100; v3_mod.y = 200; v3_mod.z = 300;

    err = modify_value(key, v1_mod, 2, v2_mod, v3_mod);
    if (err == 0) {
        printf("OK\n");
    } else {
        printf("ERROR al modificar\n");
    }

    // --------------------------------------------------------------------------
    // 5. Borramos la clave almacenada -> delete_key
    // --------------------------------------------------------------------------
    printf("[ Prueba 5 ] Borrando la clave almacenada... ");
    err = delete_key(key);
    if (err == 0) {
        printf("OK\n");
    } else {
        printf("ERROR al borrar\n");
    }

    // Verificamos extra para estar seguros de que se borró
    printf("[ Prueba 5.1 ] Verificando que ya no existe... ");
    if (exist(key) == 0) {
        printf("OK (Confirmado, no existe)\n");
    } else {
        printf("ERROR (La clave sigue ahí)\n");
    }
    
    // --------------------------------------------------------------------------
    // 6. Limpiamos al terminar -> destroy
    // --------------------------------------------------------------------------
    printf("[ Prueba 6 ] Destruyendo el almacén... "); // !!!!!!!!!!!!! Metemos una clave nueva antes de destruir para comprobar??
    err = destroy();
    if (err == 0) {
        printf("OK\n");
    } else {
        printf("ERROR\n");
    }


    printf("=== PRUEBAS FINALIZADAS ===\n");
    return 0;
}