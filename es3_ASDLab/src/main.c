#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <ctype.h>

#include "priority_queue.h"
#include "hybridsort.h"

// Struct representing a single task in the scheduling simulation.
typedef struct{
    int id;         // unique task identifier
    int start;      // arrival time in the system
    int length;     // execution time required
    int priority;   // higher value means higher execution priority
} Task;

// Comparison function used by hybridsort to sort tasks by ascending start time.
int start_compare(const void *a, const void *b){
    const Task *t1 = (const Task *)a;
    const Task *t2 = (const Task *)b;
    return (t1->start > t2->start) - (t1->start < t2->start); // ascending order
}

// Loads a CSV file containing tasks in the format: id,start,length,priority.
int load_tasks_from_csv(const char *fname, Task **tasks_out, int *n_out){
    FILE *f = fopen(fname, "r");
    if (!f){
        fprintf(stderr, "Error: cannot open input file.\n");
        return 0;
    }

    int capacity = 16; // initial dynamic array capacity
    int count = 0;     // number of loaded tasks

    Task *tasks = malloc((size_t)capacity * sizeof(Task));
    if (!tasks){
        fprintf(stderr, "Memory error (malloc tasks)\n");
        fclose(f);
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        Task t;
        int *fields[] = {&t.id, &t.start, &t.length, &t.priority};
        char *cursor = line;
        for (int j = 0; j < 4; j++) {
            char *end;
            errno = 0;
            long value = strtol(cursor, &end, 10);
            if (errno || end == cursor || value < INT_MIN || value > INT_MAX)
                goto invalid_csv;
            while (isspace((unsigned char)*end)) end++;
            if ((j < 3 && *end != ',') || (j == 3 && *end != '\0'))
                goto invalid_csv;
            *fields[j] = (int)value;
            cursor = end + (j < 3);
        }
        if (t.start < 0 || t.length <= 0) goto invalid_csv;

        // Expand array if needed
        if (count >= capacity){
            if (capacity > INT_MAX / 2) goto invalid_csv;
            capacity *= 2;
            Task *new_tasks = realloc(tasks, (unsigned long)capacity * sizeof(Task));
            if (!new_tasks){
                fprintf(stderr, "Memory error (realloc tasks)\n");
                free(tasks);
                fclose(f);
                return 0;
            }
            tasks = new_tasks;
        }

        tasks[count++] = t;
    }

    if (ferror(f)) goto invalid_csv;
    fclose(f);
    *tasks_out = tasks;
    *n_out = count;
    return 1;

invalid_csv:
    fprintf(stderr, "Error: invalid task CSV (expected id,start,length,priority; start >= 0, length > 0).\n");
    free(tasks);
    fclose(f);
    return 0;
}

// Comparison function used by the PriorityQueue.
int task_compare(const void *a, const void *b){
    const Task *t1 = (const Task *)a;
    const Task *t2 = (const Task *)b;

    if (t1->priority > t2->priority) return 1;
    if (t1->priority < t2->priority) return -1;

    if (t1->start < t2->start) return 1;
    if (t1->start > t2->start) return -1;

    if (t1->id < t2->id) return 1;
    if (t1->id > t2->id) return -1;

    return 0;
}

// Hash function for tasks.
// Since each task has a unique PID, we use the ID as hash key.
unsigned long task_hash(const void *a){
    const Task *t = (const Task *)a;
    return (unsigned long)t->id;
}

// Simulation of non-preemptive priority scheduling.
int scheduling_sim(Task *tasks, int n_tasks, const char *out_fname){
    FILE *f = fopen(out_fname, "w");
    if (!f){
        fprintf(stderr, "Error: cannot open output file.\n");
        return 0;
    }

    // Create priority queue using task_compare and task_hash
    PriorityQueue *pq = priority_queue_create(task_compare, task_hash);
    if (!pq){
        fprintf(stderr, "Error: cannot create PriorityQueue.\n");
        fclose(f);
        return 0;
    }

    int64_t time = 0; // current simulation time
    int next = 0; // index of the next task to consider (sorted by arrival time)

    while (next < n_tasks || priority_queue_size(pq) > 0){

        // Insert all tasks that have arrived (start <= time)
        while (next < n_tasks && tasks[next].start <= time){
            int r = priority_queue_push(pq, &tasks[next]);

            if (r != 1){
                fprintf(stderr, "Error: push failed.\n");
                priority_queue_free(pq);
                fclose(f);
                return 0;
            }
            // r == 0 means "already inserted" (should not happen since PIDs are unique)
            next++;
        }

        // If queue is empty, CPU is idle -> advance time
        if (priority_queue_size(pq) == 0){
            if (next < n_tasks){
                // Jump directly to next task arrival if possible
                if (time < tasks[next].start)
                    time = tasks[next].start;
                else
                    time++;
            } else break; // No more tasks in queue and none will arrive
            continue;
        }

        // Retrieve the highest-priority task
        Task *current = (Task *)priority_queue_top(pq);
        if (!current){
            fprintf(stderr, "Error: top() returned NULL.\n");
            priority_queue_free(pq);
            fclose(f);
            return 0;
        }

        // Execute task non-preemptively
        int64_t start_time = time;
        int64_t end_time = time + current->length;

        // Write result to output CSV
        if (fprintf(f, "%d,%" PRId64 ",%" PRId64 "\n", current->id, start_time, end_time) < 0) {
            priority_queue_free(pq);
            fclose(f);
            return 0;
        }

        // Advance simulation time
        time = end_time;

        // Remove executed task from PQ
        priority_queue_pop(pq);
    }

    priority_queue_free(pq);
    return fclose(f) == 0;
}

int main(int argc, char const *argv[]){
    if (argc != 3){
        fprintf(stderr, "Usage: %s input.csv output.csv\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *input_name  = argv[1];
    const char *output_name = argv[2];

    Task *tasks = NULL;
    int n_tasks = 0;

    if (!load_tasks_from_csv(input_name, &tasks, &n_tasks)){
        fprintf(stderr, "Error reading tasks.\n");
        exit(EXIT_FAILURE);
    }

    // Sort tasks by start time using hybridsort
    // hybridsort(base, nitems, size, threshold_k, compare)
    hybridsort(tasks, (size_t)n_tasks, sizeof(Task), 10, start_compare);

    // Run scheduling simulation
    if (!scheduling_sim(tasks, n_tasks, output_name)){
        fprintf(stderr, "Error during scheduling simulation.\n");
        free(tasks);
        exit(EXIT_FAILURE);
    }

    free(tasks);
    return 0;
}
