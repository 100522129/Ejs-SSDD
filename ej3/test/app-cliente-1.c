#include <stdio.h>
#include <string.h>
#include "claves.h"

// Contadores globales de pruebas
int total = 0;
int passed = 0;

// Función auxiliar para registrar el resultado de cada prueba
void resultado(int condicion, char *msg) {
    // Incrementamos el contador total de pruebas
    total++;

    // Si la condición es verdadera, la prueba ha pasado
    if (condicion == 1) {
        printf("  [OK]   %s\n", msg);
        passed++;
    } else {
        printf("  [FAIL] %s\n", msg);
    }
}
/*
Para evitar tantos if else, comprobamos el argumento a pasar directamente en la misma llamada,
el compilador hara lo siguiente:
    si err == x, entonces es verdadero y pasa un 1 a la funcion resultado
    si err != x, entonces es falso y pasa un 0 a la funcion resultado
*/

// =============================================================================
// SUITE 1: set_value — Inserción básica y restricciones
// =============================================================================
void test_set_value() {
    printf("\n=== SUITE 1: set_value ===\n");

    // Limpiamos el almacen antes de empezar
    destroy();

    // Preparamos los valores de prueba
    struct Paquete p;
    p.x = 1;
    p.y = 2;
    p.z = 3;
    float v[] = {1.0, 2.0, 3.0};

    // 1.1 Inserción normal con valores válidos
    int err = set_value("k1", "valor1", 3, v, p);
    resultado(err == 0, "1.1 Insercion valida");

    // 1.2 Clave duplicada, debe fallar
    err = set_value("k1", "otro", 3, v, p);
    resultado(err == -1, "1.2 Clave duplicada rechazada");

    // 1.3 N_value2 = 0 esta fuera del rango permitido, debe fallar
    err = set_value("k2", "val", 0, v, p);
    resultado(err == -1, "1.3 N_value2 = 0 rechazado");

    // 1.4 N_value2 = 33 esta fuera del rango permitido, debe fallar
    err = set_value("k3", "val", 33, v, p);
    resultado(err == -1, "1.4 N_value2 = 33 rechazado");

    // 1.5 N_value2 = 1 es el minimo valido, debe aceptarse
    err = set_value("k4", "val", 1, v, p);
    resultado(err == 0, "1.5 N_value2 = 1 aceptado");

    // 1.6 N_value2 = 32 es el maximo valido, debe aceptarse
    float vmax[32];
    int i;
    for (i = 0; i < 32; i++) {
        vmax[i] = (float)i;
    }
    err = set_value("k5", "val", 32, vmax, p);
    resultado(err == 0, "1.6 N_value2 = 32 aceptado");

    // 1.7 Dos claves distintas deben coexistir sin problemas
    int err1 = set_value("k6", "val6", 2, v, p);
    int err2 = set_value("k7", "val7", 2, v, p);
    resultado(err1 == 0 && err2 == 0, "1.7 Multiples claves distintas coexisten");

    // Limpiamos el almacen al terminar la suite
    destroy();
}

// =============================================================================
// SUITE 2: exist — Comprobación de existencia
// =============================================================================
void test_exist() {
    printf("\n=== SUITE 2: exist ===\n");

    // Limpiamos el almacen antes de empezar
    destroy();

    // Preparamos los valores de prueba
    struct Paquete p;
    p.x = 0;
    p.y = 0;
    p.z = 0;
    float v[] = {5.5};

    // Insertamos una clave para las pruebas
    set_value("existe", "v", 1, v, p);

    // 2.1 Una clave que fue insertada debe existir
    resultado(exist("existe") == 1, "2.1 Clave existente devuelve 1");

    // 2.2 Una clave que no fue insertada no debe existir
    resultado(exist("no_existe") == 0, "2.2 Clave inexistente devuelve 0");

    // 2.3 Tras destruir el almacen ninguna clave debe existir
    destroy();
    resultado(exist("existe") == 0, "2.3 Almacen vacio devuelve 0");

    // Limpiamos el almacen al terminar la suite
    destroy();
}

