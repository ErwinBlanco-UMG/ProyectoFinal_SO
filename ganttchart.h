#ifndef GANTTCHART_H
#define GANTTCHART_H

#include <QWidget>
#include <QPainter>
#include <vector>
#include "process.h"

class GanttChart : public QWidget {
    Q_OBJECT
public:
    explicit GanttChart(QWidget *parent = nullptr);
    void mostrarGrafico(const std::vector<GanttSlice> &lista);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<GanttSlice> slices;
};

#endif
