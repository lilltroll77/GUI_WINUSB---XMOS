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
#include "Gauges/gaugewindow.h"

/*Do not draw several lines per pixel*/
#define DECIMATE 16
#define ABUFFERS 64
#define BUFFERS 2
#define FFT_PLOT_POINTS 512
#define FFT_N 2 /*Number of different FFTs*/

QT_CHARTS_USE_NAMESPACE
enum plots_e{IA , IB , IC, Flux , Torque , SetFlux , SetTorque};
enum FFT_e{FFT_IA , FFT_IC , OFF=-1};

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
    explicit MainWindow(QQueue<union block_t>* fifo_ptr , QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void restart_stream(void);

public slots:
    void update_data();
    void update_FFT(int index);
    void show_Warning(QString str);


private:
    float filter(qreal x , enum plots_e plot );
    void parse(enum plots_e plot , enum FFT_e fft_plot , int &index, bool parseFFT , qreal scale);
    void parse_angle();
    void parse_lowspeed();
    void reset_states(void);
    Ui::MainWindow *ui;
    static const int len = 7;
    QString Namestr[len]={"I phase A" , "I phase B" , "I phase C" , "Flux" , "Tourque", "Flux set" , "Tourque set" };
    QList<QPointF> list[len-2];
    float angle[8192];
    int angle_pos=0;
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
    int listIndex[len];
    int writeCopy=0;
    int fft_pos[FFT_N]={0};
     struct f_t fft_data[2][FFT_N];
    int FFT_wr_buff=0;
    int FFT_rd_buff=0;
    QThread* fft_thread[FFT_N];
    FFTworker* fft[FFT_N];
    F_t FFT[FFT_N];
    QList<QPointF> freq;
    QQueue<union block_t>* fifo;
    GaugeWindow* gaugeWindow;
    struct I_t I[3]={0};
    qreal Xold[3]={0} , Yold[2]={0};
};

#endif // MAINWINDOW_H