// =============================================================================
// SUITE 3: get_value — Recuperación de datos
// =============================================================================
void test_get_value() {
    printf("\n=== SUITE 3: get_value ===\n");

    // Limpiamos el almacen antes de empezar
    destroy();

    // Preparamos los valores de prueba
    struct Paquete p_in;
    p_in.x = 10;
    p_in.y = 20;
    p_in.z = 30;
    float v_in[] = {1.1f, 2.2f, 3.3f};

    // Insertamos la tupla que vamos a recuperar
    set_value("gk1", "hola mundo", 3, v_in, p_in);

    // Declaramos los buffers de salida para get_value
    char v1_out[256];
    int n_out;
    float v2_out[32];
    struct Paquete p_out;

    // 3.1 Recuperacion exitosa sobre una clave existente
    int err = get_value("gk1", v1_out, &n_out, v2_out, &p_out);
    resultado(err == 0, "3.1 get_value sobre clave existente");

    // 3.2 La cadena de texto recuperada debe coincidir con la insertada
    resultado(strcmp(v1_out, "hola mundo") == 0, "3.2 value1 recuperado correctamente");

    // 3.3 El numero de elementos del vector debe coincidir con el insertado
    resultado(n_out == 3, "3.3 N_value2 recuperado correctamente");

    // 3.4 Los valores del vector de floats deben coincidir con los insertados
    resultado(v2_out[0] == 1.1f && v2_out[1] == 2.2f && v2_out[2] == 3.3f,
              "3.4 V_value2 recuperado correctamente");

    // 3.5 Los campos del paquete deben coincidir con los insertados
    resultado(p_out.x == 10 && p_out.y == 20 && p_out.z == 30,
              "3.5 Paquete recuperado correctamente");

    // 3.6 Intentar recuperar una clave que no existe debe devolver -1
    err = get_value("no_existe", v1_out, &n_out, v2_out, &p_out);
    resultado(err == -1, "3.6 get_value sobre clave inexistente devuelve -1");

    // 3.7 Pasar key NULL debe devolver -1
    err = get_value(NULL, v1_out, &n_out, v2_out, &p_out);
    resultado(err == -1, "3.7 get_value con key NULL devuelve -1");

    // Limpiamos el almacen al terminar la suite
    destroy();
}

// =============================================================================
// SUITE 4: modify_value — Modificación de tuplas
// =============================================================================
void test_modify_value() {
    printf("\n=== SUITE 4: modify_value ===\n");

    // Limpiamos el almacen antes de empezar
    destroy();

    // Preparamos los valores originales
    struct Paquete p;
    p.x = 1;
    p.y = 1;
    p.z = 1;
    float v[] = {0.1f, 0.2f};

    // Insertamos la tupla que vamos a modificar
    set_value("mk1", "original", 2, v, p);

    // Preparamos los valores de modificacion
    struct Paquete p_mod;
    p_mod.x = 99;
    p_mod.y = 88;
    p_mod.z = 77;
    float v_mod[] = {9.9f};

    // 4.1 Modificacion exitosa sobre una clave existente
    int err = modify_value("mk1", "modificado", 1, v_mod, p_mod);
    resultado(err == 0, "4.1 modify_value exitoso");

    // Recuperamos los datos para comprobar que se actualizaron
    char v1_out[256];
    int n_out;
    float v2_out[32];
    struct Paquete p_out;
    get_value("mk1", v1_out, &n_out, v2_out, &p_out);

    // 4.2 El valor de texto debe haberse actualizado
    resultado(strcmp(v1_out, "modificado") == 0, "4.2 value1 actualizado correctamente");

    // 4.3 El vector de floats debe haberse actualizado
    resultado(n_out == 1 && v2_out[0] == 9.9f, "4.3 V_value2 actualizado correctamente");

    // 4.4 El paquete debe haberse actualizado
    resultado(p_out.x == 99 && p_out.y == 88 && p_out.z == 77,
              "4.4 Paquete actualizado correctamente");

    // 4.5 Modificar una clave que no existe debe devolver -1
    err = modify_value("no_existe", "x", 1, v_mod, p_mod);
    resultado(err == -1, "4.5 modify_value sobre clave inexistente devuelve -1");

    // 4.6 N_value2 fuera de rango en modify debe devolver -1
    err = modify_value("mk1", "x", 0, v_mod, p_mod);
    resultado(err == -1, "4.6 modify_value con N_value2 = 0 rechazado");

    // Limpiamos el almacen al terminar la suite
    destroy();
}

