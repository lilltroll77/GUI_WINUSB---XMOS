
#include <math.h>
#include <complex>
#include <QVector>
#include "calcfilt.h"
#include "global_defines.h"


/// TODO make a class of this!
std::complex<double> ejw[BODE_PLOTSIZE];
std::complex<double> jw;
std::complex<double> one(1,0);
std::complex<double> c;


void set_freqz(double f , int i)
{
   if(i>=BODE_PLOTSIZE)
       qFatal("Array out of bound in set_freqz");
   jw.imag(2* M_PI * f / (double)FS);
   ejw[i]=exp(jw); // Precalc e^jw and where w = 2*pi f/FS
}


void freqz(double B[3] , double A[2] , std::complex<double> H[BODE_PLOTSIZE]){
    for(int i=0 ; i<BODE_PLOTSIZE ; i++){
        c = ejw[i];
        H[i] =( B[0] + B[1]*c + B[2]*c*c )/ (one + A[0]*c + A[1]*c*c);
    }
}


void calc_PI(PI_section_t &pi , double Bcoef[3] , double Acoef[2] ){
    //Bilinear transformation of T(s) = (s + 2*pi*fc)/s
    double k =M_PI/2 * pi.Fc/FS;
    double a=pow(10, pi.Gain/20);
    Acoef[0] = -1;
    Acoef[1] = 0;
    Bcoef[0] = a*(k+1);
    Bcoef[1] = a*(k-1);
    Bcoef[2] = 0;

}


void calcFilt(EQ_section_t &EQ , double Bcoef[3] , double Acoef[2] ){
 /*
%Calculate parametric EQ coef
    %f0 is the filter frequency
    %Q is the Q value
    %GaindB is the gain in dB
    %fs is the sampling frequency
    %Filtertype is one of
    % LPF (Lowpass Filter)
    % HPF (High pass filter)
    % BPFs (Band pass filter constant skirt gain, peak gain = Q)
    % BPFp (constant 0 dB peak gain)
    % notch
    % APF AllPass filter
    % peakingEQ
    % lowShelf
    % highSelf
   */
    double a0,a1,a2,b0,b1,b2;
//Disable the filter with H=1 of f is out of bound
/*    if( EQ.Fc < 10 || EQ.Fc > FS *0.48  ){
        Bcoef[0]=1;
        Bcoef[1]=0;
        Bcoef[2]=0;
        Acoef[0]=0;
        Acoef[1]=0;
        return;
    }
    */

    double w0 = 2 * M_PI * EQ.Fc/FS;
    double alpha=sin(w0)/(2 * EQ.Q);
    double A = pow(10,EQ.Gain/40);
    double sqA = pow(10,EQ.Gain/20);
    double p; //prewarp factor
    double w;
    switch(EQ.type){
    case LP1:
            p = tan(0.5*w0);
            a0 = 1;
            a1 = (p-1)/(p+1);
            a2 = 0;
            b0 = sqA*p/(p+1);
            b1 = b0;
            b2 = 0;
            break;
    case LP2 :
            b0 = sqA * (1 - cos(w0))/2;
            b1 =  2*b0;
            b2 =  b0;
            a0 =   1 + alpha;
            a1 =  -2*cos(w0);
            a2 =   1 - alpha;
           break;
    case HP1 :
            p = tan(0.5*w0);
            a0 = 1;
            a1 = (p-1)/(p+1);
            a2 = 0;
            b0 = sqA*(a0-a1)/2;
            b1 = -b0;
            b2 = 0;
            break;
    case HP2 :
            b0 = sqA* (1 + cos(w0))/2;
            b1 =  -2*b0;
            b2 =   b0;
            a0 =   1 + alpha;
            a1 =  -2*cos(w0);
            a2 =   1 - alpha;
            break;
            /*
    case 'BPFS' BandPass
            b0 =   sin(w0)/2 ;
            b1 =   0 ;
            b2 =  -b0;
            a0 =   1 + alpha;
            a1 =  -2*cos(w0);
            a2 =   1 - alpha;*/
    case BandPass : //'BPFp'

            b0 = sqA*  alpha;
            b1 =   0;
            b2 = sqA* -alpha;
            a0 =   1 + alpha;
            a1 =  -2*cos(w0);
            a2 =   1 - alpha;
            break;

     case Notch :
            b0 =   1;
            b1 =  -2*cos(w0);
            b2 =   1;
            a0 =   1 + alpha;
            a1 =  -2*cos(w0);
            a2 =   1 - alpha;
            break;
     case AllPass:
            b0 =   1 - alpha;
            b1 =  -2*cos(w0);
            b2 =   1 + alpha;
            a0 =   1 + alpha;
            a1 =  -2*cos(w0);
            a2 =   1 - alpha;
            break;
     case PeakingEQ:
            b0 =   1 + alpha*A;
            b1 =  -2*cos(w0);
            b2 =   1 - alpha*A;
            a0 =   1 + alpha/A;
            a1 =  -2*cos(w0);
            a2 =   1 - alpha/A;
            break;
      case Lead:
            w = 2 * M_PI * EQ.Fc;
            b0 = 2*FS + w*sqA;
            b1 = w*sqA - 2*FS;
            b2 = 0;
            a0 = 2*FS + w;
            a1 = w - 2*FS;
            a2 = 0;
            break;
      case Lead2:
            b0 =    A*( (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha );
            b1 =  2*A*( (A-1) - (A+1)*cos(w0)                   );
            b2 =    A*( (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha );
            a0 =        (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha;
            a1 =   -2*( (A-1) + (A+1)*cos(w0)                   );
            a2 =        (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha;
            break;
      case Lag:
            w = 2 * M_PI * EQ.Fc;
            b0 = w - 2*sqA*FS;
            b1 = 2*sqA*FS +w;
            b2 = 0;
            a0 = 2*FS + w;
            a1 = w - 2*FS;
            a2 = 0;
            break;
      case Lag2:
            b0 =    A*( (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha );
            b1 = -2*A*( (A-1) + (A+1)*cos(w0)                   );
            b2 =    A*( (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha );
            a0 =        (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha;
            a1 =    2*( (A-1) - (A+1)*cos(w0)                   );
            a2 =        (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha;
        break;
    case Mute:
        b0=0;
        b1=0;
        b2=0;
        a0=1;
        a1=0;
        a2=0;
    }
    Acoef[0] = a1/a0;
    Acoef[1] = a2/a0;
    Bcoef[0] = b0/a0;
    Bcoef[1] = b1/a0;
    Bcoef[2] = b2/a0;
}



