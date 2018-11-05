#ifndef FFTWORKER_H
#define FFTWORKER_H

#include <QObject>
#include "ffft/FFTRealFixLen.h"
#define FFT_POW 18
#define FFT_LEN (1<<FFT_POW)

struct F_t{
    float binDC;
    float binReal[FFT_LEN/2-1];
    float binNyquist;
    float binImag[FFT_LEN/2-1];
};

struct f_t{
    float sample[FFT_LEN];
};

enum type_e{Absolute, Level};


class FFTworker : public QObject
{
    Q_OBJECT
public:
    explicit FFTworker(QObject *parent = nullptr);

public slots:
    void calcFFT(struct F_t* X, struct f_t* x , enum type_e type , int index);

signals:
    void resultReady(int index);


private:
     ffft::FFTRealFixLen <FFT_POW> fft_object;
     float result[FFT_LEN];

};

#endif // FFTWORKER_H

