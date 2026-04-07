#!/bin/bash
export IP_TUPLAS=localhost
export PORT_TUPLAS=4500
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

echo "--- LANZANDO TEST DE ESTRÉS (500 OPERACIONES) ---"

for i in {1..10}
do
   ./app_cliente_test_carga > "resultado_carga_$i.txt" 2>&1 & 
done

wait
echo "--- Carga finalizada ---"
# Comprobar si algún cliente reportó errores
grep "Error" resultado_carga_*.txt || echo "Resultado: Todas las operaciones correctas."