#include "productor.h"
#include <QThread>
#include <QRandomGenerator>

Productor::Productor(int id, QVector<int>* buffer, QSemaphore* mutex, QSemaphore* empty, QSemaphore* full)
    : id(id), buffer(buffer), mutex(mutex), empty(empty), full(full),
    running(true), produciendo(0), bloqueado(0), esperando(0) {}

void Productor::run() {
    while (running) {
        // 🔹 Espera un tiempo aleatorio antes de intentar producir
        QThread::msleep(200 + QRandomGenerator::global()->bounded(400));

        // 🔹 Esperando espacio libre
        esperando++;
        emit actualizarTiempos(id, produciendo, bloqueado, esperando);
        if (!running) break;

        empty->acquire(); // Espera espacio disponible
        if (!running) { empty->release(); break; }

        // 🔹 Intentando entrar a sección crítica
        if (!mutex->tryAcquire()) {
            bloqueado++;
            emit actualizarTiempos(id, produciendo, bloqueado, esperando);
            mutex->acquire(); // Espera a que se libere
        }

        // 🔹 Sección crítica protegida por mutex
        buffer->append(id);
        produciendo++;
        emit logMessage(QString("🟢 Productor %1 produjo un elemento. Buffer: %2").arg(id).arg(buffer->size()));
        emit updateProgress(buffer->size());
        emit actualizarTiempos(id, produciendo, bloqueado, esperando);

        QThread::msleep(300 + QRandomGenerator::global()->bounded(500));

        mutex->release();
        full->release();
    }

    emit logMessage(QString("🟥 Productor %1 detenido").arg(id));
}

void Productor::stop() {
    running = false;
}
