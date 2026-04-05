CC       = gcc
CFLAGS   = -Wall -Wextra -g
FLAGS_LIB = -fPIC -shared

# Compila todo
all: build build/libclaves.so build/libproxyclaves.so build/servidor_mq \
     build/app_cliente_local build/app_cliente_local_mq build/app_cliente_mq

build:
	mkdir -p build

# ── BIBLIOTECAS ──────────────────────────────
# Parte A → biblioteca local
build/libclaves.so: src/claves.c include/claves.h
	$(CC) $(FLAGS_LIB) -Iinclude -o build/libclaves.so src/claves.c -lpthread

# Parte B → biblioteca proxy
build/libproxyclaves.so: src/proxy-mq.c include/mensajes.h include/claves.h
	$(CC) $(FLAGS_LIB) -Iinclude -o build/libproxyclaves.so src/proxy-mq.c -lrt

# ── EJECUTABLES ──────────────────────────────
# Servidor
build/servidor_mq: src/servidor-mq.c include/pool.h build/libclaves.so
	$(CC) $(CFLAGS) -Iinclude -o build/servidor_mq src/servidor-mq.c -Lbuild -lclaves -lrt -lpthread

# Cliente Parte A → sin colas, 1 cliente
build/app_cliente_local: test/app-cliente-1.c build/libclaves.so
	$(CC) $(CFLAGS) -Iinclude -o build/app_cliente_local test/app-cliente-1.c -Lbuild -lclaves -lpthread

# Cliente Parte A enlazado con proxy → para probar distribuido con 1 cliente
build/app_cliente_local_mq: test/app-cliente-1.c build/libproxyclaves.so
	$(CC) $(CFLAGS) -Iinclude -o build/app_cliente_local_mq test/app-cliente-1.c -Lbuild -lproxyclaves -lrt

# Cliente Parte B → con colas, varios clientes
build/app_cliente_mq: test/app-cliente-2.c build/libproxyclaves.so
	$(CC) $(CFLAGS) -Iinclude -o build/app_cliente_mq test/app-cliente-2.c -Lbuild -lproxyclaves -lrt

# ── LIMPIEZA ─────────────────────────────────
clean:
	rm -rf build
	rm -f /dev/mqueue/*

# ── Lanzar ejecutables ────────────────────────
run_servidor: build/servidor_mq
	rm -f /dev/mqueue/*
	LD_LIBRARY_PATH=./build ./build/servidor_mq

# Fase 1: versión local, sin servidor
run_cliente_local: build/app_cliente_local
	LD_LIBRARY_PATH=./build ./build/app_cliente_local

# Fase 2: versión distribuida, 1 solo cliente
# (el servidor debe estar corriendo en otra terminal: make run_servidor)
run_cliente_mq_unico: build/app_cliente_local_mq
	LD_LIBRARY_PATH=./build ./build/app_cliente_local_mq

# Fase 3: versión distribuida, 10 clientes concurrentes
# (el servidor debe estar corriendo en otra terminal: make run_servidor)
run_cliente_mq_concurrente: build/app_cliente_mq
	bash test/test_concurrencia.sh