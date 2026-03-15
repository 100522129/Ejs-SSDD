#!/bin/bash
# Uso: ./test_concurrencia.sh
# El servidor debe estar corriendo antes:
#   LD_LIBRARY_PATH=./build ./build/servidor_mq

N=10
export LD_LIBRARY_PATH="$PWD/build"
EXEC="$PWD/build/app_cliente_mq"
FALLOS=0
PIDS=()
LOGS=()

echo "Lanzando $N clientes..."

for i in $(seq 1 $N); do
    LOG="/tmp/cli_${i}_$$.log"
    LOGS+=("$LOG")
    $EXEC > "$LOG" 2>&1 &
    PIDS+=($!)
    sleep 0.1   # 100ms entre cada cliente para no saturar la cola
done

for i in "${!PIDS[@]}"; do
    wait ${PIDS[$i]}
    if [ $? -ne 0 ]; then
        echo "[FAIL] Cliente $((i+1)) — $(grep RESULTADO ${LOGS[$i]})"
        FALLOS=$((FALLOS+1))
    else
        echo "[ OK ] Cliente $((i+1)) — $(grep RESULTADO ${LOGS[$i]})"
    fi
done

rm -f "${LOGS[@]}"
echo ""
[ $FALLOS -eq 0 ] && echo "TODOS OK ($N/$N)" || echo "FALLOS: $FALLOS/$N"
exit $FALLOS