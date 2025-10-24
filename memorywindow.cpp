#include "memorywindow.h"

// ======= Funciones auxiliares =======
std::vector<int> MemoryWindow::parsePages(const QString& s) {
    std::vector<int> out;
    QRegularExpression re("[,;\\s]+");
    for (const auto& part : s.split(re, Qt::SkipEmptyParts)) {
        bool ok = false;
        int v = part.toInt(&ok);
        if (ok) out.push_back(v);
    }
    return out;
}

QString MemoryWindow::framesToString(const std::vector<int>& f) {
    QString s = "[";
    for (int i = 0; i < (int)f.size(); ++i) {
        s += (f[i] == -1 ? "-" : QString::number(f[i]));
        if (i + 1 < (int)f.size()) s += ", ";
    }
    s += "]";
    return s;
}

// ======= Constructor =======
MemoryWindow::MemoryWindow(QWidget *parent)
    : QWidget(parent)
{
    setupMemoryUI(this);
    applyStyle();

    m_timer = new QTimer(this);
    m_timer->setInterval(300);
    connect(m_timer, &QTimer::timeout, this, &MemoryWindow::onPasoMemoria);
}

// ======= Destructor =======
MemoryWindow::~MemoryWindow() = default;  // versión segura y limpia

// ======= UI de memoria =======
void MemoryWindow::setupMemoryUI(QWidget *tabMem)
{
    auto *main = new QVBoxLayout(tabMem);
    auto *boxBtns = new QGroupBox("", tabMem);
    auto *h = new QHBoxLayout(boxBtns);

    btnCfg  = new QPushButton("Configurar Memoria", boxBtns);
    btnRun  = new QPushButton("Simular (todo)", boxBtns);
    btnPaso = new QPushButton("Paso", boxBtns);
    btnAuto = new QPushButton("Auto ▶", boxBtns);
    btnPausa= new QPushButton("Pausa ⏸", boxBtns);
    h->addWidget(btnCfg);
    h->addWidget(btnRun);
    h->addWidget(btnPaso);
    h->addWidget(btnAuto);
    h->addWidget(btnPausa);
    main->addWidget(boxBtns);

    // === Tabla principal ===
    tbl = new QTableWidget(tabMem);
    tbl->setColumnCount(7);
    QStringList headers = {"Tiempo", "Referencia", "Marcos", "Reemplazo", "Fallo", "TLB", "Costo"};
    tbl->setHorizontalHeaderLabels(headers);
    tbl->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    applyColumnWidths(3);

    tbl->verticalHeader()->setVisible(false);
    tbl->setAlternatingRowColors(true);
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tbl->setSelectionMode(QAbstractItemView::NoSelection);
    tbl->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    tbl->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    tbl->setFont(mono);
    main->addWidget(tbl, 1);

    // === Resumen ===
    auto *boxSum = new QGroupBox("Resumen", tabMem);
    auto *grid = new QGridLayout(boxSum);
    lblFallos = new QLabel("-", boxSum);
    lblPfr    = new QLabel("-", boxSum);
    lblTlb    = new QLabel("-", boxSum);
    lblAvg    = new QLabel("-", boxSum);

    QFont big = font();
    big.setPointSize(big.pointSize() + 1);
    big.setBold(true);
    lblFallos->setFont(big);
    lblPfr->setFont(big);
    lblTlb->setFont(big);
    lblAvg->setFont(big);

    grid->addWidget(new QLabel("Fallos:"), 0, 0);
    grid->addWidget(lblFallos, 0, 1);
    grid->addWidget(new QLabel("PFR:"), 0, 2);
    grid->addWidget(lblPfr, 0, 3);
    grid->addWidget(new QLabel("TLB hits:"), 1, 0);
    grid->addWidget(lblTlb, 1, 1);
    grid->addWidget(new QLabel("Tiempo promedio:"), 1, 2);
    grid->addWidget(lblAvg, 1, 3);
    main->addWidget(boxSum);

    // === Botón inferior ===
    auto *footer = new QHBoxLayout();
    footer->addStretch();
    btnInfo = new QPushButton("Ver configuración", tabMem);
    btnInfo->setEnabled(false);
    btnInfo->setFixedHeight(26);
    btnInfo->setStyleSheet("QPushButton { font-size: 10px; padding: 3px 8px; }");
    footer->addWidget(btnInfo);
    main->addLayout(footer);

    // Conexiones
    connect(btnCfg,  &QPushButton::clicked, this, &MemoryWindow::onCargarMemoria);
    connect(btnRun,  &QPushButton::clicked, this, &MemoryWindow::onSimularMemoria);
    connect(btnPaso, &QPushButton::clicked, this, &MemoryWindow::onPasoMemoria);
    connect(btnAuto, &QPushButton::clicked, this, &MemoryWindow::onAutoStart);
    connect(btnPausa,&QPushButton::clicked, this, &MemoryWindow::onAutoStop);
    connect(btnInfo, &QPushButton::clicked, this, &MemoryWindow::onShowConfig);
}

