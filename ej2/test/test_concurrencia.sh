#!/bin/bash
# =============================================================
# test_concurrencia.sh — Prueba 10 clientes simultáneos
# =============================================================

N=10
EXEC="$PWD/build/app_cliente_2"
export LD_LIBRARY_PATH="$PWD/build"

PIDS=()
LOGS=()
FALLOS=0

# 1. Se lanzan los N clientes a la vez en background
#    mq_send es bloqueante, si la cola esta llena espera solo
echo "============================================"
echo "  Lanzando $N clientes concurrentes..."
echo "============================================"

for i in $(seq 1 $N); do
    LOG="/tmp/cli_${i}_$$.log"
    LOGS+=("$LOG")
    $EXEC > "$LOG" 2>&1 &
    PIDS+=($!)
done

# 2. Se espera a que todos los clientes terminen y se recogen los resultados
echo ""
for i in "${!PIDS[@]}"; do
    NUM=$((i + 1))
    wait ${PIDS[$i]}
    EXIT_CODE=$? # Línea nueva
    RESULTADO=$(grep "RESULTADO" "${LOGS[$i]}" | tail -1)

    if [ $EXIT_CODE -ne 0 ]; then  # <--- COMPROBAMOS LA VARIABLE GUARDADA
        echo "  [FAIL] Cliente $NUM — $RESULTADO"
        FALLOS=$((FALLOS + 1))
    else
        echo "  [ OK ] Cliente $NUM — $RESULTADO"
    fi
done

# 3. Se muestra el resumen final y se limpian los logs temporales
echo ""
echo "============================================"
if [ $FALLOS -eq 0 ]; then
    echo "  RESULTADO: TODOS OK ($N/$N)"
else
    echo "  RESULTADO: $FALLOS FALLOS de $N clientes"
fi
echo "============================================"

rm -f "${LOGS[@]}"
exit $FALLOS