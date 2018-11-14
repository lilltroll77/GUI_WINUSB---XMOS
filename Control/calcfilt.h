#ifndef CALCFILT_H
#define CALCFILT_H

#include <QVector>
#include <complex>

#define PLOTSIZE 256
#define FMIN 10.0
#define DEFAULT_FC 100

enum filterType_t{PI , LP1, LP2 , HP1 , HP2 , LowShelf1 , LowShelf2 , HighShelf1, HighShelf2 , PeakingEQ , Notch , AllPass , BandPass , Mute};

//Must match XMOS code

typedef struct{
        int active;
        enum filterType_t type;
        float Fc;
        int link;
        float Q;
        float Gain;
        float MasterGain;
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
void calcFilt(EQ_section_t &EQ , double fs, double Bcoef[3] , double Acoef[2] );
void calc_freqz(double fmin , double fmax , int fs);
void freqz(double B[3] , double A[2] , std::complex<double> H[]);
void calc_PI(PI_section_t &pi ,  double fs, double Bcoef[2] , double Acoef );


#endif // CALCFILT_H
