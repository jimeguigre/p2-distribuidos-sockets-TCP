// carga las librerias 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"
#include "peticion.h"
#include <mqueue.h>
#include <pthread.h>


// abre una cola de mensajes y espera a que lleguen peticiones de los clientes
void tratar_peticion(void *data) {
    // pet: puntero que contiene la dirección de memoria donde empieza la estructura mensaje_peticion
    struct mensaje_peticion *pet = (struct mensaje_peticion *)data;
    struct mensaje_respuesta res;
    mqd_t q_cli;

    memset(&res, 0, sizeof(struct mensaje_respuesta));

    // LOG PARA EL PROFESOR
    printf("[SERVIDOR] Atendiendo petición %d para la clave: '%s'\n", pet->operacion, pet->key);
    
    // se ejecuta la operación solicitada 
    if (pet->operacion == 0) { // 0 es SET
        // se busca en pet los valores necesarios 
        res.resultado = set_value(pet->key, pet->value1, pet->N_value2, pet->V_value2, pet->value3);
    } 
    else if (pet->operacion == 1) { // 1 es GET
        res.resultado = get_value(pet->key, res.value1, &res.N_value2, res.V_value2, &res.value3);
    } 
    else if (pet->operacion == 2) { // 2 es MODIFY
        res.resultado = modify_value(pet->key, pet->value1, pet->N_value2, pet->V_value2, pet->value3);
    } 
    else if (pet->operacion == 3) { // 3 es DELETE
        res.resultado = delete_key(pet->key);
    } 
    else if (pet->operacion == 4) { // 4 es EXIST
        res.resultado = exist(pet->key);
    } 
    else if (pet->operacion == 5) { // 5 es DESTROY
        res.resultado = destroy();
    } 
    else {
        res.resultado = -1; // oeración no reconocida
    }

    // se abre la cola del cliente y el servidor envía la respuesta
    q_cli = mq_open(pet->q_cliente, O_WRONLY);
    if (q_cli != -1) {
        mq_send(q_cli, (const char *)&res, sizeof(res), 0);
        mq_close(q_cli);
    } else{
        perror("Error al abrir la cola del cliente");
    }
    
    free(pet);
    pthread_exit(NULL);
}


int main() {
    mqd_t q_servidor;
    struct mq_attr attr = { .mq_maxmsg = 10, .mq_msgsize = sizeof(struct mensaje_peticion) };

    // Crear la cola del servidor
    mq_unlink("/SERVIDOR"); 
    q_servidor = mq_open("/SERVIDOR", O_CREAT | O_RDONLY, 0666, &attr);

    while (1) {
        struct mensaje_peticion *pet = malloc(sizeof(struct mensaje_peticion));
        mq_receive(q_servidor, (char *)pet, sizeof(struct mensaje_peticion), NULL);
        
        // Crear hilo para que el servidor sea concurrente
        pthread_t thid;
        pthread_create(&thid, NULL, (void *)tratar_peticion, pet);
        pthread_detach(thid); // El hilo se limpia solo al terminar
    }
    return 0;
}