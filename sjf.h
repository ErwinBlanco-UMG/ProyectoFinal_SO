#ifndef SJF_H
#define SJF_H

#include <vector>
#include "process.h"

class Sjf {
public:
    std::vector<Process> ejecutar(const std::vector<Process> &procesos, std::vector<GanttSlice> &timeline);
};

#endif
