#!/bin/bash
export IP_TUPLAS=localhost
export PORT_TUPLAS=4500
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

# Primero limpiamos la base de datos (opcional si el servidor acaba de arrancar)
# Puedes crear un pequeño cliente que llame a destroy() antes de empezar.

echo "--- PRUEBA DE EXCLUSIÓN MUTUA (DUPLICADOS) ---"

for i in {1..5}
do
   ./app_cliente_test_duplicados & 
done

wait
echo "--- Prueba finalizada ---"