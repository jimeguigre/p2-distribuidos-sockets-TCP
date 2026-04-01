#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "claves.h"

//Estructura para pasar el socket al hilo
typedef struct {
    int client_sock;
} ThreadArgs;

//Hilo que procesa la petición de un cliente
void *tratar_peticion(void *args) {
    ThreadArgs *targs = (ThreadArgs *)args;
    int client_sock = targs->client_sock;
    free(targs); //Libera memoria alojada dinámicamente

    int op_red;
    if (recv(client_sock, &op_red, sizeof(int), 0) > 0) {
        int op = ntohl(op_red);
        int resultado = -1;

        if (op == 0) { //SET_VALUE
            char key[256], val1[256];
            int n2_red, x_red, y_red, z_red;
            float v2[32];
            struct Paquete p;

            recv(client_sock, key, 256, 0);
            recv(client_sock, val1, 256, 0);
            
            recv(client_sock, &n2_red, sizeof(int), 0);
            int n2 = ntohl(n2_red);
            recv(client_sock, v2, n2 * sizeof(float), 0);

            recv(client_sock, &x_red, sizeof(int), 0);
            recv(client_sock, &y_red, sizeof(int), 0);
            recv(client_sock, &z_red, sizeof(int), 0);
            p.x = ntohl(x_red); p.y = ntohl(y_red); p.z = ntohl(z_red);

            resultado = set_value(key, val1, n2, v2, p);
            
            int res_red = htonl(resultado);
            send(client_sock, &res_red, sizeof(int), 0);

        } else if (op == 1) { //GET_VALUE
            char key[256], val1[256];
            int n2;
            float v2[32];
            struct Paquete p;

            recv(client_sock, key, 256, 0);
            resultado = get_value(key, val1, &n2, v2, &p);

            int res_red = htonl(resultado);
            send(client_sock, &res_red, sizeof(int), 0);

            if (resultado == 0) {
                send(client_sock, val1, 256, 0);
                
                int n2_net = htonl(n2);
                send(client_sock, &n2_net, sizeof(int), 0);
                send(client_sock, v2, n2 * sizeof(float), 0);

                int x_net = htonl(p.x); int y_net = htonl(p.y); int z_net = htonl(p.z);
                send(client_sock, &x_net, sizeof(int), 0);
                send(client_sock, &y_net, sizeof(int), 0);
                send(client_sock, &z_net, sizeof(int), 0);
            }

        } else if (op == 2) { //MODIFY_VALUE
            char key[256], val1[256];
            int n2_red, x_red, y_red, z_red;
            float v2[32];
            struct Paquete p;

            recv(client_sock, key, 256, 0);
            recv(client_sock, val1, 256, 0);
            
            recv(client_sock, &n2_red, sizeof(int), 0);
            int n2 = ntohl(n2_red);
            recv(client_sock, v2, n2 * sizeof(float), 0);

            recv(client_sock, &x_red, sizeof(int), 0);
            recv(client_sock, &y_red, sizeof(int), 0);
            recv(client_sock, &z_red, sizeof(int), 0);
            p.x = ntohl(x_red); p.y = ntohl(y_red); p.z = ntohl(z_red);

            resultado = modify_value(key, val1, n2, v2, p);
            
            int res_red = htonl(resultado);
            send(client_sock, &res_red, sizeof(int), 0);

        } else if (op == 3) { //DELETE_KEY
            char key[256];
            recv(client_sock, key, 256, 0);
            resultado = delete_key(key);
            int res_red = htonl(resultado);
            send(client_sock, &res_red, sizeof(int), 0);

        } else if (op == 4) { //EXIST
            char key[256];
            recv(client_sock, key, 256, 0);
            resultado = exist(key);
            int res_red = htonl(resultado);
            send(client_sock, &res_red, sizeof(int), 0);

        } else if (op == 5) { // DESTROY
            resultado = destroy();
            int res_red = htonl(resultado);
            send(client_sock, &res_red, sizeof(int), 0);
        }
    }

    close(client_sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <PUERTO>\n", argv[0]);
        return -1;
    }

    int port = atoi(argv[1]);
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    //Permite reutilizar el puerto instantáneamente tras cerrar el servidor
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        return -1;
    }

    listen(server_sock, 100);
    printf("Servidor TCP escuchando en puerto %d...\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);

        if (client_sock >= 0) {
            pthread_t thread_id;
            ThreadArgs *args = malloc(sizeof(ThreadArgs));
            args->client_sock = client_sock;
            
            pthread_create(&thread_id, NULL, tratar_peticion, (void *)args);
            pthread_detach(thread_id);
        }
    }

    close(server_sock);
    return 0;
}