// =============================================================================
// SUITE 5: delete_key — Borrado de claves
// =============================================================================
void test_delete_key() {
    printf("\n=== SUITE 5: delete_key ===\n");

    // Limpiamos el almacen antes de empezar
    destroy();

    // Preparamos los valores de prueba
    struct Paquete p;
    p.x = 0;
    p.y = 0;
    p.z = 0;
    float v[] = {1.0};

    // Insertamos tres claves para las pruebas
    set_value("dk1", "v", 1, v, p);
    set_value("dk2", "v", 1, v, p);
    set_value("dk3", "v", 1, v, p);

    // 5.1 Borrar una clave existente debe devolver 0
    int err = delete_key("dk1");
    resultado(err == 0, "5.1 Borrar clave existente devuelve 0");

    // 5.2 La clave borrada ya no debe existir en el almacen
    resultado(exist("dk1") == 0, "5.2 Clave borrada ya no existe");

    // 5.3 Las demas claves no deben verse afectadas por el borrado
    resultado(exist("dk2") == 1 && exist("dk3") == 1,
              "5.3 Las demas claves no se ven afectadas");

    // 5.4 Borrar una clave que no existe debe devolver -1
    err = delete_key("no_existe");
    resultado(err == -1, "5.4 Borrar clave inexistente devuelve -1");

    // 5.5 Borrar la misma clave por segunda vez debe devolver -1
    int err1 = delete_key("dk2");
    int err2 = delete_key("dk2");
    resultado(err1 == 0 && err2 == -1, "5.5 Segundo borrado de misma clave devuelve -1");

    // Limpiamos el almacen al terminar la suite
    destroy();
}

// =============================================================================
// SUITE 6: destroy — Vaciado del almacén
// =============================================================================
void test_destroy() {
    printf("\n=== SUITE 6: destroy ===\n");

    // Preparamos los valores de prueba
    struct Paquete p;
    p.x = 0;
    p.y = 0;
    p.z = 0;
    float v[] = {1.0};

    // Insertamos varias claves antes de destruir
    set_value("d1", "v", 1, v, p);
    set_value("d2", "v", 1, v, p);
    set_value("d3", "v", 1, v, p);

    // 6.1 destroy debe devolver 0
    int err = destroy();
    resultado(err == 0, "6.1 destroy devuelve 0");

    // 6.2 Tras destroy ninguna de las claves insertadas debe existir
    resultado(exist("d1") == 0 && exist("d2") == 0 && exist("d3") == 0,
              "6.2 Ninguna clave existe tras destroy");

    // 6.3 Llamar a destroy sobre un almacen ya vacio tambien debe devolver 0
    err = destroy();
    resultado(err == 0, "6.3 destroy sobre almacen vacio devuelve 0");
}

// =============================================================================
// SUITE 7: Flujo completo encadenado
// =============================================================================
void test_flujo_completo() {
    printf("\n=== SUITE 7: Flujo completo ===\n");

    // Limpiamos el almacen antes de empezar
    destroy();

    // Preparamos los valores de prueba
    struct Paquete p;
    p.x = 7;
    p.y = 8;
    p.z = 9;
    float v[] = {1.0f, 2.0f};

    // Insertamos la tupla inicial
    set_value("flujo", "inicial", 2, v, p);

    // 7.1 La clave debe existir tras insertarla
    resultado(exist("flujo") == 1, "7.1 Existe tras insertar");

    // Modificamos todos los campos de la tupla
    struct Paquete pm;
    pm.x = 100;
    pm.y = 200;
    pm.z = 300;
    float vm[] = {5.5f, 6.6f, 7.7f};
    modify_value("flujo", "modificado", 3, vm, pm);

    // Recuperamos los datos para verificar la modificacion
    char v1[256];
    int n;
    float vf[32];
    struct Paquete pr;
    get_value("flujo", v1, &n, vf, &pr);

    // 7.2 Los datos recuperados deben coincidir con los modificados
    resultado(strcmp(v1, "modificado") == 0 && n == 3,
              "7.2 Datos correctos tras modificar");

    // Borramos la clave
    delete_key("flujo");

    // 7.3 La clave no debe existir tras borrarla
    resultado(exist("flujo") == 0, "7.3 No existe tras borrar");

    // 7.4 Se debe poder reinsertar una clave que fue borrada previamente
    int err = set_value("flujo", "renacido", 2, v, p);
    resultado(err == 0, "7.4 Se puede reinsertar una clave previamente borrada");

    // Limpiamos el almacen al terminar la suite
    destroy();
}
                                                                                             
