#include "fcfs.h"
#include <algorithm>
#include <QFile>
#include <QTextStream>
#include <QIODevice>

std::vector<Process> Fcfs::ejecutar(std::vector<Process> procesos) {
    // Ordenar por tiempo de llegada
    std::sort(procesos.begin(), procesos.end(),
              [](const Process &a, const Process &b) {
                  return a.arrival < b.arrival;
              });

    double tiempoActual = 0.0;  // ahora es double

    for (auto &p : procesos) {
        if (tiempoActual < p.arrival)
            tiempoActual = p.arrival;  // permite decimales sin truncar

        p.start = tiempoActual;
        p.finish = p.start + p.burst;
        p.waiting = p.start - p.arrival;
        p.turnaround = p.finish - p.arrival;
        p.response = p.waiting;

        tiempoActual = p.finish;
    }

    return procesos;
}
