#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "claves.h"
#include "peticion.h"

// Función auxiliar para evitar código repetido.
// Genera un nombre de cola único basado en el PID para evitar colisiones entre clientes.
// Configura la cola con mq_maxmsg = 1 para recibir únicamente la respuesta esperada.
static int abrir_cola_respuesta(char *nombre, mqd_t *q_cliente) {
    sprintf(nombre, "/q_cli_%d", getpid());
    struct mq_attr attr = { .mq_maxmsg = 1, .mq_msgsize = sizeof(struct mensaje_respuesta) };
    
    mq_unlink(nombre); // Limpiar si existía de un crash previo
    *q_cliente = mq_open(nombre, O_CREAT | O_RDONLY, 0666, &attr);
    return (*q_cliente == -1) ? -1 : 0;
}

int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    mqd_t q_servidor, q_cliente;
    struct mensaje_peticion pet;
    struct mensaje_respuesta res;
    char nombre_cola[256];

    // validación local de parámetros antes de contactar al servidor
    if (N_value2 < 1 || N_value2 > 32 || strlen(value1) > 255) {
        return -1; // Rechazo local sin contactar al servidor
    }

    // crear una cola de respuesta única para este cliente
    if (abrir_cola_respuesta(nombre_cola, &q_cliente) == -1) return -1;

    // serialización de los argumentos en la estructura de paso de mensajes 
    memset(&pet, 0, sizeof(struct mensaje_peticion));
    pet.operacion = 0; 
    strncpy(pet.q_cliente, nombre_cola, 255);
    strncpy(pet.key, key, 255);
    strncpy(pet.value1, value1, 255);
    pet.N_value2 = N_value2;
    if (N_value2 > 0 && V_value2 != NULL)
        memcpy(pet.V_value2, V_value2, N_value2 * sizeof(float));
    pet.value3 = value3;

    // apertura de la cola del servidor y envío de la petición
    q_servidor = mq_open("/SERVIDOR", O_WRONLY);
    if (q_servidor == -1) {
        // tratamiento de errores: limpieza de recursos locales si el servidor no responde.
        mq_close(q_cliente);
        mq_unlink(nombre_cola);
        return -2;
    }

    // envío de la petición al servidor y espera de la respuesta
    mq_send(q_servidor, (const char *)&pet, sizeof(struct mensaje_peticion), 0);
    mq_receive(q_cliente, (char *)&res, sizeof(struct mensaje_respuesta), NULL);

    // se cierra y elimina la cola de respuesta tras cada operación 
    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(nombre_cola);

    return res.resultado;
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    mqd_t q_servidor, q_cliente;
    struct mensaje_peticion pet;
    struct mensaje_respuesta res;
    char nombre_cola[256];

    // crear una cola de respuesta única para este cliente
    if (abrir_cola_respuesta(nombre_cola, &q_cliente) == -1) return -1;

    // serialización de los argumentos en la estructura de paso de mensajes 
    memset(&pet, 0, sizeof(struct mensaje_peticion));
    pet.operacion = 1;
    strncpy(pet.q_cliente, nombre_cola, 255);
    strncpy(pet.key, key, 255);

    // apertura de la cola del servidor y envío de la petición
    q_servidor = mq_open("/SERVIDOR", O_WRONLY);
    if (q_servidor == -1) {
        // tratamiento de errores: limpieza de recursos locales si el servidor no responde.
        mq_close(q_cliente);
        mq_unlink(nombre_cola);
        return -2;
    }

    // envío de la petición al servidor y espera de la respuesta
    mq_send(q_servidor, (const char *)&pet, sizeof(struct mensaje_peticion), 0);
    mq_receive(q_cliente, (char *)&res, sizeof(struct mensaje_respuesta), NULL);

    if (res.resultado == 0) {
        // copia de los datos recibidos del servidor a las variables proporcionadas por el cliente
        strcpy(value1, res.value1);
        *N_value2 = res.N_value2;
        if (res.N_value2 > 0)
            memcpy(V_value2, res.V_value2, res.N_value2 * sizeof(float));
        *value3 = res.value3;
    }

    // se cierra y elimina la cola de respuesta tras cada operación 
    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(nombre_cola);

    return res.resultado;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    mqd_t q_servidor, q_cliente;
    struct mensaje_peticion pet;
    struct mensaje_respuesta res;
    char nombre_cola[256];

    // validación local de parámetros antes de contactar al servidor
    if (N_value2 < 1 || N_value2 > 32 || strlen(value1) > 255) {
        return -1; // Rechazo local sin contactar al servidor
    }
    
    // crear una cola de respuesta única para este cliente
    if (abrir_cola_respuesta(nombre_cola, &q_cliente) == -1) return -1;

    // serialización de los argumentos en la estructura de paso de mensajes 
    memset(&pet, 0, sizeof(struct mensaje_peticion));
    pet.operacion = 2;
    strncpy(pet.q_cliente, nombre_cola, 255);
    strncpy(pet.key, key, 255);
    strncpy(pet.value1, value1, 255);
    pet.N_value2 = N_value2;
    if (N_value2 > 0 && V_value2 != NULL)
        memcpy(pet.V_value2, V_value2, N_value2 * sizeof(float));
    pet.value3 = value3;

    // apertura de la cola del servidor y envío de la petición
    q_servidor = mq_open("/SERVIDOR", O_WRONLY);
    if (q_servidor == -1) {
        // tratamiento de errores: limpieza de recursos locales si el servidor no responde.
        mq_close(q_cliente);
        mq_unlink(nombre_cola);
        return -2;
    }

    // envío de la petición al servidor y espera de la respuesta
    mq_send(q_servidor, (const char *)&pet, sizeof(struct mensaje_peticion), 0);
    mq_receive(q_cliente, (char *)&res, sizeof(struct mensaje_respuesta), NULL);

    // se cierra y elimina la cola de respuesta tras cada operación
    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(nombre_cola);

    return res.resultado;
}

