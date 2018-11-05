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
//#include "usbbulk.h"
#include "fftworker.h"
/*Do not draw several lines per pixel*/
#define DECIMATE 16
#define ABUFFERS 64
#define BUFFERS 2
#define FFT_PLOT_POINTS 512
#define FFT_N 2 /*Number of different FFTs*/

QT_CHARTS_USE_NAMESPACE
enum plots_e{IA , IB , IC, Flux , Torque , SetFlux , SetTorque};
enum FFT_e{FFT_IA , FFT_IC};

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

signals:
    void restart_stream(void);

public slots:
    void update_data(USBmem_t **usb);
    void update_FFT(int index);
    void show_Warning(QString str);


private:
    void reset_states(void);
    Ui::MainWindow *ui;
    static const int len = 7;
    QString Namestr[len]={"I phase A" , "I phase B" , "I phase C" , "Flux" , "Tourque", "Flux set" , "Tourque set" };
    QList<QPointF> list[len-2];
    QLineSeries series[len];
    QLineSeries FFTseries[2];
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
    int writeCopy=0;
    int fft_pos=0;
    struct USBmem_t copy[ABUFFERS];
    struct f_t fft_data[2][FFT_N];
    int FFT_wr_buff=0;
    int FFT_rd_buff=0;

    QThread* fft_thread[FFT_N];
    FFTworker* fft[FFT_N];
    F_t FFT[FFT_N];
    QList<QPointF> freq;
};

#endif // MAINWINDOW_H
