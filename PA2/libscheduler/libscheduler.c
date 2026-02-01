/*
 * Programming Assignment 2
 * CS 5600, Spring 2026
 * Library for Scheduler Code
 * February 3, 2026
 */

#include "libscheduler.h"
int scheme = -1;  //If this fails, try NULL here instead of -1

/*
 * You may need to define some global variables or a struct to 
 * store your job queue elements. Feel free to do it here at the
 * top of the .c file, or in the .h file.
 */

/* See Canvas function descriptions for details of each function */

void scheduler_show_queue()
{

}

int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
	return -1;
}

int scheduler_job_finished(int job_number, int time)
{
	return -1;
}

int scheduler_quantum_expired(int time)
{
	return -1;
}

double scheduler_average_waiting_time()
{
	return 0;
}

double scheduler_average_turnaround_time()
{
	return 0;
}

double scheduler_average_response_time()
{
	return 0;
}

void scheduler_clean_up()
{

}
