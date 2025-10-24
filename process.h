#ifndef PROCESS_H
#define PROCESS_H

#include <string>

struct Process {
    int pid;
    double arrival;
    double burst;
    double start = 0;
    double finish = 0;
    double waiting = 0;
    double turnaround = 0;
    double response = 0;
};

// Bloques del Gantt (cuando hay desalojo, un proceso puede tener varios)
struct GanttSlice {
    int pid;
    int start;
    int finish;
};

#endif
