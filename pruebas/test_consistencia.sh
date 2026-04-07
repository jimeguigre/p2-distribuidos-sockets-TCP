#!/bin/bash
export IP_TUPLAS=localhost
export PORT_TUPLAS=4500
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

# 1. Insertar la clave inicial
./app_cliente  # Asumiendo que inserta algo inicial

# 2. Lanzar escritor en segundo plano y lector
./app_cliente_escritor &
PID_ESC=$!
./app_cliente_lector

# 3. Limpiar
kill $PID_ESC
echo "Prueba de atomicidad finalizada."