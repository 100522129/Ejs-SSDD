/* ==== ESTRUCTURAS ==== */

/* Equivalente a struct Paquete de claves.h */
struct PaqueteRPC {
    int x;
    int y;
    int z;
};

/* Argumentos para set_value y modify_value */
struct SetModArgs {
    string key<256>;
    string value1<256>;
    int N_value2;
    float V_value2<32>; /* array dinámico XDR */
    PaqueteRPC value3;
};

/* Resultado de get_value */
struct GetResult {
    int resultado;
    string value1<256>;
    int N_value2;
    float V_value2<32>;
    PaqueteRPC value3;
};

/* ==== PROGRAMA ==== */

program CLAVES_PROG {
    version CLAVES_VERS {
        int         destroy_rpc  (void)       = 1;
        int         set_rpc      (SetModArgs) = 2;
        GetResult   get_rpc      (string)     = 3;
        int         modify_rpc   (SetModArgs) = 4;
        int         delete_rpc   (string)     = 5;
        int         exist_rpc    (string)     = 6;
    } = 1;                          /* Es obligatorio poner versión */
} = 0x20001234;                     /* De 0x200...0 a 0x3FF...F reservados para programas de usuarios */