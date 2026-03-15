CC = gcc
CFLAGS = -Wall -Wextra -g
FLAGS_LIB = -fPIC -shared

# Compila todo
all: build libclaves.so libproxyclaves.so servidor_mq app_cliente_local app_cliente_mq


build:
	mkdir -p build


# ── BIBLIOTECAS ──────────────────────────────
# Parte A → biblioteca local
libclaves.so: src/claves.c include/claves.h
	$(CC) $(FLAGS_LIB) -Iinclude -o build/libclaves.so src/claves.c -lpthread

# Parte B → biblioteca proxy
libproxyclaves.so: src/proxy-mq.c include/mensajes.h include/claves.h
	$(CC) $(FLAGS_LIB) -Iinclude -o build/libproxyclaves.so src/proxy-mq.c -lrt

# ── EJECUTABLES ──────────────────────────────
# Servidor
servidor_mq: src/servidor-mq.c include/pool.h build/libclaves.so
	$(CC) -Iinclude -o build/servidor_mq src/servidor-mq.c -Lbuild -lclaves -lrt -lpthread

# Cliente Parte A → sin colas y solo 1 cliente
app_cliente_local: test/app-cliente-1.c build/libclaves.so
	$(CC) -Iinclude -o build/app_cliente_local test/app-cliente-1.c -Lbuild -lclaves -lpthread

# Cliente Parte B → con colas y varios clientes
app_cliente_mq: test/app-cliente-2.c build/libproxyclaves.so
	$(CC) -Iinclude -o build/app_cliente_mq test/app-cliente-2.c -Lbuild -lproxyclaves -lrt

# ── LIMPIEZA ─────────────────────────────────
clean:
	rm -rf build

# ── Lanzar ejecutables ─────────────────────────────────
run_cliente_local:
	LD_LIBRARY_PATH=./build ./build/app_cliente_local

run_servidor:
	rm -f /dev/mqueue/*
	LD_LIBRARY_PATH=./build ./build/servidor_mq

run_cliente_mq:
	LD_LIBRARY_PATH=./build ./build/app_cliente_mq