#!/bin/bash

# Configuración de entorno
export IP_TUPLAS=127.0.0.1
export PORT_TUPLAS=4500
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

echo "--- INICIANDO PRUEBA DE INTEGRIDAD DEL PROTOCOLO ---"

# Ejecutar el cliente de integridad
# Este cliente envía datos en los límites (256 bytes, 32 floats...)
./app_cliente_test_protocolo

# Verificar el código de salida
if [ $? -eq 0 ]; then
    echo "Resultado: El protocolo es íntegro y consistente." 
else
    echo "Resultado: ERROR en la integridad de los datos."
fi

echo "--- Prueba finalizada ---"