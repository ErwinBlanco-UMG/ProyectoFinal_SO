#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "process.h"
#include "fcfs.h"
#include "sjf.h"
#include "ganttchart.h"
#include "memorywindow.h"
#include <QTime>
#include "ui_mainwindow.h"
#include "productor.h"
#include "consumidor.h"
#include <QSemaphore>
#include <QVector>
#include <QList>
#include <QThread>
#include <QVBoxLayout>

// Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mutex(nullptr)
    , empty(nullptr)
    , full(nullptr)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnSalir, &QPushButton::clicked, this, [](){
        qApp->quit();
    });
    // === CONFIGURACIÓN DE PLANIFICACIÓN ===
    ui->tblProcesos->setColumnCount(3);
    ui->tblProcesos->setHorizontalHeaderLabels({"PID", "Llegada", "Ráfaga"});
    ui->tblProcesos->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->cbxAlgoritmo->clear();
    ui->cbxAlgoritmo->addItems({"FCFS", "SJF"});

    // === INTEGRACIÓN DEL MÓDULO DE MEMORIA ===
    MemoryWindow *memoriaWidget = new MemoryWindow(this);
    QVBoxLayout *layoutMem = new QVBoxLayout(ui->memoria);
    layoutMem->setContentsMargins(0, 0, 0, 0);
    layoutMem->addWidget(memoriaWidget);
}

// =============================
// Destructor
// =============================
MainWindow::~MainWindow()
{
    delete ui;
}

// ===========================================================
// ===================  PLANIFICACIÓN CPU  ===================
// ===========================================================

// --- Botón: Ejecutar Algoritmo ---
void MainWindow::on_btnEjecutarAlgoritmo_clicked()
{
    procesos.clear();
    int filas = ui->tblProcesos->rowCount();

    // 🔹 Recorre las filas de la tabla y obtiene los datos
    for (int i = 0; i < filas; ++i) {
        if (!ui->tblProcesos->item(i, 1) || !ui->tblProcesos->item(i, 2))
            continue; // Si alguna celda está vacía, se omite

        Process p;
        p.pid = ui->tblProcesos->item(i, 0)->text().toInt();
        p.arrival = ui->tblProcesos->item(i, 1)->text().toDouble();  // ✅ antes .toInt()
        p.burst = ui->tblProcesos->item(i, 2)->text().toDouble();    // ✅ antes .toInt()

        procesos.push_back(p);
    }

    // ⚠️ Si no hay procesos válidos, muestra advertencia
    if (procesos.empty()) {
        ui->txtSalidaPlanificacion->setText("⚠️ No hay procesos válidos para ejecutar.");
        return;
    }

    // 🔸 Elegimos el algoritmo
    QString algoritmo = ui->cbxAlgoritmo->currentText();
    std::vector<Process> resultado;
    timeline.clear();

    if (algoritmo == "SJF") {
        Sjf sjf;
        resultado = sjf.ejecutar(procesos, timeline); // 🔹 devuelve también timeline
    } else {
        Fcfs fcfs;
        resultado = fcfs.ejecutar(procesos);
        for (const auto &p : resultado)
            if (p.finish > p.start)
                timeline.push_back({p.pid, p.start, p.finish});
    }

    // === Mostrar resultados ===
    ui->txtSalidaPlanificacion->clear();
    ui->txtSalidaPlanificacion->append("PID\tLlegada\tRáfaga\tInicio\tFin\tEspera\tRetorno\tRespuesta");

    double promEspera = 0.0, promRetorno = 0.0, promResp = 0.0;

    for (const auto& p : resultado) {
        ui->txtSalidaPlanificacion->append(
            QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8")
                .arg(p.pid)
                .arg(QString::number(p.arrival, 'f', 2))
                .arg(QString::number(p.burst, 'f', 2))
                .arg(QString::number(p.start, 'f', 2))
                .arg(QString::number(p.finish, 'f', 2))
                .arg(QString::number(p.waiting, 'f', 2))
                .arg(QString::number(p.turnaround, 'f', 2))
                .arg(QString::number(p.response, 'f', 2))
            );

        promEspera  += p.waiting;
        promRetorno += p.turnaround;
        promResp    += p.response;
    }

    int n = resultado.size();
    if (n > 0) {
        ui->txtSalidaPlanificacion->append(
            QString("\nPromedios → Espera: %1 | Retorno: %2 | Respuesta: %3")
                .arg(promEspera / n, 0, 'f', 2)
                .arg(promRetorno / n, 0, 'f', 2)
                .arg(promResp / n, 0, 'f', 2)
            );
    }

    ui->txtSalidaPlanificacion->append("\n🔹 Tiempos de espera individuales:");
    for (const auto& p : resultado) {
        ui->txtSalidaPlanificacion->append(
            QString("  • Proceso %1 → Tiempo de espera: %2")
                .arg(p.pid)
                .arg(QString::number(p.waiting, 'f', 2))
            );
    }
}

// --- Botón: Agregar proceso ---
void MainWindow::on_btnAgregarProceso_clicked()
{
    int fila = ui->tblProcesos->rowCount();
    ui->tblProcesos->insertRow(fila);

    // PID automático
    QTableWidgetItem *pidItem = new QTableWidgetItem(QString::number(fila + 1));
    ui->tblProcesos->setItem(fila, 0, pidItem);

    // Deja las columnas 1 y 2 vacías para que el usuario las escriba
    ui->tblProcesos->setItem(fila, 1, new QTableWidgetItem(""));
    ui->tblProcesos->setItem(fila, 2, new QTableWidgetItem(""));
}

