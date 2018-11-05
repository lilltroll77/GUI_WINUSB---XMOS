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
#include "ffft/FFTRealFixLen.h"

#define DECIMATE 16 /*Do not draw several lines per pixel*/
#define FFT_POW 16
#define FFT_LEN (1<<FFT_POW)

QT_CHARTS_USE_NAMESPACE
enum plots_e{IA , IB , IC, Flux , Torque , SetFlux , SetTorque};

struct scale_t{
    const qreal QE = 360.0/8192.0;
    const qreal Current = 1.0/8000.0/DECIMATE;
    const qreal Flux = 1.0/8192.0/DECIMATE;
    const qreal Torque = 1.0/8192.0/DECIMATE;
};

struct F_t{
    float DC;
    float Re[FFT_LEN/2-1];
    float NQ;
    float Im[FFT_LEN/2-1];
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
    void show_Warning(QString str);


private:
    Ui::MainWindow *ui;
    static const int len = 7;
    QString Namestr[len]={"I phase A" , "I phase B" , "I phase C" , "Flux" , "Tourque", "Flux set" , "Tourque set" };
    QList<QPointF> list[len-2];
    QLineSeries series[len];
    QLineSeries FFTseries;
    QChartView *IView;
    QChartView *PIView;
    QChartView *FFTView;
    QChart *I_chart;
    QChart *PI_chart;
    QChart *FFT_chart;
    qreal dt= 26*64/5e5;
    QBoxLayout* layout;
    QGroupBox *box;
    scale_t scale;
    int updates=0;
    float x[FFT_LEN];
    int writeX=0;
    ffft::FFTRealFixLen <FFT_POW> fft_object;




};

#endif // MAINWINDOW_H
