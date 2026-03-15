#!/bin/bash

# 1. Indicamos que las bibliotecas dinámicas están en la carpeta build
export LD_LIBRARY_PATH=./build:$LD_LIBRARY_PATH

echo "Iniciando servidor..."
# 2. Ejecutamos el servidor accediendo a la carpeta build
# Guardamos el log también en build para mantener la raíz limpia
./build/servidor_mq > build/servidor.log 2>&1 &
PID_SERVER=$!

sleep 1 

echo "Lanzando clientes de estrés concurrentes..."
# 3. Lanzamos los clientes accediendo a la carpeta build
for i in {1..2}; do
    ./build/app_cliente_mq &
done

echo "Esperando a que los clientes terminen..."
wait 

echo "Pruebas concurrentes finalizadas. Apagando servidor..."
kill $PID_SERVER
echo "Servidor apagado."