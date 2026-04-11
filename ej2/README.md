# Ejercicio 2 — Almacén clave-valor sobre sockets TCP

Servicio distribuido de tuplas `<clave, value1, value2[], value3>` implementado en C. El sistema sigue una arquitectura cliente-servidor:

- **`libclaves.so`** — almacén local (lista enlazada + mutex) usado por el servidor.
- **`libproxyclaves.so`** — biblioteca cliente que redirige las operaciones al servidor vía TCP/JSON.
- **`servidor_sock`** — servidor TCP concurrente (un hilo por cliente).

## Requisitos

- GCC y Make
- POSIX threads (`-lpthread`)

## Compilar

```bash
make
```

Los binarios y bibliotecas se generan en `build/`.

## Ejecutar

Necesitas **dos terminales**.

### Terminal 1 — Servidor

```bash
make run_servidor
# Puerto por defecto: 8080. Para cambiarlo:
make run_servidor PUERTO=9090
```

### Terminal 2 — Pruebas

| Comando | Descripción |
|---|---|
| `make run_cliente_1` | Batería de pruebas funcionales (set, get, modify, delete, exist) |
| `make run_cliente_2` | Prueba de estrés con un único cliente |
| `make run_concurrente` | 10 clientes concurrentes ejecutando `app_cliente_2` en paralelo |
| `make run_test_red` | Pruebas de error de red con un simple archivo: `app_cliente_3` (**no necesita servidor corriendo**) |

Para apuntar a un servidor remoto:

```bash
make run_cliente_1 IP=192.168.1.10 PUERTO=8080
```

## Limpiar

```bash
make clean
```

## API

Definida en `include/claves.h`:

| Función | Descripción |
|---|---|
| `destroy()` | Elimina todas las tuplas |
| `set_value(key, v1, N, V2, v3)` | Inserta una tupla |
| `get_value(key, v1, N, V2, v3)` | Obtiene los valores de una clave |
| `modify_value(key, v1, N, V2, v3)` | Modifica una tupla existente |
| `delete_key(key)` | Borra una tupla por clave |
| `exist(key)` | Devuelve 1 si existe, 0 si no, -1 en error |

## Estructura del proyecto

```
ej2/
├── include/        # Cabeceras (claves.h, mensajes.h, ...)
├── src/            # Implementación (servidor, proxy, almacén, cJSON, sock_utils)
├── test/           # Clientes de prueba y script de concurrencia
└── Makefile
```
