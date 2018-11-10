#include "fftworker.h"
#include "float.h"
#include "mainwindow.h"
#include <QDebug>

float imag2mag(struct freqdomain_t* freq, int i){
    if(i==0)
        return freq->binDC;
    else{
        float a=freq->binReal[i];
        float b=freq->binImag[i];
        return sqrtf(a*a+b*b);
    }
}

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

float interp1(float x0 , float* y , float xi , float dx){
    float b0,b1,b2;
    float x1 = x0+dx;
    b0=y[0];
    b1=(y[1]-y[0])/dx;
    b2=(((y[2]-y[1])/dx)-((y[1]-y[0])/dx))/(2*dx);
    return(b0+b1*(xi-x0)+b2*(xi-x0)*(xi-x1));
}



FFTworker::FFTworker(QObject *parent) : QObject(parent)
{

}

void FFTworker::calcLogScale(struct FFT_t *F){
    F->loglog.reserve(LOG_LEN);
    if(f_table.isEmpty())
        f_table.reserve(LOG_LEN);
    const float df = fs/FFT_LEN;
    float s=logf(fs);
    float dx= (log10f(fs/2) - log10f(fs/2)/LOG_LEN)/LOG_LEN;
    for(int i=0; i<LOG_LEN ; i++){
        float f =  powf(10 , dx*float(i))-1;
        F->loglog[i].setX(f);
        F->loglog[i].setY(0);
        int k=0;
        while(i*df < f)
            i++;
        i -=2;
        if(i<0)
            i=0;
        f_table[i]=k;
    }
}

void FFTworker::calcFFT(struct FFT_t* F , int channel) {
    fft_object.do_fft((float*) &F->freq , F->time.samples);
    float y[3];
    const float df = fs/FFT_LEN;
    for(int j=0; j<LOG_LEN; j++){
        int linfreq_i = f_table[j];
        y[0] = imag2mag(&F->freq , linfreq_i);
        y[1] = imag2mag(&F->freq , linfreq_i+1);
        y[2] = imag2mag(&F->freq , linfreq_i+2);
        float x = df*linfreq_i;
        float f_log = F->loglog[j].x();
        float ylog = interp1(x , y , f_log , df);
        F->loglog[j].setY(20*log10f(ylog));
    }
    emit resultReady(channel);
}
