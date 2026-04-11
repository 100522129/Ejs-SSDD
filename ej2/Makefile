CC       = gcc
CFLAGS   = -Wall -Wextra -g
FLAGS_LIB = -fPIC -shared

# Host y puerto por defecto para los targets de run
IP = localhost
PUERTO = 8080

# Compila todo
all: build build/libclaves.so build/libproxyclaves.so build/servidor_sock \
     build/app_cliente_1 build/app_cliente_2

build:
	mkdir -p build

# ── BIBLIOTECAS ──────────────────────────────
# Parte A (libclaves.so) → almacén local (lista enlazada + mutex), usado por el servidor
build/libclaves.so: src/claves.c include/claves.h
	$(CC) $(FLAGS_LIB) -Iinclude -o build/libclaves.so src/claves.c -lpthread

# Parte B (libproxyclaves.so) → biblioteca del lado cliente (sockets TCP + JSON)
build/libproxyclaves.so: src/proxy-sock.c src/cJSON.c src/sock_utils.c include/cJSON.h include/mensajes.h include/claves.h
	$(CC) $(FLAGS_LIB) -Iinclude -o build/libproxyclaves.so src/proxy-sock.c src/cJSON.c src/sock_utils.c


# ── EJECUTABLES ──────────────────────────────
# Servidor TCP concurrente (usa libclaves.so en tiempo de enlace)
build/servidor_sock: src/servidor-sock.c src/cJSON.c src/sock_utils.c build/libclaves.so
	$(CC) $(CFLAGS) -Iinclude -o build/servidor_sock src/servidor-sock.c src/cJSON.c src/sock_utils.c -Lbuild -lclaves -lpthread

# Cliente Parte 1 → batería de pruebas funcionales (app-cliente-1.c)
build/app_cliente_1: test/app-cliente-1.c build/libproxyclaves.so
	$(CC) $(CFLAGS) -Iinclude -o build/app_cliente_1 test/app-cliente-1.c -Lbuild -lproxyclaves

# Cliente Parte 2 → prueba de estrés y concurrencia (app-cliente-2.c)
build/app_cliente_2: test/app-cliente-2.c build/libproxyclaves.so
	$(CC) $(CFLAGS) -Iinclude -o build/app_cliente_2 test/app-cliente-2.c -Lbuild -lproxyclaves

# ── LIMPIEZA ─────────────────────────────────
clean:
	rm -rf build

# ── EJECUCIÓN ────────────────────────

# Terminal 1: lanza el servidor en el puerto definido arriba
run_servidor: build/servidor_sock
	LD_LIBRARY_PATH=./build ./build/servidor_sock $(PUERTO)

# Terminal 2: pruebas funcionales con un solo cliente
run_cliente_1: build/app_cliente_1
	LD_LIBRARY_PATH=./build IP_TUPLAS=$(IP) PORT_TUPLAS=$(PUERTO) ./build/app_cliente_1

# Terminal 2: prueba de estrés con un solo cliente
run_cliente_2: build/app_cliente_2
	LD_LIBRARY_PATH=./build IP_TUPLAS=$(IP) PORT_TUPLAS=$(PUERTO) ./build/app_cliente_2

# Terminal 2: 10 clientes concurrentes
run_concurrente: build/app_cliente_2
	LD_LIBRARY_PATH=./build IP_TUPLAS=$(IP) PORT_TUPLAS=$(PUERTO) bash test/test_concurrencia.sh

.PHONY: all clean run_servidor run_cliente_1 run_cliente_2 run_concurrente