# Variables del compilador
CC = gcc
CFLAGS = -Wall -Wextra -g

# Regla principal: se ejecuta por defecto si al escribir make
all: libclaves.so cliente_local

# 1. Regla para crear la biblioteca dinámica (Parte A)
libclaves.so: claves.c claves.h
	$(CC) $(CFLAGS) -fPIC -shared -o libclaves.so claves.c -lpthread

# 2. Regla para compilar el cliente enlazándolo con la biblioteca dinámica (Parte A)
cliente_local: app-cliente.c libclaves.so
	$(CC) $(CFLAGS) -o app_cliente app-cliente.c -L. -lclaves -Wl,-rpath,.

# Regla para limpiar la carpeta de ejecutables generados
clean:
	rm -f *.o *.so app_cliente