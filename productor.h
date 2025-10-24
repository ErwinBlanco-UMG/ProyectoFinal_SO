#ifndef PRODUCTOR_H
#define PRODUCTOR_H

#include <QThread>
#include <QSemaphore>
#include <QVector>
#include <QObject>

class Productor : public QThread {
    Q_OBJECT
public:
    Productor(int id, QVector<int>* buffer, QSemaphore* mutex, QSemaphore* empty, QSemaphore* full);
    void stop();

signals:
    void updateProgress(int value);
    void logMessage(const QString& message);
    void actualizarTiempos(int id, int produciendo, int bloqueado, int esperando);

protected:
    void run() override;

private:
    int id;
    QVector<int>* buffer;
    QSemaphore* mutex;
    QSemaphore* empty;
    QSemaphore* full;
    bool running;

    // 🔹 Variables de estado del productor
    int produciendo;
    int bloqueado;
    int esperando;
};

#endif // PRODUCTOR_H