// --- Botón: Limpiar ---
void MainWindow::on_btnLimpiar_clicked()
{
    ui->tblProcesos->setRowCount(0);
    ui->txtSalidaPlanificacion->clear();
    procesos.clear();
    ui->txtSalidaPlanificacion->append("🧹 Datos borrados correctamente. Puede ingresar nuevos procesos.");
}

// --- Botón: Mostrar Gantt ---
void MainWindow::on_btnMostrarGantt_clicked()
{
    if (timeline.empty()) {
        ui->txtSalidaPlanificacion->append("⚠ No hay datos del Gantt. Ejecuta el algoritmo primero.");
        return;
    }

    auto *ventana = new GanttChart();
    ventana->setAttribute(Qt::WA_DeleteOnClose);
    ventana->mostrarGrafico(timeline);
    ventana->show();
}

// -----------------------------------------SEMAFOROS-------------------------------- //



void MainWindow::on_btniniciar_clicked()
{
    on_btnDetener_clicked();

    int numProd = ui->spnProductores->value();
    int numCons = ui->spnConsumidores->value();
    int bufferSize = ui->spnBuffer->value();

    buffer.clear();
    buffer.reserve(bufferSize);
    mutex = new QSemaphore(1);
    empty = new QSemaphore(bufferSize);
    full = new QSemaphore(0);

    ui->progressBar->setMaximum(bufferSize);
    ui->progressBar->setValue(0);
    ui->listWidget->clear();

    // Configurar tablas
    ui->tblProductores->setRowCount(numProd);
    ui->tblProductores->setColumnCount(4);
    QStringList headersProd = {"ID", "Produciendo", "Bloqueado", "Esperando"};
    ui->tblProductores->setHorizontalHeaderLabels(headersProd);

    ui->tblConsumidores->setRowCount(numCons);
    ui->tblConsumidores->setColumnCount(4);
    QStringList headersCons = {"ID", "Consumiendo", "Bloqueado", "Esperando"};
    ui->tblConsumidores->setHorizontalHeaderLabels(headersCons);

    // Crear Productores
    for (int i = 0; i < numProd; ++i) {
        Productor *p = new Productor(i + 1, &buffer, mutex, empty, full);
        productores.append(p);
        connect(p, &Productor::logMessage, this, &MainWindow::agregarLog);
        connect(p, &Productor::updateProgress, this, &MainWindow::actualizarBarra);
        connect(p, &Productor::actualizarTiempos, this, &MainWindow::actualizarTiemposProd);
        p->start();
    }

    // Crear Consumidores
    for (int i = 0; i < numCons; ++i) {
        Consumidor *c = new Consumidor(i + 1, &buffer, mutex, empty, full);
        consumidores.append(c);
        connect(c, &Consumidor::logMessage, this, &MainWindow::agregarLog);
        connect(c, &Consumidor::updateProgress, this, &MainWindow::actualizarBarra);
        connect(c, &Consumidor::actualizarTiempos, this, &MainWindow::actualizarTiemposCons);
        c->start();
    }

    ui->listWidget->addItem("✅ Simulación iniciada correctamente.");
}


void MainWindow::on_btnDetener_clicked()
{
    ui->listWidget->addItem("🟥 Solicitando detención de hilos...");
ui->listWidget->scrollToBottom();

// 1) Marcar stop en todos
for (auto p : productores) p->stop();
for (auto c : consumidores) c->stop();

// 2) Liberar semáforos para despertar hilos que estén en acquire() bloqueante
//    libera suficientes permisos para que ninguno quede colgado.
int totalThreads = productores.size() + consumidores.size();
int bufSize = buffer.size();
int releases = totalThreads + bufSize + 5;

if (empty) {
    for (int i=0;i<releases;i++) empty->release(1);
}
if (full) {
    for (int i=0;i<releases;i++) full->release(1);
}
if (mutex) {
    // un release por si alguien esperaba mutex
    mutex->release(1);
}

// 3) Esperar a que terminen y eliminar objetos
for (auto p : productores) { p->wait(); delete p; }
productores.clear();

for (auto c : consumidores) { c->wait(); delete c; }
consumidores.clear();

// 4) limpiar recursos UI y semáforos
buffer.clear();
ui->progressBar->setValue(0);

delete mutex; delete empty; delete full;
mutex = empty = full = nullptr;

ui->listWidget->addItem("✅ Simulación detenida.");
ui->listWidget->scrollToBottom();
}


void MainWindow::agregarLog(QString msg)
{
    QString time = QTime::currentTime().toString("hh:mm:ss.zzz");
    ui->listWidget->addItem("[" + time + "] " + msg);
    ui->listWidget->scrollToBottom();
}

void MainWindow::actualizarBarra(int valor)
{
    ui->progressBar->setValue(valor);
}

void MainWindow::actualizarTiemposProd(int id, int produciendo, int bloqueado, int esperando)
{
    int row = id - 1;
    ui->tblProductores->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
    ui->tblProductores->setItem(row, 1, new QTableWidgetItem(QString::number(produciendo)));
    ui->tblProductores->setItem(row, 2, new QTableWidgetItem(QString::number(bloqueado)));
    ui->tblProductores->setItem(row, 3, new QTableWidgetItem(QString::number(esperando)));
}

void MainWindow::actualizarTiemposCons(int id, int consumiendo, int bloqueado, int esperando)
{
    int row = id - 1;
    ui->tblConsumidores->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
    ui->tblConsumidores->setItem(row, 1, new QTableWidgetItem(QString::number(consumiendo)));
    ui->tblConsumidores->setItem(row, 2, new QTableWidgetItem(QString::number(bloqueado)));
    ui->tblConsumidores->setItem(row, 3, new QTableWidgetItem(QString::number(esperando)));
}
