#ifndef CONSUMIDOR_H
#define CONSUMIDOR_H

#include <QThread>
#include <QSemaphore>
#include <QVector>
#include <QObject>

class Consumidor : public QThread {
    Q_OBJECT
public:
    Consumidor(int id, QVector<int>* buffer, QSemaphore* mutex, QSemaphore* empty, QSemaphore* full);
    void stop();

signals:
    void updateProgress(int value);
    void logMessage(const QString& message);
    void actualizarTiempos(int id, int consumiendo, int bloqueado, int esperando);

protected:
    void run() override;

private:
    int id;
    QVector<int>* buffer;
    QSemaphore* mutex;
    QSemaphore* empty;
    QSemaphore* full;
    bool running;

    // 🔹 Variables de estado del consumidor
    int consumiendo;
    int bloqueado;
    int esperando;
};

#endif // CONSUMIDOR_H
