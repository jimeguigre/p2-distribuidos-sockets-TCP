#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <pthread.h>
#include "claves.h"
#include "peticion.h"

// variables para la sincronización entre el hilo principal e hilos de petición
pthread_mutex_t mutex_hilo = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_hilo = PTHREAD_COND_INITIALIZER;
int peticion_copiada = 0; // Flag para confirmar la copia de datos

// abre una cola de mensajes y espera a que lleguen peticiones de los clientes
void tratar_peticion(void *data) {
    // pet: puntero que contiene la dirección de memoria donde empieza la estructura mensaje_peticion
    struct mensaje_peticion pet_local;
    struct mensaje_respuesta res;
    mqd_t q_cli;

    // copiamos los datos del puntero a nuestra pila local para evitar problemas de concurrencia con el hilo principal
    memcpy(&pet_local, (struct mensaje_peticion *)data, sizeof(struct mensaje_peticion));

    // avisamos al hilo principal de que ya puede recibir el siguiente mensaje
    pthread_mutex_lock(&mutex_hilo);
    peticion_copiada = 1;
    pthread_cond_signal(&cond_hilo); 
    pthread_mutex_unlock(&mutex_hilo);

    // usamos la copia local
    memset(&res, 0, sizeof(struct mensaje_respuesta));
    printf("[SERVIDOR] Atendiendo petición %d para la clave: '%s'\n", pet_local.operacion, pet_local.key);
    
    // se ejecuta la operación solicitada 
    if (pet_local.operacion == 0) { // 0 es SET
        // se busca en pet los valores necesarios 
        res.resultado = set_value(pet_local.key, pet_local.value1, pet_local.N_value2, pet_local.V_value2, pet_local.value3);
    } 
    else if (pet_local.operacion == 1) { // 1 es GET
        res.resultado = get_value(pet_local.key, res.value1, &res.N_value2, res.V_value2, &res.value3);
    } 
    else if (pet_local.operacion == 2) { // 2 es MODIFY
        res.resultado = modify_value(pet_local.key, pet_local.value1, pet_local.N_value2, pet_local.V_value2, pet_local.value3);
    } 
    else if (pet_local.operacion == 3) { // 3 es DELETE
        res.resultado = delete_key(pet_local.key);
    } 
    else if (pet_local.operacion == 4) { // 4 es EXIST
        res.resultado = exist(pet_local.key);
    } 
    else if (pet_local.operacion == 5) { // 5 es DESTROY
        res.resultado = destroy();
    } 
    else {
        res.resultado = -1; // operación no reconocida
    }

    // se abre la cola del cliente y el servidor envía la respuesta
    q_cli = mq_open(pet_local.q_cliente, O_WRONLY);
    if (q_cli != -1) {
        mq_send(q_cli, (const char *)&res, sizeof(res), 0);
        mq_close(q_cli);
    } else {
        perror("Error al devolver respuesta al cliente");
    }
    
    pthread_exit(NULL);
}

int main() {
    mqd_t q_servidor;
    struct mensaje_peticion pet_temp; // Buffer temporal de recepción
    struct mq_attr attr = { .mq_maxmsg = 10, .mq_msgsize = sizeof(struct mensaje_peticion) };

    // Inicialización de la cola del servidor
    mq_unlink("/SERVIDOR"); 
    q_servidor = mq_open("/SERVIDOR", O_CREAT | O_RDONLY, 0666, &attr);
    if (q_servidor == -1) {
        perror("Error al abrir la cola del servidor");
        return -1;
    }

    printf("SERVIDOR INICIADO\n");

    while (1) {
        // El main se bloquea aquí esperando un mensaje de cualquier cliente
        if (mq_receive(q_servidor, (char *)&pet_temp, sizeof(struct mensaje_peticion), NULL) != -1) {
            
            pthread_t thid;
            pthread_mutex_lock(&mutex_hilo);
            peticion_copiada = 0; // Preparamos el flag para el nuevo hilo

            // Creamos el hilo pasando la dirección de la estructura temporal
            if (pthread_create(&thid, NULL, (void *)tratar_peticion, &pet_temp) == 0) {
                pthread_detach(thid);

                // El main se detiene hasta que el hilo confirme que copió pet_temp
                while (peticion_copiada == 0) {
                    pthread_cond_wait(&cond_hilo, &mutex_hilo);
                }
            }
            pthread_mutex_unlock(&mutex_hilo);
            // Ahora que peticion_copiada es 1, el bucle vuelve arriba y pet_temp se puede reutilizar
        }
    }

    mq_close(q_servidor);
    return 0;
}