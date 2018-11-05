#include <QDebug>
#include <stdio.h>
#include <QMessageBox>
#include <QThread>
#include <QValueAxis>
#include "data_struct.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    freq.reserve(FFT_PLOT_POINTS);
    for(int i=0; i<FFT_N ; i++){
        fft_thread[i] = new QThread();
        fft[i] = new FFTworker();
        fft[i]->moveToThread(fft_thread[i]);
    }
    I_chart =   new QChart();
    PI_chart =  new QChart();
    FFT_chart = new QChart();
    IView  =    new QChartView(I_chart);
    PIView =    new QChartView(PI_chart);
    FFTView =   new QChartView(FFT_chart);

    I_chart ->setTitle(QString("XMOS captured sensor data @ %1 kHz").arg(1/dt , 0, 'f' , 2) );
    PI_chart->setTitle(QString("XMOS PI controller @ %1 kHz").arg(1/dt , 0, 'f' , 2) );
    FFT_chart->setTitle(QString("FFT with size %1").arg(FFT_LEN));
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

    FFTseries[FFT_IA].setName("I phase A");
    FFTseries[FFT_IC].setName("I phase C");
    QPen pen = FFTseries[FFT_IA].pen();
    pen.setWidth(1);
    pen.setColor(series[IA].color());
    FFTseries[FFT_IA].setPen(pen);
    pen.setColor(series[IC].color());
    FFTseries[FFT_IC].setPen(pen);
    qreal fs=1000/dt;
    for(int i=0; i< FFT_PLOT_POINTS ; i++){
        qreal f = i*(fs/FFT_LEN);
        FFTseries[0].append(f , 0);
        QPoint pnt;
        pnt.setX(f);
        freq.append(pnt);
        FFTseries[1].append(f, 0);
    }
    FFT_chart->addSeries(&FFTseries[0]);
    FFT_chart->addSeries(&FFTseries[1]);

    I_chart ->createDefaultAxes();
    I_chart-> axisX()->setTitleText("Shaft angle Deg°");
    I_chart-> axisY()->setTitleText("Current [A]");
    I_chart-> axisX()->setRange(0 , 360);
    I_chart-> axisY()->setRange(-2.0 , 2.0);

    PI_chart->createDefaultAxes();
    PI_chart-> axisX()->setTitleText("Shaft angle Deg°");
    PI_chart-> axisY()->setRange(-0.5 , 1.5);
    PI_chart-> axisX()->setRange(0 , 360);



    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisY = new QValueAxis();
    axisX->setLabelFormat("%.0f");
    axisY->setLabelFormat("%.0f");
    axisX->setRange(0 , round(FFT_PLOT_POINTS*(fs/FFT_LEN)));
    axisY->setRange(-40.0 , 80.0);
    axisX->setTitleText("Frequency [Hz]");
    axisY->setTitleText("Level [dB]");

    //axisX->setTickInterval(50.0);
    axisX->setMinorTickCount(3);
    axisX->setTickCount(10);
    axisX->applyNiceNumbers();
    axisY->setTickCount(7);
    axisY->setMinorTickCount(3);
    axisY->setTickType(QValueAxis::TickType::TicksFixed);


    FFT_chart->addAxis( axisX , Qt::AlignBottom);
    FFT_chart->addAxis( axisY , Qt::AlignLeft);
    FFTseries[0].attachAxis(axisX);
    FFTseries[0].attachAxis(axisY);
    FFTseries[1].attachAxis(axisX);
    FFTseries[1].attachAxis(axisY);

    //FFT_chart->createDefaultAxes();

     IView ->setRenderHint(QPainter:: Antialiasing);
     PIView->setRenderHint(QPainter:: Antialiasing);
     FFTView ->setRenderHint(QPainter:: Antialiasing);

     layout = new QBoxLayout(QBoxLayout::TopToBottom , this);
     layout ->addWidget(IView);
     layout ->addWidget(PIView);
     layout ->addWidget(FFTView);

     box = new QGroupBox("Plots" , this);
     box->setLayout(layout);
     this -> setCentralWidget(box);
     this -> setMinimumWidth(1024);
     this -> setMinimumHeight(768);
     for(int i=0; i<FFT_N ; i++){
        connect(fft[i] , &FFTworker::resultReady , this ,  &MainWindow::update_FFT );
        fft_thread[i]->start();
        }
}

void MainWindow::update_FFT(int index){
     for(int i=0; i<FFT_PLOT_POINTS ; i++){
         float dB = FFT[index].binReal[i];
        freq[i].setY(dB);
    }
    FFTseries[index].replace(freq);
   FFT_rd_buff = !FFT_rd_buff;
}

void MainWindow::reset_states(){
    fft_pos=0;
    FFT_wr_buff=0;
    FFT_rd_buff=0;
    updates=0;
    writeCopy=0;
    fft_pos=0;
    FFT_wr_buff=0;
    FFT_rd_buff=0;
}

void MainWindow::update_data(struct USBmem_t** mem){
    memcpy(copy , mem , sizeof(copy)); //Faster with local cache copy ?
        int listIndex=0;
        for(int j=0; j<ABUFFERS ; j++){
            //unsigned check = reinterpret_cast<struct USBmem_t*>((char*) mem + sizeof(struct USBmem_t)*j )->checknumber;
            //struct hispeed_vector_t* fast = &reinterpret_cast<struct USBmem_t*>((char*) mem + sizeof(struct USBmem_t)*j )->fast;
            //struct hispeed_vector_t* fast = &copy[j].fast;
            if(copy[j].checknumber != 3141592543){
                qDebug()<< "Packet check error, restarting stream";
                emit restart_stream();
                reset_states();
            }
            else{
                int readPos = 0;
                for(int i=0; i<(128/DECIMATE) ; i++){
                    int IAsum=0 , ICsum=0 , FluxSum=0 , TorqueSum=0;
                    for( int d=0; d<DECIMATE ; d++ ){
                        qint32 ia = copy[j].fast.IA[readPos];
                        IAsum += ia;
                        fft_data[FFT_IA][FFT_wr_buff].sample[fft_pos] = ia;
                        qint32 ic = copy[j].fast.IC[readPos];
                        ICsum += ic;
                        fft_data[FFT_IC][FFT_wr_buff].sample[fft_pos] = ic;
                        FluxSum += copy[j].fast.Flux[readPos];
                        TorqueSum += copy[j].fast.Torque[readPos];
                        readPos++;
                        fft_pos++;
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

        if(fft_pos >= FFT_LEN ){
            fft_pos=0;
            FFT_wr_buff=!FFT_wr_buff;
            for(int i=0; i< FFT_N ; i++)
                fft[i]->calcFFT(&FFT[i] , &fft_data[i][FFT_rd_buff] , Level , i);
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


