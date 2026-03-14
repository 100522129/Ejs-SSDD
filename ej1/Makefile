CC = gcc
CFLAGS = -Wall -Wextra -g
FLAGS_LIB = -fPIC -shared

# Compila todo
all: libclaves.so libproxyclaves.so servidor_mq app_cliente_local app_cliente_mq

# ── BIBLIOTECAS ──────────────────────────────
# Parte A → biblioteca local
libclaves.so: claves.c claves.h
	$(CC) $(FLAGS_LIB) -o libclaves.so claves.c -lpthread

# Parte B → biblioteca proxy
libproxyclaves.so: proxy-mq.c mensajes.h claves.h
	$(CC) $(FLAGS_LIB) -o libproxyclaves.so proxy-mq.c -lrt

# ── EJECUTABLES ──────────────────────────────
# Servidor
servidor_mq: servidor-mq.c pool.h libclaves.so
	$(CC) -o servidor_mq servidor-mq.c -L. -lclaves -lrt -lpthread

# Cliente Parte A → sin colas
app_cliente_local: app-cliente.c libclaves.so
	$(CC) -o app_cliente_local app-cliente.c -L. -lclaves -lpthread

# Cliente Parte B → con colas
app_cliente_mq: app-cliente.c libproxyclaves.so
	$(CC) -o app_cliente_mq app-cliente.c -L. -lproxyclaves -lrt

# ── LIMPIEZA ─────────────────────────────────
clean:
	rm -f *.so *.o servidor_mq app_cliente_local app_cliente_mq
	rm -f /dev/mqueue/*

# ── Lanzar ejecutables ─────────────────────────────────
run_cliente_local:
	LD_LIBRARY_PATH=. ./app_cliente_local

run_servidor:
	rm -f /dev/mqueue/*
	LD_LIBRARY_PATH=. ./servidor_mq

run_cliente_mq:
	LD_LIBRARY_PATH=. ./app_cliente_mq