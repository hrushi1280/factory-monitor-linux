CC=gcc
CFLAGS=-pthread

all:
	$(CC) src/main.c -o main
	$(CC) src/sensor.c -o sensor
	$(CC) src/monitor.c -o monitor $(CFLAGS)

run: all
	./main

clean:
	rm -f main sensor monitor logs.txt
