/*
 * Programming Assignment 2
 * CS 5600, Spring 2026
 * Scheduler Library
 * February 18, 2026
 *
 * This file implements the scheduling library used by the simulator.
 * It supports FCFS, SJF, PSJF, PRI, PPRI, and Round Robin.
 * Jobs are managed in a linked list that stays sorted based on
 * whichever scheduling policy is active.
 */

#include "libscheduler.h"

int scheme = -1;

/*
 * Each job in the system gets one of these structs.
 * We track everything we need for scheduling decisions
 * and for computing the final stats at the end.
 */
typedef struct job {
    int job_number;
    int arrival_time;
    int running_time;        /* how long this job needs to run total */
    int remaining_time;      /* how much time is left (used for PSJF) */
    int priority;
    int first_run_time;      /* when this job first got the CPU, -1 if it hasn't yet */
    int last_scheduled_time; /* when this job was last put on the CPU */
    struct job *next;
} job_t;

/* queue head and stats we accumulate as jobs finish */
static job_t *job_queue = NULL;
static int total_jobs = 0;
static int completed_jobs = 0;
static double total_waiting = 0.0;
static double total_turnaround = 0.0;
static double total_response = 0.0;

/**
 * Allocates and initializes a new job struct with the given info.
 */
static job_t *create_job(int job_number, int time, int running_time, int priority)
{
    job_t *j = (job_t *)malloc(sizeof(job_t));
    j->job_number = job_number;
    j->arrival_time = time;
    j->running_time = running_time;
    j->remaining_time = running_time;
    j->priority = priority;
    j->first_run_time = -1;
    j->last_scheduled_time = -1;
    j->next = NULL;
    return j;
}

/**
 * Compares two jobs to figure out which one should go first in the
 * ready queue. Returns negative if 'a' should go before 'b'.
 *
 * - SJF uses original running_time
 * - PSJF uses remaining_time (so partially-run jobs are compared fairly)
 * - PRI/PPRI use priority value (lower number = higher priority)
 * - FCFS/RR just go by arrival order
 *
 * Ties are always broken by arrival_time (earlier arrival wins).
 */
static int compare_jobs(job_t *a, job_t *b)
{
    switch (scheme) {
        case SJF:
            if (a->running_time != b->running_time)
                return a->running_time - b->running_time;
            return a->arrival_time - b->arrival_time;

        case PSJF:
            if (a->remaining_time != b->remaining_time)
                return a->remaining_time - b->remaining_time;
            return a->arrival_time - b->arrival_time;

        case PRI:
        case PPRI:
            if (a->priority != b->priority)
                return a->priority - b->priority;
            return a->arrival_time - b->arrival_time;

        case FCFS:
        case RR:
        default:
            return a->arrival_time - b->arrival_time;
    }
}

/**
 * Puts a job into the ready queue in the right spot.
 *
 * If protect_head is set, the job at the front of the queue (i.e. the
 * one currently running) won't be displaced. This is how we handle
 * non-preemptive schemes -- the running job stays put even if the new
 * job would normally go before it.
 *
 * For FCFS and RR we just tack the new job onto the end.
 */
static void insert_sorted(job_t *new_job, int protect_head)
{
    /* RR and FCFS both just go to the back of the line */
    if (scheme == RR || scheme == FCFS) {
        if (job_queue == NULL) {
            job_queue = new_job;
        } else {
            job_t *cur = job_queue;
            while (cur->next != NULL)
                cur = cur->next;
            cur->next = new_job;
        }
        return;
    }

    /* empty queue -- just drop it in */
    if (job_queue == NULL) {
        job_queue = new_job;
        return;
    }

    if (protect_head) {
        /*
         * Non-preemptive: skip past the head (it's running, don't touch it)
         * and find the right sorted position among the waiting jobs.
         */
        job_t *prev = job_queue;
        job_t *cur = job_queue->next;
        while (cur != NULL && compare_jobs(cur, new_job) <= 0) {
            prev = cur;
            cur = cur->next;
        }
        new_job->next = cur;
        prev->next = new_job;
    } else {
        /*
         * Preemptive: the new job might need to go right to the front
         * if it beats the currently running job.
         */
        if (compare_jobs(new_job, job_queue) < 0) {
            new_job->next = job_queue;
            job_queue = new_job;
        } else {
            job_t *prev = job_queue;
            job_t *cur = job_queue->next;
            while (cur != NULL && compare_jobs(cur, new_job) <= 0) {
                prev = cur;
                cur = cur->next;
            }
            new_job->next = cur;
            prev->next = new_job;
        }
    }
}

/**
 * Finds a job by its number and pulls it out of the queue.
 * Returns the removed job so the caller can grab info from it.
 */
static job_t *remove_job(int job_number)
{
    if (job_queue == NULL)
        return NULL;

    job_t *prev = NULL;
    job_t *cur = job_queue;
    while (cur != NULL) {
        if (cur->job_number == job_number) {
            if (prev == NULL)
                job_queue = cur->next;
            else
                prev->next = cur->next;
            cur->next = NULL;
            return cur;
        }
        prev = cur;
        cur = cur->next;
    }
    return NULL;
}

/**
 * Before we compare a new job against the running one (for PSJF),
 * we need to subtract the time the running job has used since it
 * was last scheduled. Otherwise remaining_time would be stale.
 */
static void update_running_remaining(int current_time)
{
    if (job_queue == NULL)
        return;

    job_t *running = job_queue;
    if (running->last_scheduled_time >= 0) {
        int elapsed = current_time - running->last_scheduled_time;
        running->remaining_time -= elapsed;
        running->last_scheduled_time = current_time;
    }
}


