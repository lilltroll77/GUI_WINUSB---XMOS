#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QChart>
#include <QLineSeries>
#include <QMainWindow>
#include <QChartView>
#include <QWidget>
#include <QGroupBox>
#include <QLayout>
#include "data_struct.h"

#define DECIMATE 16

QT_CHARTS_USE_NAMESPACE
enum plots_e{IA , IB , IC, Flux , Torque , SetFlux , SetTorque};

struct scale_t{
    const qreal QE = 360.0/8192.0;
    const qreal Current = 1.0/8000.0/DECIMATE;
    const qreal Flux = 1.0/8192.0/DECIMATE;
    const qreal Torque = 1.0/8192.0/DECIMATE;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void update_data(USBmem_t **usb);


private:
    Ui::MainWindow *ui;
    static const int len = 7;
    QString Namestr[len]={"I phase A" , "I phase B" , "I phase C" , "Flux" , "Tourque", "Flux set" , "Tourque set" };
    QList<QPointF> list[len-2];
    QLineSeries series[len];
    QChartView *IView;
    QChartView *PIView;
    QChart *I_chart;
    QChart *PI_chart;
    qreal dt= 26*64/5e5;
    QBoxLayout* layout;
    QGroupBox *box;
    scale_t scale;
    int updates=0;



};

#endif // MAINWINDOW_H
