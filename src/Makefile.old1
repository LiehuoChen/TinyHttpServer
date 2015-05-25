CC=gcc
CFLAG=-I. -lpthread
OBJCLIENT=client.o
OBJSERVER=server.o main.o


all:t10client t10server

t10client: $(OBJCLIENT)
	$(CC) -o t10client $^ $(CFLAG)

t10server: $(OBJSERVER) server.h
	$(CC) $^ $(CFLAG) -o t10server

%.o: %.c 
	$(CC) -c $^ -o $@ $(CFLAG)

clean:
	rm -f t10server t10client *.o
