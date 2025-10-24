#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "process.h"
#include <vector>
#include "productor.h"
#include "consumidor.h"
#include <QSemaphore>
#include <QVector>
#include <QList>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnEjecutarAlgoritmo_clicked();
    void on_btnMostrarGantt_clicked();
    void on_btnAgregarProceso_clicked();
    void on_btnLimpiar_clicked();

    void agregarLog(QString msg);
    void actualizarBarra(int valor);
    void on_btniniciar_clicked();
    void on_btnDetener_clicked();

    void actualizarTiemposProd(int id, int produciendo, int bloqueado, int esperando);
    void actualizarTiemposCons(int id, int consumiendo, int bloqueado, int esperando);

private:
    Ui::MainWindow *ui;
    std::vector<Process> procesos;
    QVector<int> buffer;
    QSemaphore *mutex;
    QSemaphore *empty;
    QSemaphore *full;

    QList<Productor*> productores;
    QList<Consumidor*> consumidores;

private:
    std::vector<GanttSlice> timeline;  // 🔹 Guardará los tramos del Gantt
};

#endif // MAINWINDOW_H
