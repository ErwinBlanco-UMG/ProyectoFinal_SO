#include "sjf.h"
#include <algorithm>
#include <limits>

std::vector<Process> Sjf::ejecutar(const std::vector<Process> &in, std::vector<GanttSlice> &timeline)
{
    int n = in.size();
    if (n == 0) return {};

    std::vector<Process> p = in;
    std::sort(p.begin(), p.end(), [](const Process &a, const Process &b){
        return a.arrival < b.arrival;
    });

    timeline.clear();

    double tiempoActual = 0.0;
    int completados = 0;
    std::vector<bool> terminado(n, false);

    while (completados < n) {
        int idx = -1;
        double menorRafaga = std::numeric_limits<double>::max();

        // 🔹 Buscar proceso listo con la menor ráfaga
        for (int i = 0; i < n; ++i) {
            if (!terminado[i] && p[i].arrival <= tiempoActual) {
                if (p[i].burst < menorRafaga) {
                    menorRafaga = p[i].burst;
                    idx = i;
                }
            }
        }

        // 🔹 Si no hay proceso disponible aún
        if (idx == -1) {
            tiempoActual++;
            continue;
        }

        // 🔹 Ejecutar proceso seleccionado
        p[idx].start = tiempoActual;
        p[idx].finish = p[idx].start + p[idx].burst;
        p[idx].waiting = p[idx].start - p[idx].arrival;
        if (p[idx].waiting < 0) p[idx].waiting = 0;
        p[idx].turnaround = p[idx].finish - p[idx].arrival;
        p[idx].response = p[idx].waiting;

        // Agregar tramo al Gantt
        timeline.push_back({p[idx].pid, p[idx].start, p[idx].finish});

        tiempoActual = p[idx].finish;
        terminado[idx] = true;
        completados++;
    }

    return p;
}
