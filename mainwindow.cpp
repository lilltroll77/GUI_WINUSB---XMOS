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


MainWindow::MainWindow(fifo* fifo_ptr , QWidget *parent) :
    QMainWindow(parent)
    //,ui(new Ui::MainWindow)
{
    gaugeWindow = new GaugeWindow(this);
    Fifo = fifo_ptr;
    menuBar = new QMenuBar(this);
    menuSettings = new QMenu("Plot settings" , this);
    menuHelp     = new QMenu("Help" , this);
    menuHelp->setDisabled(true);
    QAction* plotTF = new QAction(this);
    QAction* plotSens = new QAction(this);
    plotTF->setText("plot IN->OUT TF");
    plotTF->setToolTip("plot In->Out transfer function of the system");
    plotSens->setText("plot Sens.");
    plotSens->setToolTip("plot the sensitivity of the system");
    menuSettings->addAction(plotTF);
    menuSettings->addAction(plotSens);
    menuBar->addMenu(menuSettings );
    menuBar->addMenu(menuHelp);


    connect(plotTF , SIGNAL(triggered()) , this , SLOT(slot_plotSensitivity()));
    connect(plotSens , SIGNAL(triggered()) , this , SLOT(slot_plotTransferFunction()));



    calcMLS();

    for(int i=0; i<FFT_N ; i++){
        fft_thread[i] = new QThread();
        fft[i] = new FFTworker(MLScorrLevel , this);
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
    //FFT_chart->setTitle(QString("FFT with size %1 of correlated MLS").arg(FFT_LEN));
    slot_plotTransferFunction();
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
    I_chart-> axisY()->setRange(-5 , 5);

    PI_chart->createDefaultAxes();
    PI_chart-> axisX()->setTitleText("Shaft angle Deg°");
    PI_chart-> axisY()->setRange(-5 , 5);
    PI_chart-> axisX()->setRange(0 , 360);



    QLogValueAxis* axisX = new QLogValueAxis();
    QValueAxis* axisY = new QValueAxis();
    axisX->setLabelFormat("%.0f");
    axisY->setLabelFormat("%.0f");
    axisX->setRange(2*fs/FFT_LEN , ceil(fs/2));
    axisY->setRange(-80 , 20);
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
     this ->setMenuBar(menuBar);
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
    fftIndexA=0;
    fftIndexC=0;
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
    union block_t* block = Fifo->read();
    for(int i=0; i<128 ; i++)
        angle[angle_pos++] = block->samples[i]*scale.QE;
    angle_pos &=8191;
}

unsigned MainWindow::parse_lowspeed(){
    union block_t* block = Fifo->read();
    float temp = block->lowSpeed.temp;
    fuseStatus(!(bool) block->lowSpeed.states); //XMOS code is inverted

    if(expectedIndex != block->lowSpeed.index){
        qDebug()<< expectedIndex << block->lowSpeed.index << "Diff=" << expectedIndex - block->lowSpeed.index;
        expectedIndex = block->lowSpeed.index;
    }
    expectedIndex++;
    //qDebug()<<temp;
    gaugeWindow->setTemp(temp);
    return block->lowSpeed.index & (FFT_LEN/128-1);
}

void MainWindow::updatePhaseCurrent(qreal i , struct I_t &current ,  enum plots_e plot){
    if(i >MAX_CURRRENT)
        i = MAX_CURRRENT;
    else if(i < - MAX_CURRRENT)
        i = -MAX_CURRRENT;
    current.RMS = filter(i*i, plot);
    qreal absI=abs(i);
    if(absI >  current.peak)
        current.peak = absI;
    else
        current.peak -=20*(dt/1000);

}

int MainWindow::parse(enum plots_e plot , qreal scale, int index){
    union block_t* block = Fifo->read();
    int readPos = 0;
    switch(plot){
    case IA:
        for(int i=0; i<(128/DECIMATE) ; i++){
            int sum=0;
            for( int d=0; d<DECIMATE ; d++ ){
                qint32 val = block->samples[readPos++];
                sum += val;
                fft_data[FFT_IA][FFT_wr_buff].sample[fftIndexA++] = val;
            }
            qreal ia = sum*scale;
            list[IA][index++].setY(ia);
            updatePhaseCurrent(ia , current[IA] ,  IA);

        }
        break;
   case IC:
        for(int i=0; i<(128/DECIMATE) ; i++){
            int sum=0;
            for( int d=0; d<DECIMATE ; d++ ){
                qint32 val = block->samples[readPos++];
                sum += val;
                fft_data[FFT_IC][FFT_wr_buff].sample[fftIndexC++] = val;
            }
            qreal ic = sum*scale;
            qreal ib = -(list[IA][index].y() + ic);
            list[IC][index].setY(ic);
            list[IB][index++].setY(ib);
            updatePhaseCurrent(ib , current[IB] ,  IB);
            updatePhaseCurrent(ic, current[IC] ,  IC);

        }
        break;
    case Torque:
    case Flux:
        for(int i=0; i<(128/DECIMATE) ; i++){
            int sum=0;
            for( int d=0; d<DECIMATE ; d++ )
                sum += block->samples[readPos++];
            list[plot][index++].setY(sum*scale);
        }
        break;
     default:
        break;
    }
    return index;
}



void MainWindow::update_data(){
    while( Fifo->getSize() >= 8){
        Fifo->checkSize();
/*1*/   unsigned block = parse_lowspeed();
/*2*/   parse(IA , scale.Current , listIndex); //2
/*3*/   parse(IC , scale.Current , listIndex);
/*4*/   parse_angle();
/*5*/   parse(Torque , scale.Torque , listIndex);
/*6*/   listIndex = parse(Flux   , scale.Flux , listIndex);
/*7*/   Fifo->read();
/*8*/   Fifo->read();
        if(listIndex == 128/DECIMATE*ABUFFERS){
            listIndex=0;
            series[IA].replace(list[IA]);
            series[IB].replace(list[IB]);
            series[IC].replace(list[IC]);
            series[Torque].replace(list[Torque]);
            series[Flux].replace(list[Flux]);
            gaugeWindow->setcurrentGauge(current);
            float rpm = (angle[8191]-angle[0])* (60.0f/360.0f/dt/8.192);
            gaugeWindow->setShaftSpeed(rpm);
        }
        if( (block==0) | (fftIndexA>= FFT_LEN)){
            fftIndexA = 0;
            fftIndexC = 0;
            if(block ==0){
                fft[FFT_IA] -> calcFFT(&FFT[FFT_IA] , &fft_data[FFT_IA][FFT_rd_buff] , LogLog , FFT_IA , v_LUT);
                fft[FFT_IC] -> calcFFT(&FFT[FFT_IC] , &fft_data[FFT_IC][FFT_rd_buff] , LogLog , FFT_IC , v_LUT);
                FFT_wr_buff = !FFT_wr_buff;
            }
            // else data out of sync.
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

void MainWindow::calcMLS(){
//
    //calculate MLS seq.
    quint32 lfsr=1 ,lsb;
    for(int i=0; i<FFT_LEN-1 ; i++){
        lsb = lfsr & 1;            /* Get LSB (i.e., the output bit). */
        lfsr >>= 1;                /* Shift register */
        if (lsb)                   /* If the output bit is 1, apply toggle mask. */
            lfsr ^= 0x20400;
        mls[i]= lsb;
    }
    mls[FFT_LEN-1]= 0;
    fft_object.do_fft((float*)&MLS , mls);
    float offset = dB(MLS.binReal[0] , MLS.binImag[0])-48;
    for(int i=0; i<FFT_LEN/2-1 ; i++)
        MLScorrLevel[i] = dB(MLS.binReal[i] , MLS.binImag[i])-offset;
}

void MainWindow::currentRange(double current){
    I_chart-> axisY()->setRange(-current , current);
}

void MainWindow::slot_plotSensitivity(){
    emit SignalSource(1);
    FFT_chart->setTitle(QString("FFT with size %1 of correlated MLS. Plotting INPUT -> OUTPUT").arg(FFT_LEN));

}

void MainWindow::slot_plotTransferFunction(){
    emit SignalSource(0);
    FFT_chart->setTitle(QString("FFT with size %1 of correlated MLS. Plotting OUTPUT sensitivity").arg(FFT_LEN));

}

MainWindow::~MainWindow()
{
    delete ui;
}


