#ifndef FFTWORKER_H
#define FFTWORKER_H

#include <QObject>
#include <QList>
#include <QPointF>
#include "ffft/FFTRealFixLen.h"
#define FFT_POW 18
#define FFT_LEN (1<<FFT_POW)
#define LOG_LEN 512

struct timedomain_t{
    float samples[FFT_LEN];
};

struct freqdomain_t{
    float binDC;
    float binReal[FFT_LEN/2-1];
    float binNyquist;
    float binImag[FFT_LEN/2-1];
};

struct FFT_t{
    struct timedomain_t time;
    struct freqdomain_t freq;
    QList<QPointF> loglog;
};

enum type_e{Absolute, Level};


class FFTworker : public QObject
{
    Q_OBJECT
public:
    explicit FFTworker(QObject *parent = nullptr);

public slots:
    void calcFFT(FFT_t *F , int channel);
    void calcLogScale(FFT_t *F);

signals:
    void resultReady(int index);


private:
     ffft::FFTRealFixLen <FFT_POW> fft_object;
     QVector<int> f_table;
     const float fs=5E8f/26.0f/64.0f;

};

#endif // FFTWORKER_H

