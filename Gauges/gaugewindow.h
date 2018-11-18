#ifndef GAUGEWINDOW_H
#define GAUGEWINDOW_H

#include <QtWidgets>
#include <QMainWindow>
#include "currentgague.h"
#include "tempgauge.h"
#include "qegauge.h"
#include "tachometer.h"
#include "global_enums.h"


class GaugeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GaugeWindow(QWidget *parent = nullptr);
    ~GaugeWindow();

public slots:
    void setcurrentGauge(I_t current[]);
    void setShaftAngle(float angle);
    void setShaftSpeed(float rpm);
    void setTemp(float temp);

private:
    QGridLayout* top_layout;
    QGroupBox* top_box;
    tempgauge* tempGauge;
    QEgauge* angleGauge;
    tachometer* tachometerGauge;
    currentGague* currentGauge;
};

#endif // MAINWINDOW_H
