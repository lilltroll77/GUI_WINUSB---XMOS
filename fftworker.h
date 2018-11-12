#ifndef FFTWORKER_H
#define FFTWORKER_H

#include <QObject>
#include "ffft/FFTRealFixLen.h"
#include <QVector>
#include <QPointF>
#define FFT_POW 18
#define FFT_LEN (1<<FFT_POW)

float dB(float a , float b);

struct F_t{
    float binDC;
    float binReal[FFT_LEN/2-1];
    float binNyquist;
    float binImag[FFT_LEN/2-1];
};

struct f_t{
    float sample[FFT_LEN];
    int i=0;
};

enum type_e{Absolute, Level};


class FFTworker : public QObject
{
    Q_OBJECT
public:
    explicit FFTworker(QObject *parent = nullptr , int ch=0);
    struct f_t fft_data[2]; // double buffer

public slots:
    void calcFFT(QQueue<QVector<float> > *fft_queue);
    void rewind();
    bool bufferFull();

signals:
    void resultReady(QVector<QPointF>* freq , int channel);


private:
     ffft::FFTRealFixLen <FFT_POW> fft_object;
     struct F_t FFT_Data;
     const float fs=5E8f/26.0f/64.0f;
     QVector<int> v_LUT={1 , 128 , 384, 896 , 1920 ,  3968 , 8064 , 16256 , 32640 , 65408 , FFT_LEN/2-2};
     QVector<QPointF> freq;
     int channel;

};

#endif // FFTWORKER_H

