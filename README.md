# ------ PWNED ------

## Versión SIN colas
```
app-cliente.c llama a set_value()
        ↓
libclaves.so tiene el código de set_value()
        ↓
modifica la lista enlazada en memoria RAM
        ↓
todo ocurre en el MISMO proceso
```

## Versión CON colas
```
app-cliente.c llama a set_value()
        ↓
libproxyclaves.so tiene el código de set_value()
  → pero aquí set_value() NO modifica nada
  → aquí set_value() manda un mensaje por cola
        ↓
        cola POSIX /SERVIDOR
        ↓
servidor.c recibe el mensaje
        ↓
servidor llama a set_value() de libclaves.so
        ↓
modifica la lista enlazada en memoria RAM
```