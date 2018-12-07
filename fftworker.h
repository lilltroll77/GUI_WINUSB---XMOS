#ifndef FFTWORKER_H
#define FFTWORKER_H

#include <QObject>
#include <QVector>
#include "ffft/FFTRealFixLen.h"
#define FFT_POW 18
#define FFT_LEN (1<<FFT_POW)
#define FFT_GROUPING 128
#define FFT_PLOT_POINTS (FFT_GROUPING*20)


const float fs=5E8f/26.0f/64.0f;

struct F_t{
    float binDC;
    float binReal[FFT_LEN/2-1];
    float binNyquist;
    float binImag[FFT_LEN/2-1];
    float loglogMax[FFT_PLOT_POINTS];
    float loglogMin[FFT_PLOT_POINTS];
};

struct f_t{
    float sample[FFT_LEN];
};

enum type_e{Absolute, Level , LogLog};
float dB(float a , float b);

class FFTworker : public QObject
{
    Q_OBJECT
public:
    explicit FFTworker(float *Xcorr=nullptr , QObject *parent = nullptr);

public slots:
    void calcFFT(struct F_t* X, struct f_t* x , enum type_e type , int index, QVector<int> &v_LUT);
    void useXCorr(bool state);

signals:
    void resultReady(int index , enum type_e type);


private:
     ffft::FFTRealFixLen <FFT_POW> fft_object;
     float* mls_xcorr;
     bool use_xcorr=true;


};

#endif // FFTWORKER_H

