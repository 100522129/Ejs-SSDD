
> **Autores:**
> - Pablo Pérez Tenorio · `100522321`
> - Pablo Pino Castillo · `100522129`

---

## 📋 Índice

1. [[#1. Diseño de la Aplicación]]
    - [[#1.1 Estructura de Datos]]
    - [[#1.2 Diseño de la Versión Distribuida (Servidor)]]
    - [[#1.3 Diseño del Lado del Cliente (Proxy)]]
2. [[#2. Compilación y Generación de Ejecutables]]
    - [[#2.1 Proceso de Compilación]]
    - [[#2.2 Modos de Ejecución]]
3. [[#3. Batería de Pruebas y Resultados]]
    - [[#3.1 Pruebas Estructura de Datos]]
    - [[#3.2 Pruebas Concurrencia]]

---

## 1. Diseño de la Aplicación

El objetivo de la práctica ha consistido en desarrollar un **servicio de almacén de tuplas** del tipo `<key, value1, value2, value3>`, partiendo de una arquitectura monolítica y evolucionando hasta una **arquitectura distribuida cliente-servidor** mediante colas de mensajes POSIX.

---

### 1.1 Estructura de Datos

Para el almacenamiento de los elementos se implementó una **Lista Enlazada Simple** en memoria dinámica (`struct slist`), cumpliendo así con el requisito de no imponer un límite máximo de elementos sin necesitar instalaciones adicionales como bases de datos.

#### Nodo de la lista

Cada nodo (`struct NodoTupla`) almacena:

| Campo      | Descripción                      |
| ---------- | -------------------------------- |
| `key`      | Clave identificadora de la tupla |
| `value1`   | Primer valor asociado            |
| `N_value2` | Tamaño del array `V_value2`      |
| `V_value2` | Array de valores                 |
| `value3`   | Estructura con el tercer valor   |

#### Concurrencia

> [!important] 
La estructura `slist` incluye un cerrojo `pthread_mutex_t mutex_lista`. Cada operación de la API (`set_value`, `get_value`, etc.) **bloquea el mutex antes de acceder** a la lista y lo libera al terminar, garantizando atomicidad.

---

### 1.2 Diseño de la Versión Distribuida (Servidor)

Se declara un **puntero global** `struct hilo_trabajador *pool` (inicializado a `NULL`) accesible desde cualquier función, lo que permite que tanto el hilo principal como los trabajadores puedan referenciar el pool en cualquier momento.

#### Inicialización

```
main() ──► Crear cola POSIX (/SERVIDOR)
       ──► Configurar atributos de la cola
       ──► Inicializar pool de hilos trabajadores
```

El **pool de hilos** evita el desperdicio de CPU que supondría crear y destruir un hilo por cada petición. En su lugar, los hilos trabajadores, una vez terminada su tarea, **no mueren sino que se duermen** esperando nuevo trabajo.

Cada entrada del pool contiene:

- **Estado**: libre u ocupado
- **Identificador único**: para localizar sus propios datos dentro del pool
- **Semáforo**: para la señalización jefe → trabajador

#### Bucle Principal

```
loop:
  mq_receive() ──► Bloquea esperando mensaje en /SERVIDOR
               ──► Recibe petición y la elimina de la cola
               ──► Busca hilo libre en el pool (protegido con mutex)
               ──► Asigna tarea y hace sem_post()
                          │
                          ▼
               [Hilo trabajador despierta]
               ──► Ejecuta operación de la librería .so
               ──► Vuelve a sem_wait() (se duerme)
```

> [!note] Ciclo de vida de un hilo trabajador 
> Al crearse, el hilo arranca directamente en la función `trabajador()`, donde inmediatamente se bloquea en `sem_wait()`. Permanece dormido hasta que el hilo jefe le asigna una tarea con `sem_post()`, sin consumir CPU innecesariamente.

---

### 1.3 Diseño del Lado del Cliente (Proxy)

El proxy (`proxy-mq.c`) **suplanta las llamadas a la API original**. Cada vez que la aplicación cliente invoca una función, el proxy sigue este flujo:

```
1. Validaciones previas
   └─ Longitud de cadenas, límites de valores → devuelve   -1 si falla

2. Empaqueta la petición (struct peticion)

3. Crea cola de respuesta exclusiva
   └─ Nombre: /CLIENTE_<pid>  (evita colisiones entre clientes)

4. Envía a /SERVIDOR y bloquea esperando (struct respuesta)

5. Desempaqueta respuesta
   └─ Limpia y cierra su cola
   └─ Devuelve resultado  (-2 si hay error de comunicación)
```

> [!tip] Función auxiliar `send_rcv()` 
> Se implementó una función auxiliar `send_rcv()` que centraliza **toda la comunicación** entre proxy y servidor (envío + recepción), y es reutilizada por todas las operaciones de la API.

---

## 2. Compilación y Generación de Ejecutables

Se desarrolló un `Makefile` que automatiza la compilación de bibliotecas dinámicas y ejecutables.

### 2.1 Proceso de Compilación

```bash
make        # o make all
```

Genera los siguientes ejecutables:

| Ejecutable          | Descripción                    |
| ------------------- | ------------------------------ |
| `app_cliente_local` | Versión monolítica             |
| `app_cliente_mq`    | Versión distribuida (cliente)  |
| `servidor_mq`       | Versión distribuida (servidor) |

> [!warning] Para ejecutar `app_cliente_mq`, el proceso `servidor_mq` debe estar corriendo previamente en segundo plano.

### 2.2 Modos de Ejecución

#### Versión monolítica

```bash
make run_cliente_local
```

#### Versión distribuida

```bash
# Terminal 1 — arrancar el servidor 
make run_servidor 

# Terminal 2 — un solo cliente (app-cliente-1 con proxy)
make run_cliente_mq_unico 

# Terminal 2 — 10 clientes concurre
make run_cliente_mq_concurrente
```

#### Limpieza

```bash
make clean
```

Elimina ejecutables, bibliotecas dinámicas y posibles **colas POSIX colgadas** en `/dev/mqueue`.

---

## 3. Batería de Pruebas y Resultados

Para verificar el correcto funcionamiento del sistema se desarrolló un cliente de pruebas (`app-cliente.c`) organizado en **7 suites independientes**, una por cada operación de la interfaz. Cada suite limpia el almacén antes de ejecutarse para partir siempre de un estado conocido.

---

### 3.1 Pruebas Estructura de Datos

| Suite       | Operación      | Casos probados                                                                                                                  |
| ----------- | -------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| **Suite 1** | `set_value`    | Inserción válida · Rechazo de clave duplicada · Validación de `N_value2` en límites (0 y 33 → No Permitido, 1 y 32 → Permitido) |
| **Suite 2** | `exist`        | Devuelve `1` si existe · `0` si nunca insertada · `0` tras destruir el almacén                                                  |
| **Suite 3** | `get_value`    | Recuperación correcta de los tres campos · Clave inexistente → `-1` · `NULL` como clave → `-1`                                  |
| **Suite 4** | `modify_value` | Actualización correcta de los tres campos · Clave inexistente → `-1` · `N_value2` fuera de rango → No Permitido                 |
| **Suite 5** | `delete_key`   | Borrado correcto · Clave deja de existir · Otras claves no afectadas · Segundo borrado → `-1`                                   |
| **Suite 6** | `destroy`      | Almacén vacío tras la llamada · `destroy` sobre almacén vacío → `0`                                                             |
| **Suite 7** | Flujo completo | Insertar → Comprobar → Modificar → Recuperar → Borrar → Reinsertar con la misma clave                                           |

---

### 3.2 Pruebas Concurrencia

Para verificar el correcto funcionamiento del servidor bajo carga concurrente se desarrolló un segundo cliente de pruebas (`app-cliente-2.c`) y un script de bash (`test_concurrencia.sh`).

**Cliente de pruebas concurrente**

A diferencia de `app-cliente-1.c`, este cliente genera sus claves usando el PID del proceso (`clave_<PID>`), garantizando que cada instancia opere sobre un espacio de claves propio sin interferir con el resto. Cada cliente realiza 9 operaciones: 3 inserciones, 3 recuperaciones verificando que los datos no se hayan corrompido, y 3 borrados.

**Metodología**

El script lanza 10 instancias del cliente simultáneamente en background y espera a que todas terminen, recogiendo el código de retorno de cada una. Si todas devuelven 0 la prueba se considera superada.

**Ejecución**

````bash
# Terminal 1
make run_servidor

# Terminal 2
make run_cliente_mq_concurrente

**Resultado obtenido**

Lanzando 10 clientes concurrentes...

  [ OK ] Cliente 1  — [PID 16043] RESULTADO: 9 / 9
  [ OK ] Cliente 2  — [PID 16045] RESULTADO: 9 / 9
  ...
  [ OK ] Cliente 10 — [PID 16061] RESULTADO: 9 / 9

  RESULTADO: TODOS OK (10/10)
````
