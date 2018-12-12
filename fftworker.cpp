#include "fftworker.h"
#include "float.h"
//#include "mainwindow.h"
#include <QDebug>

float dB(float a , float b){
    const float offset=20*log10f(FFT_LEN);
    const float min=10*log10f(FLT_MIN);
    float prod = a*a+b*b;
    if(prod>0)
        return 10*log10f(a*a+b*b)-offset;
    else
        return
              min-offset;
}

FFTworker::FFTworker(float* Xcorr, QObject *parent) : QObject(parent)
{
    const qreal a0 = 7938.0/18608.0;
    const qreal a1 = 9240.0/18608.0;
    const qreal a2 = 1430.0/18608.0;
    mls_xcorr = Xcorr;
    for(int i=0; i<FFT_LEN ; i++)
        blackman[i] = a0 - a1*cos(2*M_PI*i/(FFT_LEN-1)) + a2*cos(4*M_PI*i/(FFT_LEN-1));
}

void FFTworker::useXCorr(bool state){
    use_xcorr = state;
}

void FFTworker::calcFFT(struct F_t* X ,struct f_t* x , type_e type, int index, QVector<int> &v_LUT) {
    for(int i=0; i<FFT_LEN ; i++)
        x->sample[i] *=blackman[i];
    fft_object.do_fft((float*) X , (float*)x);
    switch(type){
    case Absolute:
        emit resultReady(index, type);
        break;
    case Level:
        X->binReal[0] = dB(X->binReal[0] , 1);
        for(int i=1; i< FFT_PLOT_POINTS ; i++)
            X->binReal[i] = dB(X->binReal[i] , X->binImag[i]);
        emit resultReady(index , type);
        break;
    case LogLog:
        int f_log=0;
        int exit=0;

        for(int v=0;; v++){
            int width = 1<<v; // 1 2 4 8 16...
            int start = v_LUT[v];
            int stop =  v_LUT[v+1];
            for(int i = start; i < stop ; i+=width){
                float max=-1000 , min=1000 , level;
                for(int k=i; k< width+i ; k++){
                    if(k < FFT_LEN/2){
                        level=dB(X->binReal[k] , X->binImag[k]);
                        if(use_xcorr)
                            level-=mls_xcorr[k];
                        else
                            level-=mls_xcorr[0];

                        if( level > max)
                            max = level;
                        if( level < min)
                            min = level;
                    }
                    else
                        exit=1;
                }
                X->loglogMin[f_log] = min;
                X->loglogMax[f_log++] = max;
                if(exit){
                    emit resultReady(index , type);
                    return;
                }
            }
        }
    }
}
