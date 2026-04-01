#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "claves.h"


// FUNCIÓN AUXILIAR: Establecer la conexión TCP con el servidor
// Lee las variables de entorno indicadas en el enunciado.

static int conectar_servidor() {
    char *ip_str = getenv("IP_TUPLAS");
    char *port_str = getenv("PORT_TUPLAS");

    if (ip_str == NULL || port_str == NULL) {
        printf("ERROR: Variables de entorno IP_TUPLAS o PORT_TUPLAS no definidas.\n");
        return -1;
    }

    int port = atoi(port_str);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    struct hostent *he = gethostbyname(ip_str);
    if (he == NULL) {
        close(sock);
        return -1;
    }
    memcpy(&server_addr.sin_addr, he->h_addr_list[0], he->h_length);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}


// IMPLEMENTACIÓN DE LA API


int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    if (N_value2 < 1 || N_value2 > 32 || strlen(value1) > 255) return -1;

    int sock = conectar_servidor();
    if (sock < 0) return -1;

    //Opcode: 0 para set_value
    int op = htonl(0);
    send(sock, &op, sizeof(int), 0);

    //Key (tamaño fijo 256 para simplificar protocolo)
    char key_buf[256] = {0};
    strncpy(key_buf, key, 255);
    send(sock, key_buf, 256, 0);

    //Value1
    char val1_buf[256] = {0};
    strncpy(val1_buf, value1, 255);
    send(sock, val1_buf, 256, 0);

    //N_value2
    int net_N2 = htonl(N_value2);
    send(sock, &net_N2, sizeof(int), 0);

    //V_value2 (array de floats)
    send(sock, V_value2, N_value2 * sizeof(float), 0);

    //value3 (struct desglosado)
    int net_x = htonl(value3.x);
    int net_y = htonl(value3.y);
    int net_z = htonl(value3.z);
    send(sock, &net_x, sizeof(int), 0);
    send(sock, &net_y, sizeof(int), 0);
    send(sock, &net_z, sizeof(int), 0);

    //Recibir resultado
    int resultado_red;
    if (recv(sock, &resultado_red, sizeof(int), 0) <= 0) {
        close(sock);
        return -1;
    }

    close(sock);
    return ntohl(resultado_red);
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    int sock = conectar_servidor();
    if (sock < 0) return -1;

    //Opcode: 1 para get_value
    int op = htonl(1);
    send(sock, &op, sizeof(int), 0);

    //Key
    char key_buf[256] = {0};
    strncpy(key_buf, key, 255);
    send(sock, key_buf, 256, 0);

    //Recibir resultado de la operación
    int res_red;
    if (recv(sock, &res_red, sizeof(int), 0) <= 0) {
        close(sock);
        return -1;
    }
    int res = ntohl(res_red);

    //Si existe, recibimos el resto de datos en el mismo orden
    if (res == 0) {
        recv(sock, value1, 256, 0);
        
        int n2_red;
        recv(sock, &n2_red, sizeof(int), 0);
        *N_value2 = ntohl(n2_red);
        
        if (*N_value2 > 0) {
            recv(sock, V_value2, (*N_value2) * sizeof(float), 0);
        }

        int x_red, y_red, z_red;
        recv(sock, &x_red, sizeof(int), 0);
        recv(sock, &y_red, sizeof(int), 0);
        recv(sock, &z_red, sizeof(int), 0);
        
        value3->x = ntohl(x_red);
        value3->y = ntohl(y_red);
        value3->z = ntohl(z_red);
    }

    close(sock);
    return res;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    if (N_value2 < 1 || N_value2 > 32 || strlen(value1) > 255) return -1;

    int sock = conectar_servidor();
    if (sock < 0) return -1;

    //Opcode: 2 para modify_value
    int op = htonl(2);
    send(sock, &op, sizeof(int), 0);

    char key_buf[256] = {0};
    strncpy(key_buf, key, 255);
    send(sock, key_buf, 256, 0);

    char val1_buf[256] = {0};
    strncpy(val1_buf, value1, 255);
    send(sock, val1_buf, 256, 0);

    int net_N2 = htonl(N_value2);
    send(sock, &net_N2, sizeof(int), 0);
    send(sock, V_value2, N_value2 * sizeof(float), 0);

    int net_x = htonl(value3.x);
    int net_y = htonl(value3.y);
    int net_z = htonl(value3.z);
    send(sock, &net_x, sizeof(int), 0);
    send(sock, &net_y, sizeof(int), 0);
    send(sock, &net_z, sizeof(int), 0);

    int resultado_red;
    if (recv(sock, &resultado_red, sizeof(int), 0) <= 0) {
        close(sock);
        return -1;
    }

    close(sock);
    return ntohl(resultado_red);
}

int delete_key(char *key) {
    int sock = conectar_servidor();
    if (sock < 0) return -1;

    //Opcode: 3 para delete_key
    int op = htonl(3);
    send(sock, &op, sizeof(int), 0);

    char key_buf[256] = {0};
    strncpy(key_buf, key, 255);
    send(sock, key_buf, 256, 0);

    int resultado_red;
    if (recv(sock, &resultado_red, sizeof(int), 0) <= 0) {
        close(sock);
        return -1;
    }

    close(sock);
    return ntohl(resultado_red);
}

int exist(char *key) {
    int sock = conectar_servidor();
    if (sock < 0) return -1;

    //Opcode: 4 para exist
    int op = htonl(4);
    send(sock, &op, sizeof(int), 0);

    char key_buf[256] = {0};
    strncpy(key_buf, key, 255);
    send(sock, key_buf, 256, 0);

    int resultado_red;
    if (recv(sock, &resultado_red, sizeof(int), 0) <= 0) {
        close(sock);
        return -1;
    }

    close(sock);
    return ntohl(resultado_red);
}

int destroy() {
    int sock = conectar_servidor();
    if (sock < 0) return -1;

    //Opcode: 5 para destroy
    int op = htonl(5);
    send(sock, &op, sizeof(int), 0);

    int resultado_red;
    if (recv(sock, &resultado_red, sizeof(int), 0) <= 0) {
        close(sock);
        return -1;
    }

    close(sock);
    return ntohl(resultado_red);
}