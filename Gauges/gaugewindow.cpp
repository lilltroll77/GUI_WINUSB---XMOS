#include "gaugewindow.h"
#include "mainwindow.h"
#include <QDebug>


GaugeWindow::GaugeWindow(QWidget *parent) :
     QMainWindow(parent)
{
    setWindowTitle("Gauges");
    top_layout = new QGridLayout(this);
    top_box = new QGroupBox(this);

    currentGauge = new currentGague(this);
    tempGauge = new tempgauge(this);
    angleGauge = new QEgauge(this);
    tachometerGauge = new tachometer(this);

    top_layout->addWidget(currentGauge->widget , 1,1);
    top_layout->addWidget(tempGauge->widget , 2,1);
    top_layout->addWidget(tachometerGauge->widget , 1, 2);
    top_layout->addWidget(angleGauge->widget , 2 ,2);
    top_box->setLayout(top_layout);
    top_box->setTitle("Gauges");
    this->setCentralWidget(top_box );
    this->show();

    // ******** For testing purpose ***************

    struct I_t I ={1.0f , 0.0f};
    float T=0 , deg=0 , RPM=0;


/*
    while(1){
        wait(16);
        tempGauge->setTemp(T+20.0f);
        angleGauge->setAngle(deg);
        currentGauge->setCurrent(&I);
        tachometerGauge->setSpeed(RPM);

        I.peak = fmodf(I.peak+0.1f , 25.0f);
        I.ms = fmodf(I.ms+0.1f , 25.0f);
        T = fmodf(T+0.25f , 100.0f);
        RPM = fmodf(RPM+5 , 3000.0f);
        deg = fmodf(deg+ 1.0f , 360.0f);
        //readCOMdata();

    }
 */
}

void GaugeWindow::setShaftAngle(float angle){
    angleGauge->setAngle(angle);
}

void GaugeWindow::setcurrentGauge(struct I_t I[]){
    I_t Imax={0,0};
    float Ia=I[IA].peak;
    float Ib=I[IB].peak;
    float Ic=I[IC].peak;
    if(Ia > Ic)
        Imax.peak = Ia;
    else
        Imax.peak = Ic;
    if(Ib > Imax.peak)
        Imax.peak = Ib;

    Ia = sqrtf(I[IA].RMS);
    Ib = sqrtf(I[IB].RMS);
    Ic = sqrtf(I[IC].RMS);
    Imax.RMS = (Ia + Ib + Ic )/3; // Add RMS currents
    //qDebug()<< Imax.RMS;
    currentGauge->setCurrent(&Imax);
};

GaugeWindow::~GaugeWindow()
{
}
