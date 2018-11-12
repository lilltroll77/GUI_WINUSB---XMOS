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
FFTworker::FFTworker(QObject *parent, int ch) : QObject(parent)
{
    channel = ch;
    freq.reserve(v_LUT.at(1)*v_LUT.size());
    for(int v=0; v < v_LUT.size()-1 ; v++){
        qreal f1 , f2;
        int width = (1<<v); // 1 2 4 8 16 ...
        int start = v_LUT.at(v);
        int stop =  v_LUT.at(v+1);
        for(int i= start; i< stop ; i+=width){
                f1 = (qreal) i * (fs/FFT_LEN); // first freq
                f2 = (qreal)(i+width-1) * (fs/FFT_LEN);
            qreal f = sqrt(f1*f2); // Calculate mean pos in log scale
            QPointF pnt(f , 0);
            freq.append(pnt);
        }
    }
}

void FFTworker::rewind(){
    fft_data->i=0;
}

bool FFTworker::bufferFull(){
    if(fft_data->i>=FFT_LEN)
        return true;
    return false;
}

void FFTworker::calcFFT(QQueue<QVector<float>>* fft_queue) {
    QVector<float> x = fft_queue->dequeue();
    fft_object.do_fft((float*) &FFT_Data, x.data());
    int f_log=0;
     for(int v=0; v < v_LUT.size()-1 ; v++){
        float max=-1000 , level;
        int width = 1<<v; // 1 2 4 8 16...
        int start = v_LUT.at(v);
        int stop =  v_LUT.at(v+1);
        for(int i = start; i < stop ; i+=width){
            for(int k=i; k< width+i ; k++){
                if(k >= FFT_LEN/2-1){
                   emit resultReady(&freq , channel);
                   return;
                }
                level=dB(FFT_Data.binReal[k] , FFT_Data.binImag[k]);
                if( level > max)
                    max = level;
            }
            if(f_log == freq.size())
                qCritical("f_log == freq.size() in FFTworker::calcFFT");
            freq[f_log++].setY(max);
        }
    }
}
