#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../claves.h"

#define ITERACIONES 50

int main() {
    char key[256];
    char v1[256] = "dato_estres";
    float v2[] = {1.1, 2.2};
    struct Paquete v3 = {1, 2, 3};
    int pid = getpid();

    printf("Cliente %d: Iniciando ráfaga de %d peticiones...\n", pid, ITERACIONES);

    for (int i = 0; i < ITERACIONES; i++) {
        sprintf(key, "key_%d_%d", pid, i);
        
        // Inserción rápida
        if (set_value(key, v1, 2, v2, v3) != 0) {
            printf("Cliente %d: Error en inserción %d\n", pid, i);
            return -1;
        }

        // Lectura rápida para verificar integridad
        int res_n2;
        float res_v2[32];
        char res_v1[256];
        struct Paquete res_v3;
        if (get_value(key, res_v1, &res_n2, res_v2, &res_v3) != 0) {
            printf("Cliente %d: Error en lectura %d\n", pid, i);
            return -1;
        }
    }

    printf("Cliente %d: ¡PRUEBA DE ESTRÉS FINALIZADA CON ÉXITO!\n", pid);
    return 0;
}