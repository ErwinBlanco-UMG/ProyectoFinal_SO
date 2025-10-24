#include "ganttchart.h"
#include <QFont>
#include <QPen>

GanttChart::GanttChart(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Diagrama de Gantt — SJF (SRTF)");
    resize(900, 320);
}

void GanttChart::mostrarGrafico(const std::vector<GanttSlice> &lista) {
    slices = lista;
    repaint();
}

void GanttChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int startX = 50;
    const int y = 80;
    const int blockH = 40;
    const int blockW = 30;

    painter.setFont(QFont("Arial", 11, QFont::Bold));
    painter.drawText(startX, 50, "Diagrama de Gantt (SRTF)");

    if (slices.empty()) {
        painter.drawText(startX, y + 30, "⚠ No hay datos para mostrar.");
        return;
    }

    QVector<QColor> colores = {
        QColor("#F7DC6F"), QColor("#82E0AA"), QColor("#85C1E9"),
        QColor("#BB8FCE"), QColor("#F1948A"), QColor("#73C6B6"),
        QColor("#F8C471"), QColor("#A3E4D7"), QColor("#D7BDE2")
    };

    int tmax = 0;

    for (const auto &s : slices) {
        if (s.finish <= s.start) continue;

        QColor color = colores[(s.pid - 1) % colores.size()];
        painter.setBrush(color);
        painter.setPen(Qt::black);

        const int w = (s.finish - s.start) * blockW;
        QRect rect(startX + s.start * blockW, y, w, blockH);
        painter.drawRect(rect);
        painter.setFont(QFont("Arial", 9, QFont::Bold));
        painter.drawText(rect, Qt::AlignCenter, QString("P%1").arg(s.pid));

        tmax = std::max(tmax, s.finish);
    }

    const int lineY = y + blockH + 20;
    painter.setPen(Qt::black);
    painter.drawLine(startX, lineY, startX + tmax * blockW, lineY);
    painter.setFont(QFont("Arial", 8));
    for (int t = 0; t <= tmax; ++t) {
        const int x = startX + t * blockW;
        painter.drawText(x - 3, lineY + 12, QString::number(t));
    }
}



