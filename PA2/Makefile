#
# Programming Assignment 2
# CS 5600, Spring 2024
# Your Name
# Date
#

CC = gcc
INC = -I.
FLAGS = -W -Wall -g

all: simulator

simulator: simulator.o libscheduler/libscheduler.o
	$(CC) $^ -o $@

libscheduler/libscheduler.o: libscheduler/libscheduler.c libscheduler/libscheduler.h
	$(CC) -c $(FLAGS) $(INC) $< -o $@

simulator.o: simulator.c libscheduler/libscheduler.h
	$(CC) -c $(FLAGS) $(INC) $< -o $@

.PHONY : clean
clean:
	-rm -f simulator *.o libscheduler/*.o
