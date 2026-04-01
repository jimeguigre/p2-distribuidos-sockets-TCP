CC = gcc
CFLAGS = -Wall -Werror -fPIC
#LDFLAGS: quitamos -lrt porque ya no usamos mqueue
LDFLAGS_LOGICA = -L. -lclaves -lpthread -Wl,-rpath,.
LDFLAGS_PROXY = -L. -lproxyclaves -Wl,-rpath,.
LD_LIBRARY_PATH_EXPORT = export LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH

all: libclaves.so libproxyclaves.so servidor app_cliente

#BIBLIOTECAS
libclaves.so: claves.o
	$(CC) -shared -o libclaves.so claves.o

libproxyclaves.so: proxy-sock.o
	$(CC) -shared -o libproxyclaves.so proxy-sock.o

claves.o: claves.c claves.h
	$(CC) $(CFLAGS) -c claves.c

proxy-sock.o: proxy-sock.c claves.h
	$(CC) $(CFLAGS) -c proxy-sock.c

#EJECUTABLES
servidor: servidor-sock.o libclaves.so
	$(CC) -o servidor servidor-sock.o $(LDFLAGS_LOGICA)

servidor-sock.o: servidor-sock.c claves.h
	$(CC) $(CFLAGS) -c servidor-sock.c

app_cliente: app_cliente.o libproxyclaves.so
	$(CC) -o app_cliente app_cliente.o $(LDFLAGS_PROXY)

app_cliente.o: app_cliente.c claves.h
	$(CC) $(CFLAGS) -c app_cliente.c

#LIMPIEZA
clean:
	rm -f *.o *.so app_cliente servidor