#
# Producer Consumer Activity
# CS 5600, Spring 2024
# Your Name
# Date
#

CC = gcc
INC = -I.
FLAGS = -W -Wall -g -pthread

all: prodcons

prodcons: prodcons.o
	$(CC) $(FLAGS) $^ -o $@

prodecons.o: prodcons.c
	$(CC) -c $(FLAGS) $(INC) $< -o $@

.PHONY : clean
clean:
	-rm -f prodcons *.o 
