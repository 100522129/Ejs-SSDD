#ifndef SOCK_UTILS_H
#define SOCK_UTILS_H

// Envía exactamente len bytes por el socket. Devuelve -1 si hay error, 0 si éxito.
int sendMessage(int socket, char *buffer, int len);

// Recibe exactamente len bytes del socket. Devuelve -1 si hay error, 0 si éxito.
int recvMessage(int socket, char *buffer, int len);

#endif