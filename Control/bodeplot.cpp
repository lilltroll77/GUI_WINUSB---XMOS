#include <complex>
#include <math.h>
#include "bodeplot.h"
#include "transform.h"
#include "calcfilt.h"
#include "QDebug"



bodeplot::bodeplot(QWidget *parent) : QWidget(parent)
{

    for(int ch=0; ch<2 ; ch++)
     for(int sec=0; sec<3 ; sec++)
      for(int i=0; i<BODE_PLOTSIZE; i++)
        H[ch][sec][i]= std::complex<double>(1,0);
    chart = new QChart();
    chartview = new QChartView(chart);
    freq.reserve(BODE_PLOTSIZE);
    for(int i=0; i<BODE_PLOTSIZE ; i++){
         double f = lin2log(i , BODE_PLOTSIZE, 150, 0.001);
         //qDebug()<<f;
         bodeFlux.append(f , 0);
         bodeTorque.append(f , 0);
         set_freqz(1000*f , i);
    }
    freqAxis.setMax(150);
    freqAxis.setMin(0.001);
    freqAxis.setMinorTickCount(8);
    freqAxis.setTitleText("Frequency [kHz]");
    freqAxis.setLabelFormat("%G");

    levelAxis.setMin(-50);
    levelAxis.setMax(50);
    levelAxis.setTickCount(11);
    levelAxis.setMinorTickCount(1);
    levelAxis.setTitleText("Level [dB]");

    bodeTorque.setName("Torque");
    bodeFlux.setName("Magnetic Flux");

    chart->addSeries(&bodeTorque);
    chart->addSeries(&bodeFlux);

    chart->addAxis(&freqAxis , Qt::AlignBottom);
    chart->addAxis(&levelAxis , Qt::AlignLeft);
    bodeTorque.attachAxis(&freqAxis);
    bodeTorque.attachAxis(&levelAxis);
    bodeFlux.attachAxis(&freqAxis);
    bodeFlux.attachAxis(&levelAxis);
    chartview->setChart(chart);
    chartview->setMinimumWidth(350);
    chartview->setRenderHint(QPainter:: Antialiasing);
    top_layout.addWidget(chartview);

    this->setLayout(&top_layout);

}

void bodeplot::calcLevel(int channel){
    for(int i=0; i<BODE_PLOTSIZE ; i++){
        std::complex<double> c =  H[channel][0][i]  * H[channel][1][i] * H[channel][2][i];
        level[i] = 10*log10( (double)norm(c) );
    }
}

void bodeplot::PIchanged(double B[3] , double A[2] , int channel){
    //qDebug() << "PI" << channel;
    freqz(B , A , 0 , H[channel][0]);
    calcLevel(channel);
    if(channel == 0){
        for(int i=0; i < BODE_PLOTSIZE ; i++)
            bodeTorque.replace(i, bodeTorque.at(i).x(), level[i]);
    }
    else{
        for(int i=0; i < BODE_PLOTSIZE ; i++)
            bodeFlux.replace(i, bodeFlux.at(i).x(), level[i]);
    }
}

void bodeplot::EQchanged(double B[3] , double A[2], float fc , int channel , int section){
    //qDebug() << "EQ" << channel << section;
    freqz(B,A, fc, H[channel][section]);
    calcLevel(channel);
    if(channel == 0){
        for(int i=0; i < BODE_PLOTSIZE ; i++)
            bodeTorque.replace(i, bodeTorque.at(i).x(), level[i]);
    }
    else{
        for(int i=0; i < BODE_PLOTSIZE ; i++)
            bodeFlux.replace(i, bodeFlux.at(i).x(), level[i]);
    }
}
