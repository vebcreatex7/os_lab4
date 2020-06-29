
CC=g++
CFLAGS=-pthread 

all: 
	$(CC) $(CFLAGS) main.cpp -o main