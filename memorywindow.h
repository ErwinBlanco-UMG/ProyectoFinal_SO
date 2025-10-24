#ifndef MEMORYWINDOW_H
#define MEMORYWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QTimer>
#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QFontDatabase>
#include <vector>

#include "MemorySimulator.h"

class MemoryWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MemoryWindow(QWidget *parent = nullptr);
    ~MemoryWindow();

private slots:
    void onCargarMemoria();
    void onSimularMemoria();
    void onPasoMemoria();
    void onAutoStart();
    void onAutoStop();
    void onShowConfig();

private:
    // === Controles UI ===
    QPushButton   *btnCfg   = nullptr;
    QPushButton   *btnRun   = nullptr;
    QPushButton   *btnPaso  = nullptr;
    QPushButton   *btnAuto  = nullptr;
    QPushButton   *btnPausa = nullptr;
    QPushButton   *btnInfo  = nullptr;

    QTableWidget  *tbl      = nullptr;
    QLabel        *lblFallos = nullptr;
    QLabel        *lblPfr    = nullptr;
    QLabel        *lblTlb    = nullptr;
    QLabel        *lblAvg    = nullptr;

    // === Datos y simulación ===
    std::vector<int> m_pages;
    MemConfig        m_cfg;
    MemorySimulator  m_sim;
    bool             m_sessionActive = false;
    QTimer          *m_timer = nullptr;

    QString m_lastConfigHtml;

    // === Métodos auxiliares ===
    void setupMemoryUI(QWidget *parent);
    void applyStyle();
    void resetTable();
    void appendStepRow(const Step &s);
    void updateSummary(const MemResult &r);
    std::vector<int> parsePages(const QString& s);
    static QString framesToString(const std::vector<int>& f);
    void applyColumnWidths(int frames);
};

#endif // MEMORYWINDOW_H
