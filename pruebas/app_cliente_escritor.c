#include "../claves.h"
#include <stdio.h>

int main() {
    char *key = "CLAVE_ATOMICA";
    float v2_a[] = {1.1}, v2_b[] = {9.9};
    struct Paquete p_a = {1,1,1}, p_b = {9,9,9};

    while(1) {
        modify_value(key, "VALOR_A", 1, v2_a, p_a);
        modify_value(key, "VALOR_B", 1, v2_b, p_b);
    }
    return 0;
}