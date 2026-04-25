CC       = gcc
CFLAGS   = -Wall -Wextra -g
FLAGS_LIB = -fPIC -shared
IP = localhost

all: build build/libclaves.so build/clavesRPC_server build/app_cliente_1 build/app_cliente_2

build:
	mkdir -p build

build/libclaves.so: clavesRPC_client.c clavesRPC_clnt.c clavesRPC_xdr.c clavesRPC.h include/claves.h
	$(CC) $(FLAGS_LIB) -Iinclude -I/usr/include/tirpc -o build/libclaves.so clavesRPC_client.c clavesRPC_clnt.c clavesRPC_xdr.c -ltirpc

build/clavesRPC_server: clavesRPC_server.c clavesRPC_svc.c clavesRPC_xdr.c src/claves.c clavesRPC.h include/claves.h
	$(CC) $(CFLAGS) -Iinclude -I/usr/include/tirpc -o build/clavesRPC_server clavesRPC_server.c clavesRPC_svc.c clavesRPC_xdr.c src/claves.c -ltirpc -lpthread

build/app_cliente_1: test/app-cliente-1.c build/libclaves.so
	$(CC) $(CFLAGS) -Iinclude -o build/app_cliente_1 test/app-cliente-1.c -Lbuild -lclaves

build/app_cliente_2: test/app-cliente-2.c build/libclaves.so
	$(CC) $(CFLAGS) -Iinclude -o build/app_cliente_2 test/app-cliente-2.c -Lbuild -lclaves

clean:
	rm -rf build

run_servidor: build/clavesRPC_server
	./build/clavesRPC_server

run_cliente_1: build/app_cliente_1
	LD_LIBRARY_PATH=./build IP_TUPLAS=$(IP) ./build/app_cliente_1

run_cliente_2: build/app_cliente_2
	LD_LIBRARY_PATH=./build IP_TUPLAS=$(IP) ./build/app_cliente_2

run_concurrente: build/app_cliente_2
	LD_LIBRARY_PATH=./build IP_TUPLAS=$(IP) bash test/test_concurrencia.sh

.PHONY: all clean run_servidor run_cliente_1 run_cliente_2 run_concurrente