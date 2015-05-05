all:t10client t10server

t10client:
	gcc -o t10client client.c

t10server:
	gcc -o t10server server.c -pthread

clean:
	rm -f t10server t10client
