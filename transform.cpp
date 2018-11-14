#include "transform.h"


//Translates from log scale to linscale
int log2lin(double arg, int max, double maxLog, double minLog){
    double value = log10(arg/minLog)/log10(maxLog/minLog)*(double)(max);
    return (int)round(value);
}

double lin2log(int val,int max, double maxLog, double minLog){
    //y=0;min*10^(y/99*log10(max/min))
    double scale = log10(maxLog/minLog)/max;
    return minLog*pow(10, (double)val * scale);
}