int delete_key(char *key) {
    mqd_t q_servidor, q_cliente;
    struct mensaje_peticion pet;
    struct mensaje_respuesta res;
    char nombre_cola[256];

    // crear una cola de respuesta única para este cliente
    if (abrir_cola_respuesta(nombre_cola, &q_cliente) == -1) return -1;

    // serialización de los argumentos en la estructura de paso de mensajes 
    memset(&pet, 0, sizeof(struct mensaje_peticion));
    pet.operacion = 3;
    strncpy(pet.q_cliente, nombre_cola, 255);
    strncpy(pet.key, key, 255);

    // apertura de la cola del servidor y envío de la petición
    q_servidor = mq_open("/SERVIDOR", O_WRONLY);
    if (q_servidor == -1) {
        mq_close(q_cliente);
        mq_unlink(nombre_cola);
        return -2;
    }

    // envío de la petición al servidor y espera de la respuesta
    mq_send(q_servidor, (const char *)&pet, sizeof(struct mensaje_peticion), 0);
    mq_receive(q_cliente, (char *)&res, sizeof(struct mensaje_respuesta), NULL);

    // se cierra y elimina la cola de respuesta tras cada operación 
    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(nombre_cola);

    return res.resultado;
}

int exist(char *key) {
    mqd_t q_servidor, q_cliente;
    struct mensaje_peticion pet;
    struct mensaje_respuesta res;
    char nombre_cola[256];

    // crear una cola de respuesta única para este cliente
    if (abrir_cola_respuesta(nombre_cola, &q_cliente) == -1) return -1;

    // serialización de los argumentos en la estructura de paso de mensajes 
    memset(&pet, 0, sizeof(struct mensaje_peticion));
    pet.operacion = 4;
    strncpy(pet.q_cliente, nombre_cola, 255);
    strncpy(pet.key, key, 255);

    // apertura de la cola del servidor y envío de la petición
    q_servidor = mq_open("/SERVIDOR", O_WRONLY);
    if (q_servidor == -1) {
        // tratamiento de errores: limpieza de recursos locales si el servidor no responde.
        mq_close(q_cliente);
        mq_unlink(nombre_cola);
        return -2;
    }

    // envío de la petición al servidor y espera de la respuesta
    mq_send(q_servidor, (const char *)&pet, sizeof(struct mensaje_peticion), 0);
    mq_receive(q_cliente, (char *)&res, sizeof(struct mensaje_respuesta), NULL);

    // se cierra y elimina la cola de respuesta tras cada operación 
    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(nombre_cola);

    return res.resultado;
}

int destroy() {
    mqd_t q_servidor, q_cliente;
    struct mensaje_peticion pet;
    struct mensaje_respuesta res;
    char nombre_cola[256];

    // crear una cola de respuesta única para este cliente
    if (abrir_cola_respuesta(nombre_cola, &q_cliente) == -1) return -1;

    // serialización de los argumentos en la estructura de paso de mensajes 
    memset(&pet, 0, sizeof(struct mensaje_peticion));
    pet.operacion = 5;
    strncpy(pet.q_cliente, nombre_cola, 255);

    // apertura de la cola del servidor y envío de la petición
    q_servidor = mq_open("/SERVIDOR", O_WRONLY);
    if (q_servidor == -1) {
        // tratamiento de errores: limpieza de recursos locales si el servidor no responde.
        mq_close(q_cliente);
        mq_unlink(nombre_cola);
        return -2;
    }

    // envío de la petición al servidor y espera de la respuesta
    mq_send(q_servidor, (const char *)&pet, sizeof(struct mensaje_peticion), 0);
    mq_receive(q_cliente, (char *)&res, sizeof(struct mensaje_respuesta), NULL);

    // se cierra y elimina la cola de respuesta tras cada operación 
    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(nombre_cola);

    return res.resultado;
}