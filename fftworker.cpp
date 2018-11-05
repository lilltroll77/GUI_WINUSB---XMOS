#include "fftworker.h"
#include "float.h"
#include "mainwindow.h"

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

FFTworker::FFTworker(QObject *parent) : QObject(parent)
{

}

void FFTworker::calcFFT(struct F_t* X ,struct f_t* x , type_e type, int index) {
       fft_object.do_fft((float*) X , (float*)x);
       if(type == Absolute)
        emit resultReady(index);
       else{
           X->binReal[0] = dB(X->binReal[0] , 1);
           for(int i=1; i< FFT_PLOT_POINTS ; i++)
               X->binReal[i] = dB(X->binReal[i] , X->binImag[i]);
           emit resultReady(index);
        }
}
