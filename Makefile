CC = gcc
CFLAGS = -Wall -Werror -fPIC
# LDFLAGS para cuando usamos la lógica real (servidor y cliente mono)
LDFLAGS_LOGICA = -L. -lclaves -lpthread -lrt -Wl,-rpath,.
# LDFLAGS para cuando el cliente es distribuido (usa el proxy)
LDFLAGS_PROXY = -L. -lproxyclaves -lrt -Wl,-rpath,.

LD_LIBRARY_PATH_EXPORT = export LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH

# Generamos todo: las 2 librerías y los 3 ejecutables
# se generan también los archivos usados para las pruebas
all: libclaves.so libproxyclaves.so servidor_mq app_cliente cliente_lector cliente_escritor app_cliente_test_carga app_cliente_test_duplicados

# BIBLIOTECAS

# La lógica original (Tabla Hash)
libclaves.so: claves.o
	$(CC) -shared -o libclaves.so claves.o

# El proxy (Colas de mensajes)
libproxyclaves.so: proxy_mq.o
	$(CC) -shared -o libproxyclaves.so proxy_mq.o

claves.o: claves.c claves.h
	$(CC) $(CFLAGS) -c claves.c

proxy_mq.o: proxy_mq.c claves.h peticion.h
	$(CC) $(CFLAGS) -c proxy_mq.c

# EJECUTABLES

# Servidor: usa la lógica real
servidor_mq: servidor_mq.o libclaves.so
	$(CC) -o servidor_mq servidor_mq.o $(LDFLAGS_LOGICA)

servidor_mq.o: servidor_mq.c claves.h peticion.h
	$(CC) $(CFLAGS) -c servidor_mq.c

# cliente (distribuido y monolítico, mismo archivo): usa el proxy para hablar con el servidor
app_cliente: app_cliente.o libproxyclaves.so
	$(CC) -o app_cliente app_cliente.o $(LDFLAGS_PROXY)

app_cliente.o: app_cliente.c claves.h
	$(CC) $(CFLAGS) -c app_cliente.c

# EJECUTABLES DE PRUEBA (dentor de la carpeta pruebas/)

cliente_lector: pruebas/cliente_lector.c libproxyclaves.so
	$(CC) $(CFLAGS) -o cliente_lector pruebas/cliente_lector.c $(LDFLAGS_PROXY)

cliente_escritor: pruebas/cliente_escritor.c libproxyclaves.so
	$(CC) $(CFLAGS) -o cliente_escritor pruebas/cliente_escritor.c $(LDFLAGS_PROXY)

app_cliente_test_carga: pruebas/app_cliente_test_carga.c libproxyclaves.so
	$(CC) $(CFLAGS) -o app_cliente_test_carga pruebas/app_cliente_test_carga.c $(LDFLAGS_PROXY)

app_cliente_test_duplicados: pruebas/app_cliente_test_duplicados.c libproxyclaves.so
	$(CC) $(CFLAGS) -o app_cliente_test_duplicados pruebas/app_cliente_test_duplicados.c $(LDFLAGS_PROXY)



# LIMPIEZA
clean:
	rm -f *.o *.so app_cliente servidor_mq cliente_lector cliente_escritor app_cliente_test_carga app_cliente_test_duplicados