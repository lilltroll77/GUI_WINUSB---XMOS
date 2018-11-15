#ifndef CALCFILT_H
#define CALCFILT_H

#include <QVector>
#include <complex>

#define FMIN 10.0
#define DEFAULT_FC 100

enum filterType_t{Lead , Lead2 , Lag , Lag2 , Notch , AllPass , /* DISABLED ->*/ LP1 , LP2 , HP1 , HP2 , BandPass , PeakingEQ, Mute};

//Must match XMOS code

typedef struct{
        int active=0;
        enum filterType_t type = Notch;
        float Fc=1000.0f;
        float Q=0.701f;
        float Gain=0;
    }EQ_section_t;

typedef struct{
    float Fc;
    float Gain;
}PI_section_t;


typedef struct{
    EQ_section_t section[4];
    unsigned delay;
}EQ_channel_t;

QVector<double>* f_ref();
void calcFilt(EQ_section_t &EQ , double Bcoef[3] , double Acoef[2] );
void set_freqz(double f , int i);
void freqz(double B[3] , double A[2] , std::complex<double> H[]);
void calc_PI(PI_section_t &pi , double Bcoef[3] , double Acoef[2] );


#endif // CALCFILT_H
