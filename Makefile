
src= ./src/
obj= ./obj/
res= ./res/

all: 
	make clear
	make main

clear :
	-rm $(res)out.txt
	-rm $(obj)operations.o
	-rm $(obj)proc.o
	-rm $(obj)debug.o
	-rm $(obj)parser.o
	-rm $(obj)main.o
	-rm main


main: $(src)proc.c $(src)operations.c $(src)parser.c $(src)main.c
	gcc -c $(src)operations.c -o $(obj)operations.o
	gcc -c $(src)parser.c -o $(obj)parser.o
	gcc -c $(src)proc.c -o $(obj)proc.o 
	gcc -c $(src)debug.c -o $(obj)debug.o 
	gcc -c $(src)main.c -o $(obj)main.o 
	gcc $(obj)operations.o $(obj)parser.o $(obj)proc.o $(obj)debug.o $(obj)main.o -o main


.PHONY: main clear