// =============================================================================
// SUITE 8: Parámetros NULL                                                                        
// =============================================================================                 
void test_null_params() {                                                                   
    printf("\n=== SUITE 8: Parámetros NULL ===\n");                                         
                                                                                            
    destroy();                                                                              
                                                                                            
    char v1_out[256];                                                                       
    int n_out;                                                                              
    float v2_out[32];                                                                       
    struct Paquete p_out;                                                                   
                                                                                            
    // 8.1 get_value con key NULL            
    int err = get_value(NULL, v1_out, &n_out, v2_out, &p_out);                              
    resultado(err == -1, "8.1 get_value con key NULL devuelve -1");                         
                                                                                            
    // 8.2 delete_key con key NULL           
    err = delete_key(NULL);                                                                 
    resultado(err == -1, "8.2 delete_key con key NULL devuelve -1");                        
                                                                                            
    // 8.3 exist con key NULL              
    err = exist(NULL);                                                                      
    resultado(err == -1, "8.3 exist con key NULL devuelve -1");                                     
                                                                                            
    destroy();                                                                              
}                                                                                           
                                                                                            
// =============================================================================            
// SUITE 9: Límites de tamaño de clave y value1                                             
// =============================================================================            
void test_limites_strings() {                                                               
    printf("\n=== SUITE 9: Limites de strings ===\n");                                      
                                                                                            
    destroy();                                                                              
                                                                                            
    struct Paquete p;                                                                       
    p.x = 0; p.y = 0; p.z = 0;                                                              
    float v[] = {1.0f};                                                                     
                                                                                            
    // 9.1 N_value2 = -1 (negativo, fuera de rango)                      
    int err = set_value("k_neg", "val", -1, v, p);                                          
    resultado(err == -1, "9.1 set_value con N_value2 = -1 rechazado");                      
                                                                                            
    // 9.2 Clave de 255 chars (maximo valido: strnlen devuelve 255, no 256)                 
    char key255[256];                                                                       
    memset(key255, 'a', 255);                                                               
    key255[255] = '\0';                                                                     
    err = set_value(key255, "val", 1, v, p);                                                
    resultado(err == 0, "9.2 Clave de 255 chars aceptada");                                 
                                                                                            
    // 9.3 Clave de 256 chars (invalida: strnlen con maxlen=256 devuelve 256)               
    char key256[257];                                                                       
    memset(key256, 'b', 256);                                                               
    key256[256] = '\0';                                                                     
    err = set_value(key256, "val", 1, v, p);                                                
    resultado(err == -1, "9.3 Clave de 256 chars rechazada");                               
                                                                                            
    // 9.4 value1 de 255 chars (maximo valido)                                              
    char val255[256];                                                                       
    memset(val255, 'c', 255);                                                               
    val255[255] = '\0';                                                                     
    err = set_value("k_val255", val255, 1, v, p);                                           
    resultado(err == 0, "9.4 value1 de 255 chars aceptado");                                
                                                                                            
    // 9.5 value1 de 256 chars (invalido: strnlen con maxlen=256 devuelve 256)              
    char val256[257];                                                                       
    memset(val256, 'd', 256);                                                               
    val256[256] = '\0';                                                                     
    err = set_value("k_val256", val256, 1, v, p);                                           
    resultado(err == -1, "9.5 value1 de 256 chars rechazado");                              
                                                                                            
    // 9.6 modify_value con N_value2 negativo                         
    float v2[] = {9.9f};                                                                    
    err = set_value("k_mod", "original", 1, v2, p);                                         
    err = modify_value("k_mod", "nuevo", -5, v2, p);                                        
    resultado(err == -1, "9.6 modify_value con N_value2 negativo rechazado");               
                                                                                            
    destroy();                                                                              
}                                                                                           
                                                                                            
// =============================================================================            
// MAIN
// =============================================================================
int main() {
    printf("============================================\n");
    printf("   BATERIA DE PRUEBAS - app-cliente.c\n");
    printf("============================================\n");

    // Ejecutamos todas las suites de pruebas
    test_set_value();
    test_exist();
    test_get_value();
    test_modify_value();
    test_delete_key();
    test_destroy();
    test_flujo_completo();
    test_null_params();                                                                     
    test_limites_strings();                                                                
    
    // Mostramos el resumen final de resultados
    printf("\n============================================\n");
    printf("  RESULTADO: %d / %d pruebas pasadas\n", passed, total);
    printf("============================================\n");

    // Devolvemos 0 si todas las pruebas han pasado, -1 en caso contrario
    if (passed == total) {
        return 0;
    } else {
        return -1;
    }
}