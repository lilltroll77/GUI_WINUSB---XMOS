#include <QDebug>
#include <stdio.h>
#include <QMessageBox>
#include <QThread>
#include <QQueue>
#include <QValueAxis>
#include <QLogValueAxis>
#include <QBoxLayout>
#include "data_struct.h"
#include "mainwindow.h"  /// !!! QMainWindow is a Qt class and is not mainwindow.h!!!
#include "ui_mainwindow.h"
#include "math.h"


MainWindow::MainWindow(QQueue<union block_t>* fifo_ptr , QWidget *parent) :
    QMainWindow(parent)
    //,ui(new Ui::MainWindow)
{
    gaugeWindow = new GaugeWindow(this);
    fifo = fifo_ptr;


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
    calcLogScale();
    FFTseries[0].append(freq);
    FFTseries[1].append(freq);
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



    QLogValueAxis* axisX = new QLogValueAxis();
    QValueAxis* axisY = new QValueAxis();
    axisX->setLabelFormat("%.0f");
    axisY->setLabelFormat("%.0f");
    axisX->setRange(floor(fs/FFT_LEN) , ceil(fs/2));
    axisY->setRange(-20.0 , 80.0);
    axisX->setMinorTickCount(8);// 2:9 20:10:90
    axisY->setTickCount(6); // 20dB

    axisY->setMinorTickCount(3);

    axisX->setTitleText("Frequency [Hz]");
    axisY->setTitleText("Level [dB]");


    axisY->setTickType(QValueAxis::TickType::TicksFixed);


    FFT_chart->addAxis( axisX , Qt::AlignBottom);
    FFT_chart->addAxis( axisY , Qt::AlignLeft);
    FFTseries[0].attachAxis(axisX);
    FFTseries[0].attachAxis(axisY);
    FFTseries[1].attachAxis(axisX);
    FFTseries[1].attachAxis(axisY);

    //FFT_chart->createDefaultAxes();

     //IView ->setRenderHint(QPainter:: HighQualityAntialiasing);
     //PIView->setRenderHint(QPainter:: HighQualityAntialiasing);
     FFTView ->setRenderHint(QPainter:: Antialiasing);

     layout = new QBoxLayout(QBoxLayout::TopToBottom , this);
     layout ->addWidget(IView);
     layout ->addWidget(PIView);
     layout ->addWidget(FFTView);

     QWidget *placeholderWidget = new QWidget();
     placeholderWidget->setLayout(layout);

     //box = new QGroupBox("Plots" , this);
     //box->setLayout(layout);
     this -> setCentralWidget(placeholderWidget);
     this -> setMinimumWidth(1024);
     this -> setMinimumHeight(768);
     for(int i=0; i<FFT_N ; i++){
        connect(fft[i] , &FFTworker::resultReady , this ,  &MainWindow::update_FFT );
        fft_thread[i]->start();
        }

}

void MainWindow::update_FFT(int index , enum type_e type){
    switch(type){
    case Absolute:
        break;
    case Level:
        for(int i=0; i<FFT_PLOT_POINTS ; i++){
            float dB = FFT[index].binReal[i];
            freq[i].setY(dB);
        }
        break;
    case LogLog:
        ///We cannot draw 2^18 lines in the plot for the FFT, instead a line is drawn from the bin with the minimum value to the bin with the highest value
        for(int i=v_LUT[0]; i<v_LUT[1]; i++)
            freq[i].setY(FFT[index].loglogMax[i]);
        int k=v_LUT[1];
        for(int i=k; i<freq.size()-1;){
            if(k>FFT_PLOT_POINTS)
                qFatal("!FFT_PLOT_POINTS must be increased!");
            freq[i++].setY(FFT[index].loglogMin[k]);
            freq[i++].setY(FFT[index].loglogMax[k++]);
        }
    }

  /*  QGraphicsScene* scene;
    QLineF line(10.0, 40.0, 1000.0, 40.0);
    scene = FFTView->scene();
    scene->addLine(line);
 */
    FFTseries[index].replace(freq);
    FFT_rd_buff = !FFT_rd_buff;

}

void MainWindow::reset_states(){
    for(int i=0; i < FFT_N ; i++)
        fft_pos[i]=0;
    FFT_wr_buff=0;
    FFT_rd_buff=0;
    updates=0;
    writeCopy=0;
    FFT_wr_buff=0;
    FFT_rd_buff=0;
}

float MainWindow::filter(qreal x , enum plots_e plot ){
    qreal y;
    const qreal B=1e-3;
    const qreal maxI=32;
    if(x>maxI)
        x=maxI;
    else if(x<-maxI)
        x=-maxI;
    y = B * (x + Xold[plot]) + Yold[plot]*(1-2*B);
    Xold[plot] = x;
    Yold[plot] = y;
    return (float)y;
}

void MainWindow::parse_angle(){
    union block_t block = fifo->dequeue();
    for(int i=0; i<128 ; i++)
        angle[angle_pos++] = block.samples[i]*scale.QE;
    angle_pos &=8191;
}

