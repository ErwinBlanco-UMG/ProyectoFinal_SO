#include "consumidor.h"
#include <QThread>
#include <QRandomGenerator>

Consumidor::Consumidor(int id, QVector<int>* buffer, QSemaphore* mutex, QSemaphore* empty, QSemaphore* full)
    : id(id), buffer(buffer), mutex(mutex), empty(empty), full(full),
    running(true), consumiendo(0), bloqueado(0), esperando(0) {}

void Consumidor::run() {
    while (running) {
        // 🔹 Espera un tiempo aleatorio antes de intentar consumir
        QThread::msleep(200 + QRandomGenerator::global()->bounded(400));

        // 🔹 Esperando que haya algo que consumir
        esperando++;
        emit actualizarTiempos(id, consumiendo, bloqueado, esperando);
        if (!running) break;

        full->acquire(); // Espera que haya un producto disponible
        if (!running) { full->release(); break; }

        // 🔹 Intentando entrar a sección crítica
        if (!mutex->tryAcquire()) {
            bloqueado++;
            emit actualizarTiempos(id, consumiendo, bloqueado, esperando);
            mutex->acquire();
        }

        // 🔹 Sección crítica protegida por mutex
        if (!buffer->isEmpty()) {
            buffer->removeLast();
            consumiendo++;
            emit logMessage(QString("🔵 Consumidor %1 consumió un elemento. Buffer: %2").arg(id).arg(buffer->size()));
            emit updateProgress(buffer->size());
            emit actualizarTiempos(id, consumiendo, bloqueado, esperando);
        }

        QThread::msleep(300 + QRandomGenerator::global()->bounded(500));

        mutex->release();
        empty->release();
    }

    emit logMessage(QString("🟦 Consumidor %1 detenido").arg(id));
}

void Consumidor::stop() {
    running = false;
}
