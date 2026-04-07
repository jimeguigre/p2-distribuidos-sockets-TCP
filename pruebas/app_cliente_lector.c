#include "../claves.h"
#include <stdio.h>
#include <string.h>

int main() {
    char v1[256]; int n2; float v2[32]; struct Paquete p;
    for (int i = 0; i < 5000; i++) {
        get_value("CLAVE_ATOMICA", v1, &n2, v2, &p);
        // Si v1 es A, x debe ser 1. Si detectamos v1=A y x=9, la atomicidad falló.
        if (strcmp(v1, "VALOR_A") == 0 && p.x != 1) {
            printf("¡ERROR DE CONSISTENCIA! Datos mezclados detectados.\n");
            return -1;
        }
    }
    printf("Consistencia confirmada tras 5000 lecturas.\n");
    return 0;
}