void::MainWindow::parse_lowspeed(){
    union block_t block = fifo->dequeue();
    float temp = block.lowSpeed.lowspeed.temp;
    //qDebug()<<temp;
    gaugeWindow->setTemp(temp);
}

void MainWindow::parse(enum plots_e plot , enum FFT_e fft_plot , int &index , bool parseFFT , qreal scale){
    if(fifo->size()==0)
        return;
    union block_t block = fifo->dequeue();
    int readPos = 0;
    if(parseFFT){
        for(int i=0; i<(128/DECIMATE) ; i++){
            int sum=0;
            for( int d=0; d<DECIMATE ; d++ ){
                qint32 val = block.samples[readPos++];
                sum += val;
                fft_data[fft_plot][FFT_wr_buff].sample[fft_pos[fft_plot]++] = val;
            }
            qreal scaled = sum*scale;
            list[plot][index++].setY(scaled);

            //peak hold
            current[plot].i = abs(scaled);
            if(current[plot].i >= current[plot].peak)
                current[plot].peak = current[plot].i;
            else
                current[plot].peak -=2*(dt/1000); //[A/s]
            current[plot].RMS = filter(scaled*scaled , plot);
        }
    }
    else{
        for(int i=0; i<(128/DECIMATE) ; i++){
            int sum=0;
            for( int d=0; d<DECIMATE ; d++ ){
                sum += block.samples[readPos++];
            }
            list[plot][index++].setY(sum*scale);
        }
    }
}


void MainWindow::update_data(){
   //qDebug("R");
    while(fifo->count() >= 8*ABUFFERS){
        for(int i=0; i<len ; i++)
            listIndex[i]=0;

        for(int j=0; j<ABUFFERS ; j++){
            parse_lowspeed();
            parse(IA , FFT_IA , listIndex[IA] , true , scale.Current);
            parse(IC , FFT_IC , listIndex[IC] , true , scale.Current);
            parse_angle();
            parse(Torque , OFF, listIndex[Torque] , false , scale.Torque);
            parse(Flux , OFF, listIndex[Flux] , false , scale.Flux);
            fifo->removeFirst();//U
            fifo->removeFirst();//ang
        }
        for(int i=0 ; i<(8192/DECIMATE) ; i++)
            list[IB][i].setY(-list[IA][i].y() - list[IC][i].y() );
        for(int i=0; i<(128/DECIMATE) ; i++){ // A already
            qreal Ib = list[IA][i].y() + list[IC][i].y();
            current[IB].RMS = filter(Ib*Ib , IB);
            float absIb= abs(Ib);
            if(absIb >  current[IB].peak)
                current[IB].peak = absIb;
            else
                current[IB].peak -=2*(dt/1000);
        }

        series[IA].replace(list[IA]);
        series[IB].replace(list[IB]);
        series[IC].replace(list[IC]);
        series[Torque].replace(list[Torque]);
        series[Flux].replace(list[Flux]);
        gaugeWindow->setcurrentGauge(current);
        float rpm = (angle[8191]-angle[0])* (60.0f/360.0f/dt/8.192);
        //qDebug()<<rpm;
        gaugeWindow->setShaftSpeed(rpm);
        if(fft_pos[0] >= FFT_LEN ){
            for(int i=0; i < FFT_N ; i++)
                fft_pos[i]=0;
            FFT_wr_buff=!FFT_wr_buff;
            for(int i=0; i< FFT_N ; i++)
                fft[i]->calcFFT(&FFT[i] , &fft_data[i][FFT_rd_buff] , LogLog , i , v_LUT);
        }
    }//while
}

void MainWindow::show_Warning(QString str){
    QMessageBox *msgbox = new QMessageBox(QMessageBox::Warning , "Warning" , str);
     msgbox->show();
}

void MainWindow::calcLogScale(){
    v_LUT.append(1);
    int sum=0;
    for(int i=0; v_LUT.last()<=fs ;i++){
        sum +=FFT_GROUPING<<i;
        v_LUT.append(sum);
    }
    freq.reserve(2*FFT_PLOT_POINTS);
    for(int v=0;; v++){
        qreal f1 , f2;
        int width = (1<<v); // 1 2 4 8 16 ...
        int start = v_LUT[v];
        int stop =  v_LUT[v+1];
        for(int i= start; i< stop ; i+=width){
            f1 = (qreal) i * (fs/FFT_LEN); // first freq
            f2 = (qreal)(i+width-1) * (fs/FFT_LEN);
            QPointF pnt(sqrt(f1*f2) , -100);
            freq.append(pnt);
            if(width>1){
                //pnt.setX(f2);
                freq.append(pnt);
            }
            //qDebug() << pnt;
            if(f2> fs/2){
                qDebug()<<"Plotting" << freq.size() << "lines in FFT";
                return;
            }
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


