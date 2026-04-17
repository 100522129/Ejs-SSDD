#!/bin/bash
# test_red.sh — Prueba errores de conexión
# Debe ejecutarse SIN el servidor corriendo.

unset IP_TUPLAS
unset PORT_TUPLAS

# Ruta relativa al script, independiente del directorio de trabajo
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXEC="$SCRIPT_DIR/../build/app_test_red"
export LD_LIBRARY_PATH="$SCRIPT_DIR/../build"

FALLOS=0

check() {
    local desc="$1"
    local resultado="$2"
    if [ "$resultado" -ne 0 ]; then
        echo "  [ OK ] $desc (falló como se esperaba)"
    else
        echo "  [FAIL] $desc (debería haber fallado)"
        FALLOS=$((FALLOS + 1))
    fi
}

echo "=== PRUEBAS DE ERROR DE RED ==="

# A.1 Sin IP_TUPLAS
(unset IP_TUPLAS; export PORT_TUPLAS=8080; $EXEC > /dev/null 2>&1)
check "1. IP_TUPLAS no definida" $?

# A.2 Sin PORT_TUPLAS
(export IP_TUPLAS=localhost; unset PORT_TUPLAS; $EXEC > /dev/null 2>&1)
check "2. PORT_TUPLAS no definida" $?

# A.3 IP inalcanzable (192.0.2.x es TEST-NET, nunca responde)
# timeout 5 para no esperar el timeout completo de TCP (~1 min)
(export IP_TUPLAS=192.0.2.1; export PORT_TUPLAS=8080; timeout 5 $EXEC > /dev/null 2>&1)
check "3. IP inalcanzable" $?

# A.4 Puerto cerrado en localhost
(export IP_TUPLAS=localhost; export PORT_TUPLAS=1; $EXEC > /dev/null 2>&1)
check "4. Puerto incorrecto" $?

echo ""
echo "============================================"
[ $FALLOS -eq 0 ] && echo "TODOS OK: 4/4 pruebas pasadas" || echo "$FALLOS FALLOS"
echo "============================================"
exit $FALLOS