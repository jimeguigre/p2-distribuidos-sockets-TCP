#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../claves.h"

void imprimir_datos(char *titulo, char *v1, int n2, float *v2, struct Paquete v3) {
    printf("\n--- %s ---\n", titulo);
    printf("Value1:  %s\n", v1);
    printf("N_Value2: %d\n", n2);
    printf("V_Value2: [%.3f, %.3f, ..., %.3f]\n", v2[0], v2[1], v2[n2-1]);
    printf("Paquete (value3): x=%d, y=%d, z=%d\n", v3.x, v3.y, v3.z);
    printf("--------------------------\n");
}

int main() {
    char key[256] = "clave_protocolo_test";
    char v1[256] = "Dato con longitud maxima para verificar buffers de 256 bytes";
    float v2[32];
    for(int i=0; i<32; i++) v2[i] = i + 0.5f;
    int n2 = 32;
    struct Paquete v3 = {111, 222, 333};

    // Mostramos lo que el cliente va a enviar
    imprimir_datos("DATOS ENVIADOS POR EL PROXY", v1, n2, v2, v3);

    if (set_value(key, v1, n2, v2, v3) != 0) {
        printf("Error al insertar.\n");
        return -1;
    }

    char res_v1[256]; int res_n2; float res_v2[32]; struct Paquete res_v3;
    if (get_value(key, res_v1, &res_n2, res_v2, &res_v3) == 0) {
        
        // Mostramos lo que el proxy ha recibido y deserializado del servidor
        imprimir_datos("DATOS RECIBIDOS DESDE EL SERVIDOR", res_v1, res_n2, res_v2, res_v3);

        if (strcmp(v1, res_v1) == 0 && v3.x == res_v3.x) {
            printf("\n>>> RESULTADO: INTEGRIDAD CONFIRMADA BIT A BIT <<<\n");
        }
    }
    return 0;
}