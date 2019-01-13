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
#include "fftworker.h"
#include <QMenu>
#include <QMenuBar>
#include <QValueAxis>
#include <QStatusBar>
#include "Gauges/gaugewindow.h"
//#include "global_enums.h"
//#include "global_defines.h"
#include "fifo.h"

QT_CHARTS_USE_NAMESPACE

#define MAX_CURRRENT 32.0
#define N_MAG 7 //Motor magnets per half turn ?
struct scale_t{
    const qreal QE = 360.0/8192.0;
    const qreal Current = MAX_CURRRENT/1048576.0; //Must match the XMOS settings in CDC
    const qreal Flux = 2*MAX_CURRRENT/1048576.0;
    const qreal Torque = 2*MAX_CURRRENT/1048576.0;
    const qreal U = 2*MAX_CURRRENT/1048576.0;
    const qreal angle = N_MAG/8192.0;
};


struct signal_t{
    QAction* Off;
    //QAction* MLS16 = new QAction("MLS 2^16" ,this);
    //QAction* MLS17 = new QAction("MLS 2^17" ,this);
    QAction* MLS18;
    QAction* RND;
    QAction* Sine;
    QAction* Octave;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(fifo* fifo_ptr, QWidget *parent = nullptr);
    QVector<int> v_LUT;
    GaugeWindow* gaugeWindow;
    ~MainWindow();

signals:
    void restart_stream(void);
    void fuseStatus(bool state);
    void SignalSource(int source);
    void SignalGenerator(int index);
    void useXCorr(bool state);
    void set_statusRow(int row , quint16 val);
    void decode_DRVregs(void* reg);

public slots:
    void update_data();
    void update_FFT(int index, type_e type);
    void show_Warning(QString str);
    void currentRange(double current);
    void slot_plotTransferFunction();
    void slot_plotSensitivity();
    void slot_ZoomIn();
    void slot_ZoomOut();
    void slot_signal();
    void slot_Checkbox(int index);


private:
    enum signal_e{OFF , MLS18 , RND , SINE , OCTAVE};
    enum message{SHUTDOWN=1 , DRV_ERROR=2 , DRV_SETTINGS=4 , TEMP_CHANGED=8 , FUSE_CHANGED=16 , LOAD_CHANGED=32 , FUSE_STATE=1<<24 };

    void calcLogScale();
    float filter(qreal x , enum plots_e plot );
    double Decimate(qint32 *x , double *z);
    void updatePhaseCurrent(qreal i , struct I_t &current ,  enum plots_e plot);
    void parse(enum plots_e plot );
    void parse_angle();
    unsigned parse_lowspeed();
    void reset_states(void);
    void calcMLS();
    Ui::MainWindow *ui;
    QMenu *menuSettings;
    QMenu *menuHelp;
    QMenu *menuSignal;
    QMenuBar *menuBar;
    QAction* Signal[5];
    static const int len = 8;
    QList<QPointF> list[2*4];
    int QEangle[128/DECIMATE]={0};
    qreal iA[128/DECIMATE];
    int angle_pos=0;
    QLineSeries series[len];
    QLineSeries FFTseries[2];
    QChartView *IView;
    QChartView *PIView;
    QChartView *FFTView;
    QChart *chart1;
    QChart *chart2;
    QChart *FFT_chart;
    QValueAxis* axisY1;
    QValueAxis* axisY2;
    QValueAxis* axisX1;
    QValueAxis* axisX2;
    qreal dt= 26*64/5e5;
    QGridLayout* layout;
    QGroupBox *box;
    scale_t scale;
    int updates=0;
    int listIndex=0;
    int writeCopy=0;
    int fftIndexA=0 , fftIndexC=0;
     struct f_t fft_data[2][FFT_N];
    int FFT_wr_buff=0;
    int FFT_rd_buff=0;
    QThread* fft_thread[FFT_N];
    FFTworker* fft[FFT_N];
    F_t FFT[FFT_N];
    QList<QPointF> freq;
    QValueAxis* axisYFFT;
    //QQueue<union block_t>* fifo;
    fifo* Fifo;
    struct I_t current[3]={{0}};
    qreal Xold[3]={0} , Yold[3]={0};
    float MLScorrLevel[FFT_LEN/2];
    float mls[FFT_LEN];
    struct F_t MLS;
    ffft::FFTRealFixLen <FFT_POW> fft_object;
    quint32 expectedIndex=0;
    int DSPstates=1;
    bool useOpenGL = true;
    QStatusBar* statusbar;
    union block_t* blockU;
    QGroupBox* choiceBox[2];
    static const int plotChoiceN=8;
    QCheckBox* checkbox[2][plotChoiceN];
    QVBoxLayout* layoutBox[2];
    QSignalMapper* signalMapper;
    enum plotMode_e{ABCOut , AlphaBetaOut , CurrentIn , DiffCurrentIn , AlphaBetaIn, DQIn , DQIn_fix , SpaceVector};
    int plotMode[2]={ABCOut , CurrentIn};
    qint32 oldState=0;
    int load=0;
    qreal B[3]={ 0.003916126660547 , 0.007832253321095 , 0.003916126660547};
    qreal A[2]={ -1.815341082704568 , 0.831005589346757};
    double Z[plotChoiceN][2]={{0}};
};

#endif // MAINWINDOW_H
