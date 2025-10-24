#ifndef FCFS_H
#define FCFS_H

#include <vector>
#include "process.h"

class Fcfs {
public:
    std::vector<Process> ejecutar(std::vector<Process> procesos);
};

#endif // FCFS_H
