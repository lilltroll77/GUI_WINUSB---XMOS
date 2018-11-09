#include "fftworker.h"
#include "float.h"
#include "mainwindow.h"
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

float interp1(float* x , float* y , float xi , float dx){
    float b0,b1,b2;
    b0=y[0];
    b1=(y[1]-y[0])/dx;
    b2=(((y[2]-y[1])/dx)-((y[1]-y[0])/dx))/(2*dx);
    return(b0+b1*(xi-x[0])+b2*(xi-x[0])*(xi-x[1]));
}

FFTworker::FFTworker(QObject *parent) : QObject(parent)
{
    const float points =512;
    const float df = fs/FFT_LEN;
    for(int i=0; i<scale_points ; i++)
        linscale[i] = i/(scale_points-1)*fs/2;

    float s=logf(fs);
    float dx= (log10f(fs/2) - log10f(fs/2)/points)/points;
    for(int i=0; i<points ; i++){
        logscale[i]= powf(10 , dx*float(i))-1;
        qDebug()<< i<< logscale[i];
    }
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
