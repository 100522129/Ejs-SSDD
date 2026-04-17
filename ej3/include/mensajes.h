#ifndef MENSAJES_H
#define MENSAJES_H

#include "claves.h"

// Códigos de operación
#define OP_INIT   0
#define OP_SET    1
#define OP_GET    2
#define OP_MODIFY 3
#define OP_DELETE 4
#define OP_EXIST  5

// ---------------------------------------------------------
// ESTRUCTURA 1: PETICIÓN (Lo que el Proxy envía al Servidor)
// ---------------------------------------------------------
struct peticion {
    int op;               // Código de la operación (OP_SET, OP_GET...)
    char q_name[256];     // Nombre de la cola del cliente para responder
    
    // Parámetros de la API
    char key[256];
    char value1[256];
    int N_value2;
    float V_value2[32];
    struct Paquete value3;
};

// ---------------------------------------------------------
// ESTRUCTURA 2: RESPUESTA (Lo que el Servidor devuelve)
// ---------------------------------------------------------
struct respuesta {
    int resultado;        // Devolverá 0 (éxito) o -1 (error)
    
    // Estos campos solo se rellenarán cuando la operación sea OP_GET
    char value1[256];
    int N_value2;
    float V_value2[32];
    struct Paquete value3;
};

#endif