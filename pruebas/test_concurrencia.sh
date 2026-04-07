#!/bin/bash

# Configurar variables de entorno
export IP_TUPLAS=localhost
export PORT_TUPLAS=4500  # debe ser el mismo puerto que el servidor 

# Configurar la ruta de las bibliotecas para que encuentre libproxyclaves.so
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

echo "--- INICIANDO PRUEBA DE CONCURRENCIA CON 5 CLIENTES ---"

# Lanzar 5 clientes en segundo plano
for i in {1..5}
do
   echo "Lanzando cliente $i..."
   ./app_cliente > "resultado_cliente_$i.txt" 2>&1 & 
done

# Esperar a que todos los procesos en segundo plano terminen
wait

echo "--- Todas las peticiones han sido procesadas ---"
echo "Revisando archivos de salida..."

# Ver brevemente si todos tuvieron éxito
grep "correcta" resultado_cliente_*.txt