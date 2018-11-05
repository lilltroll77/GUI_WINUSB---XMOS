#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "data_struct.h"
#include "QDebug"
#include <QTime>
#include <stdio.h>
#include "usbbulk.h"
#include <QMessageBox>


float dB(float a , float b){
    const float offset=20*log10f(FFT_LEN);
    return 10*log10f(a*a+b*b)-offset;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    I_chart =   new QChart();
    PI_chart =  new QChart();
    FFT_chart = new QChart();
    IView  =    new QChartView(I_chart);
    PIView =    new QChartView(PI_chart);
    FFTView =   new QChartView(FFT_chart);

    I_chart ->setTitle(QString("XMOS captured sensor data @ %1 kHz").arg(1/dt , 0, 'f' , 2) );
    PI_chart->setTitle(QString("XMOS PI controller @ %1 kHz").arg(1/dt , 0, 'f' , 2) );
    FFT_chart->setTitle(QString("FFTtitle"));
    for(int i=IA; i<=Torque ; i++){
        QPointF pnt;
        list[i].reserve(128/DECIMATE*ABUFFERS);
        //Simple low pass filtering
        for(int k=0; k<128/DECIMATE*ABUFFERS ; k++){
            pnt.setX(DECIMATE * (qreal)k * scale.QE);
            pnt.setY((qreal) 0);
            list[i].append(pnt);
        }
        series[i].replace( list[i]);
    }
    //Only use 2 values for setpoint, at least for now
    series[ SetFlux].append(0 , 0);
    series[ SetFlux].append(360,0);
    series[ SetTorque].append(0,0.5);
    series[ SetTorque].append(360,0.5);

    for(int i=IA; i<=IC ; i++){
        series[i].setName(Namestr[i]);
        I_chart ->addSeries( &series[i]);
    }
    for(int i=Flux ; i< len; i++){
        series[i].setName(Namestr[i]);
        PI_chart ->addSeries(&series[i]);
    }

    FFTseries.setName("FFT");
    for(int i=0; i< 256 ; i++)
        FFTseries.append(i,0);
    FFT_chart->addSeries(&FFTseries);

    I_chart ->createDefaultAxes();
    I_chart-> axisX()->setTitleText("Shaft angle Deg°");
    I_chart-> axisY()->setTitleText("Current [A]");
    I_chart-> axisX()->setRange(0 , 360);
    I_chart-> axisY()->setRange(-2.0 , 2.0);

    PI_chart->createDefaultAxes();
    PI_chart-> axisX()->setTitleText("Shaft angle Deg°");
    PI_chart-> axisY()->setRange(-0.5 , 1.5);
    PI_chart-> axisX()->setRange(0 , 360);

    FFT_chart->createDefaultAxes();
    FFT_chart->axisY()->setRange(-25 , 75);
    FFT_chart->axisY()->setTitleText("Level [dB]");

     IView ->setRenderHint(QPainter:: Antialiasing);
     PIView->setRenderHint(QPainter:: Antialiasing);
     //FFT_View->setRenderHint(QPainter::OpaqueHint);

     layout = new QBoxLayout(QBoxLayout::TopToBottom , this);
     layout ->addWidget(IView);
     layout ->addWidget(PIView);
     layout ->addWidget(FFTView);

     box = new QGroupBox("Plots" , this);
     box->setLayout(layout);
     this -> setCentralWidget(box);
     this -> setMinimumWidth(1024);
     this -> setMinimumHeight(768);
}

void MainWindow::update_data(struct USBmem_t** mem){
    struct USBmem_t copy[ABUFFERS];
    memcpy(copy , mem , sizeof(copy)); ///Needed for thread memory safty??
        int listIndex=0;
        for(int j=0; j<ABUFFERS ; j++){
            //unsigned check = reinterpret_cast<struct USBmem_t*>((char*) mem + sizeof(struct USBmem_t)*j )->checknumber;
            //struct hispeed_vector_t* fast = &reinterpret_cast<struct USBmem_t*>((char*) mem + sizeof(struct USBmem_t)*j )->fast;
            //struct hispeed_vector_t* fast = &copy[j].fast;
            if(copy[j].checknumber != 3141592543)
                qDebug()<< "Packet check error";
            else{
                int readPos = 0;
                for(int i=0; i<(128/DECIMATE) ; i++){
                    int IAsum=0 , ICsum=0 , FluxSum=0 , TorqueSum=0;
                    for( int d=0; d<DECIMATE ; d++ ){
                        IAsum +=copy[j].fast.IA[readPos];
                        x[writeX] = copy[j].fast.IA[readPos];
                        ICsum +=copy[j].fast.IC[readPos];
                        FluxSum += copy[j].fast.Flux[readPos];
                        TorqueSum += copy[j].fast.Torque[readPos];
                        readPos++;
                        writeX++;
                    }
                    list[IA][listIndex].setY(IAsum*scale.Current);
                    list[IC][listIndex].setY(ICsum*scale.Current);
                    list[IB][listIndex].setY( - list[IA][listIndex].y() - list[IC][listIndex].y()  );
                    list[Flux][listIndex].setY(FluxSum*scale.Flux);
                    list[Torque][listIndex].setY(TorqueSum*scale.Torque);
                    listIndex++;
                }
            }

        }
        series[IA].replace(list[IA]);
        series[IB].replace(list[IB]);
        series[IC].replace(list[IC]);
        series[Torque].replace(list[Torque]);
        series[Flux].replace(list[Flux]);

        if(writeX >= FFT_LEN ){
            writeX=0;

            F_t f;

            fft_object.do_fft((float*)&f, x);
            FFTseries.replace(0 , 0 , dB(f.DC , 1) );
            for(int i=1; i< 256 ; i++)
                FFTseries.replace(i , i , dB(f.Re[i] , f.Im[i]) );
        }

}

void MainWindow::show_Warning(QString str){
    QMessageBox *msgbox = new QMessageBox(QMessageBox::Warning , "Warning" , str);
     msgbox->show();
}


MainWindow::~MainWindow()
{
    delete ui;
}