// ======= Estilo =======
void MemoryWindow::applyStyle()
{
    QString css = R"(
        QPushButton {
            padding: 6px 10px;
            border: 1px solid #c9c9c9;
            border-radius: 6px;
            background: #f7f7f7;
        }
        QPushButton:hover { background:#f0f0f0; }
        QPushButton:disabled { color:#999; background:#fafafa; }
        QGroupBox { font-weight:600; margin-top:8px; }
        QHeaderView::section {
            background:#fafafa; border:1px solid #dddddd; padding:4px; font-weight:600;
        }
    )";
    this->setStyleSheet(css);
}

// ======= Ajuste de columnas =======
void MemoryWindow::applyColumnWidths(int)
{
    auto *h = tbl->horizontalHeader();
    h->setMinimumSectionSize(50);

    for (int c = 0; c < tbl->columnCount(); ++c)
        h->setSectionResizeMode(c, QHeaderView::Fixed);

    tbl->setColumnWidth(0, 70);   // Tiempo
    tbl->setColumnWidth(2, 185);  // Marcos
    tbl->setColumnWidth(4, 70);   // Fallo
    tbl->setColumnWidth(5, 70);   // TLB
    tbl->setColumnWidth(6, 80);   // Costo

    h->setSectionResizeMode(1, QHeaderView::Stretch); // Referencia
    h->setSectionResizeMode(3, QHeaderView::Stretch); // Reemplazo
}

// ======= Tabla =======
void MemoryWindow::resetTable() {
    tbl->setRowCount(0);
    lblFallos->setText("-");
    lblPfr->setText("-");
    lblTlb->setText("-");
    lblAvg->setText("-");
}

void MemoryWindow::appendStepRow(const Step &s)
{
    int row = tbl->rowCount();
    tbl->insertRow(row);

    auto makeItem = [&](const QString& text, Qt::Alignment al = Qt::AlignCenter) {
        auto *it = new QTableWidgetItem(text);
        it->setTextAlignment(al);
        return it;
    };

    tbl->setItem(row, 0, makeItem(QString("t%1").arg(s.index)));
    tbl->setItem(row, 1, makeItem(QString::number(s.page)));
    tbl->setItem(row, 2, makeItem(framesToString(s.frames), Qt::AlignVCenter | Qt::AlignLeft));
    tbl->setItem(row, 3, makeItem(s.victimPage >= 0 ? QString::number(s.victimPage) : "-"));

    auto *f = makeItem(s.pageFault ? "Sí" : "No");
    f->setForeground(s.pageFault ? Qt::red : Qt::darkGreen);
    tbl->setItem(row, 4, f);

    auto *t = makeItem(s.tlbHit ? "Sí" : "No");
    t->setForeground(s.tlbHit ? Qt::darkGreen : Qt::red);
    tbl->setItem(row, 5, t);

    tbl->setItem(row, 6, makeItem(QString::number(s.timeCost)));
    tbl->scrollToBottom();
}

void MemoryWindow::updateSummary(const MemResult &r)
{
    lblFallos->setText(QString("%1 de %2").arg(r.totalFaults).arg(r.steps.size()));
    lblPfr->setText(QString::number(r.faultRate() * 100.0, 'f', 2) + "%");
    lblTlb->setText(QString("%1 / %2 (%3)")
                        .arg(r.tlbHits)
                        .arg(r.tlbHits + r.tlbMiss)
                        .arg(QString::number(r.tlbHitRate() * 100.0, 'f', 2) + "%"));
    lblAvg->setText(QString::number(r.avgAccessTime(), 'f', 2));
}

// ======= Simulación =======
void MemoryWindow::onCargarMemoria()
{
    bool ok = false;
    int frames = QInputDialog::getInt(this, "Configurar", "Número de marcos:", 3, 1, 64, 1, &ok);
    if (!ok) return;
    QStringList pols = {"FIFO", "LRU", "OPT"};
    QString pol = QInputDialog::getItem(this, "Política", "Elige:", pols, 1, false, &ok);
    if (!ok) return;
    int tlb = QInputDialog::getInt(this, "TLB", "Tamaño TLB (0=off):", 0, 0, 1024, 1, &ok);
    if (!ok) return;
    QString seq = QInputDialog::getText(this, "Traza", "Páginas (coma o espacio)",
                                        QLineEdit::Normal, "7, 0, 1, 2, 0, 3, 0, 4", &ok);
    if (!ok || seq.trimmed().isEmpty()) return;

    m_cfg.frames = frames;
    m_cfg.policy = (pol == "FIFO") ? ReplacementPolicy::FIFO :
                       (pol == "LRU") ? ReplacementPolicy::LRU :
                       ReplacementPolicy::OPT;
    m_cfg.tlbSize = tlb;
    m_cfg.costTLBHit = 1;
    m_cfg.costMemAccess = 10;
    m_cfg.costPageFault = 200;
    m_pages = parsePages(seq);

    applyColumnWidths(frames);
    resetTable();
    btnRun->setEnabled(true);
    btnPaso->setEnabled(true);
    btnAuto->setEnabled(true);
    btnInfo->setEnabled(true);
    m_sessionActive = false;

    m_lastConfigHtml = QString("✅ <b>Configuración:</b><br>"
                               "• Marcos: <b>%1</b><br>• Política: <b>%2</b><br>"
                               "• TLB: <b>%3</b><br>• Secuencia: <b>%4</b>")
                           .arg(frames)
                           .arg(pol)
                           .arg(tlb)
                           .arg(seq);
    QMessageBox::information(this, "Configuración completada", m_lastConfigHtml);
}

void MemoryWindow::onSimularMemoria()
{
    if (m_pages.empty()) {
        QMessageBox::warning(this, "Error", "Configura primero.");
        return;
    }
    resetTable();
    auto r = m_sim.simulateAll(m_pages, m_cfg);
    for (const auto &s : r.steps) appendStepRow(s);
    tbl->scrollToBottom();
    updateSummary(r);
}

void MemoryWindow::onPasoMemoria()
{
    if (m_pages.empty()) {
        QMessageBox::warning(this, "Error", "Configura primero.");
        return;
    }
    if (!m_sessionActive) {
        resetTable();
        m_sim.begin(m_pages, m_cfg);
        m_sessionActive = true;
    }
    Step s;
    if (m_sim.step(s)) {
        appendStepRow(s);
        updateSummary(m_sim.partial());
    } else {
        updateSummary(m_sim.partial());
        m_sessionActive = false;
        m_timer->stop();
    }
}

void MemoryWindow::onAutoStart()
{
    if (m_pages.empty()) {
        QMessageBox::warning(this, "Error", "Configura primero.");
        return;
    }
    if (!m_sessionActive) {
        resetTable();
        m_sim.begin(m_pages, m_cfg);
        m_sessionActive = true;
    }
    btnAuto->setEnabled(false);
    btnPausa->setEnabled(true);
    m_timer->start();
}

void MemoryWindow::onAutoStop()
{
    if (m_timer->isActive()) {
        m_timer->stop();
        btnAuto->setEnabled(true);
        btnPausa->setEnabled(false);
    }
}

void MemoryWindow::onShowConfig()
{
    if (m_lastConfigHtml.isEmpty())
        QMessageBox::information(this, "Configuración", "Aún no configurada.");
    else
        QMessageBox::information(this, "Configuración actual", m_lastConfigHtml);
}
