#ifndef CALCFILT_H
#define CALCFILT_H

#include <QVector>
#include <complex>
#include "global_defines.h"

#define FMIN 10.0
#define DEFAULT_FC 100

enum filterType_t{Lead , Lead2 , Lag , Lag2 , Notch , AllPass , /* DISABLED ->*/ LP1 , LP2 , HP1 , HP2 , BandPass , PeakingEQ, Mute};

//Must match XMOS code

typedef struct{
        quint32 active=0;
        enum filterType_t type = Notch;
        float Fc=1000.0f;
        float Q=0.701f;
        float Gain=0;
        quint32 B0f;
        quint32 B1f;
        quint32 B2f;
        quint32 A1f;
        quint32 A2f;
        quint32 align;
        double B0;
        double B1;
        double B2;
        double A1;
        double A2;
    }EQ_section_t;

typedef struct{
    float Fc;
    float Gain;
    quint32 B0f;
    quint32 B1f;
    quint32 B2f;
    quint32 A1f;
    quint32 A2f;
}PI_section_t;


typedef struct{
    EQ_section_t section[4];
    unsigned delay;
}EQ_channel_t;

QVector<double>* f_ref();
void calcFilt(EQ_section_t &EQ );
void set_freqz(double f , int i);
void freqz(double B[3] , double A[2], float fc , std::complex<double> H[BODE_PLOTSIZE]);
void calc_PI(PI_section_t &pi , double Bcoef[3] , double Acoef[2] );


#endif // CALCFILT_H