/**
 * Prints the job queue in order -- used by the simulator for debugging.
 */
void scheduler_show_queue()
{
    job_t *cur = job_queue;
    while (cur != NULL) {
        printf("%d ", cur->job_number);
        cur = cur->next;
    }
}

/**
 * Called when a brand new job shows up.
 *
 * We create a job struct, stick it in the queue, and figure out
 * who should be running. For non-preemptive schemes the current
 * job keeps going. For preemptive ones, the new job might jump
 * to the front if it's "better" (shorter remaining time or
 * higher priority).
 *
 * One tricky thing: if job_finished() just returned some job X
 * to run, but then this function gets called in the same time
 * unit and a different job Y preempts X -- then X never actually
 * ran. We need to undo X's first_run_time so its response time
 * doesn't get messed up.
 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
    job_t *new_j = create_job(job_number, time, running_time, priority);
    total_jobs++;

    /* if the queue was empty, this job goes right to the CPU */
    if (job_queue == NULL) {
        job_queue = new_j;
        new_j->first_run_time = time;
        new_j->last_scheduled_time = time;
        return new_j->job_number;
    }

    /* bring the running job's remaining_time up to date */
    update_running_remaining(time);

    int old_running = job_queue->job_number;

    /* non-preemptive: don't let the new job cut in front */
    if (scheme == FCFS || scheme == SJF || scheme == PRI) {
        insert_sorted(new_j, 1);
    } else {
        /* preemptive or RR: new job might take over */
        insert_sorted(new_j, 0);
    }

    int new_running = job_queue->job_number;

    /* handle the case where preemption just happened */
    if (new_running != old_running) {
        /*
         * The old head got bumped. If it had its first_run_time set
         * to right now (meaning job_finished picked it but it never
         * actually got CPU time), we need to clear that -- otherwise
         * its response time would be wrong.
         */
        job_t *cur = job_queue;
        while (cur != NULL) {
            if (cur->job_number == old_running) {
                if (cur->first_run_time == time)
                    cur->first_run_time = -1;
                cur->last_scheduled_time = -1;
                break;
            }
            cur = cur->next;
        }
        job_queue->last_scheduled_time = time;
    }

    /* record when this job first touches the CPU */
    if (job_queue->first_run_time < 0)
        job_queue->first_run_time = time;

    return new_running;
}

/**
 * Called when a job is done running.
 *
 * We pull it out of the queue, compute its stats, free it,
 * and return whoever should run next (or -1 if nothing's left).
 */
int scheduler_job_finished(int job_number, int time)
{
    job_t *finished = remove_job(job_number);
    if (finished == NULL)
        return -1;

    /* figure out this job's contribution to the averages */
    int turnaround = time - finished->arrival_time;
    int waiting = turnaround - finished->running_time;
    int response = (finished->first_run_time >= 0) ?
                   (finished->first_run_time - finished->arrival_time) : 0;

    total_turnaround += turnaround;
    total_waiting += waiting;
    total_response += response;
    completed_jobs++;

    free(finished);

    /* nobody left to run */
    if (job_queue == NULL)
        return -1;

    /* next job in line starts (or resumes) now */
    job_queue->last_scheduled_time = time;
    if (job_queue->first_run_time < 0)
        job_queue->first_run_time = time;

    return job_queue->job_number;
}

/**
 * Called when the RR time slice runs out.
 *
 * The current job (head of queue) gets moved to the back,
 * and the next job in line takes over.
 */
int scheduler_quantum_expired(int time)
{
    if (job_queue == NULL)
        return -1;

    /* if there's only one job, it just keeps running */
    if (job_queue->next == NULL)
        return job_queue->job_number;

    /* pull the current job off the front */
    job_t *old_head = job_queue;
    job_queue = old_head->next;
    old_head->next = NULL;

    /* stick it at the back of the queue */
    job_t *cur = job_queue;
    while (cur->next != NULL)
        cur = cur->next;
    cur->next = old_head;

    /* bookkeeping for the swap */
    old_head->last_scheduled_time = -1;
    job_queue->last_scheduled_time = time;
    if (job_queue->first_run_time < 0)
        job_queue->first_run_time = time;

    return job_queue->job_number;
}

/**
 * Returns the average waiting time across all completed jobs.
 * waiting = turnaround - running_time
 */
double scheduler_average_waiting_time()
{
    if (completed_jobs == 0)
        return 0.0;
    return total_waiting / completed_jobs;
}

/**
 * Returns the average turnaround time across all completed jobs.
 * turnaround = finish_time - arrival_time
 */
double scheduler_average_turnaround_time()
{
    if (completed_jobs == 0)
        return 0.0;
    return total_turnaround / completed_jobs;
}

/**
 * Returns the average response time across all completed jobs.
 * response = first_run_time - arrival_time
 */
double scheduler_average_response_time()
{
    if (completed_jobs == 0)
        return 0.0;
    return total_response / completed_jobs;
}

/**
 * Frees everything and resets all state.
 * Called once at the very end after all jobs are done.
 */
void scheduler_clean_up()
{
    job_t *cur = job_queue;
    while (cur != NULL) {
        job_t *tmp = cur->next;
        free(cur);
        cur = tmp;
    }
    job_queue = NULL;
    total_jobs = 0;
    completed_jobs = 0;
    total_waiting = 0.0;
    total_turnaround = 0.0;
    total_response = 0.0;
}
