# CPU Scheduler Library

A process scheduling library written in C that implements six classic CPU scheduling algorithms. Built as an event-driven simulator that reads jobs from CSV files and schedules them according to the selected policy.

## Scheduling Algorithms

| Algorithm | Description | Preemptive |
|-----------|-------------|:----------:|
| **FCFS** | First Come First Served — simple FIFO queue | ❌ |
| **SJF** | Shortest Job First — picks the job with the shortest burst time | ❌ |
| **PSJF** | Preemptive Shortest Job First — preempts if a shorter job arrives | ✅ |
| **PRI** | Priority Scheduling — lower number = higher priority | ❌ |
| **PPRI** | Preemptive Priority — preempts if a higher-priority job arrives | ✅ |
| **RR** | Round Robin — each job gets a fixed time quantum, then rotates | ✅ |

## Build & Run

```bash
# Build
make clean
make

# Run
./simulator <scheme> <input_file>
```

### Examples

```bash
./simulator fcfs examples/proc1.csv
./simulator sjf  examples/proc2.csv
./simulator rr2  examples/proc1.csv     # Round Robin, quantum = 2
./simulator rr4  examples/proc2.csv     # Round Robin, quantum = 4
```

## Project Structure

```
├── simulator.c                  # Event-driven simulator (driver)
├── libscheduler/
│   ├── libscheduler.h           # Public API and scheduling scheme definitions
│   └── libscheduler.c           # Core scheduling logic
├── examples/                    # Sample CSV inputs and expected outputs
├── Makefile                     # Build configuration
└── README                       # Original project spec
```

## Implementation Details

- **Data Structure** — Jobs are managed in a sorted linked list (ready queue). Insertion order depends on the active scheduling policy.
- **Non-Preemptive Schemes** (FCFS, SJF, PRI) — The running job at the head of the queue is never displaced by a newly arriving job.
- **Preemptive Schemes** (PSJF, PPRI) — A new job can preempt the currently running job if it has a shorter remaining time or higher priority.
- **Round Robin** — New jobs are appended to the tail. When the quantum expires, the current job moves to the back of the queue.
- **Tie-Breaking** — Ties are broken by arrival time (earlier arrival wins).
- **Response Time Accuracy** — If a job is selected but immediately preempted before actually executing, its first-run timestamp is reset to preserve correct response time metrics.

## Metrics

After simulation, the library computes:

- **Average Waiting Time** — `(turnaround − burst) / jobs`
- **Average Turnaround Time** — `(finish − arrival) / jobs`
- **Average Response Time** — `(first_run − arrival) / jobs`

## Tech Stack

- **Language:** C (C99)
- **Build:** GNU Make
- **Platform:** Linux / Unix
