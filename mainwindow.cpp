#include <QDebug>
#include <stdio.h>
#include <QMessageBox>
#include <QThread>
#include <QQueue>
#include <QLogValueAxis>
#include <QBoxLayout>
#include <QPen>
#include "data_struct.h"
#include "mainwindow.h"  /// !!! QMainWindow is a Qt class and is not mainwindow.h!!!
#include "ui_mainwindow.h"
#include "math.h"


MainWindow::MainWindow(fifo* fifo_ptr , QWidget *parent) :
    QMainWindow(parent)
    //,ui(new Ui::MainWindow)
{
    gaugeWindow = new GaugeWindow(this);
    qDebug()<<"enum" <<sizeof(plotMode_e);

    Fifo = fifo_ptr;
    menuBar = new QMenuBar(this);
    menuSettings = new QMenu("Plot settings" , this);
    menuHelp     = new QMenu("Help" , this);
    menuSignal   = new QMenu("Signal generator" , this);
    menuHelp->setDisabled(true);

    signalMapper = new QSignalMapper(this);

    for(int i=0; i<2 ; i++){
         checkbox[i][ABCOut]= new QCheckBox("ABC->" ,this);
         checkbox[i][ABCOut]->setToolTip("3phase output signal");
         checkbox[i][AlphaBetaOut]= new QCheckBox("ab->" ,this);
         checkbox[i][AlphaBetaOut]->setFont(QFont("Symbol"));
         checkbox[i][AlphaBetaOut]->setToolTip("Output Alpha-Beta");
         checkbox[i][CurrentIn]= new QCheckBox( "I" ,this);
         checkbox[i][CurrentIn]->setToolTip("Phase currents");
         checkbox[i][DiffCurrentIn]= new QCheckBox( "dI" ,this);
         checkbox[i][DiffCurrentIn]->setToolTip("Phase diff currents");
         checkbox[i][AlphaBetaIn]= new QCheckBox( "->ab" ,this);
         checkbox[i][AlphaBetaIn]->setFont(QFont("Symbol"));
         checkbox[i][AlphaBetaIn]->setToolTip("Alpha-Beta feedback");
         checkbox[i][DQIn]= new QCheckBox( "->DQ" ,this);
         checkbox[i][DQIn]->setToolTip("Direct-Quadrature feedback");
         choiceBox[i] = new QGroupBox(this);
         layoutBox[i] = new QVBoxLayout(this);
         for(int j=0; j<plotChoiceN ; j++){
             layoutBox[i]->addWidget(checkbox[i][j]);
             connect(checkbox[i][j] , SIGNAL(clicked()) , signalMapper , SLOT(map()));
             signalMapper->setMapping(checkbox[i][j], plotChoiceN*i+j);
         }
         choiceBox[i]->setLayout(layoutBox[i]);
    }
    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(slot_Checkbox(int)));


    QAction* plotTF = new QAction(this);
    QAction* plotSens = new QAction(this);
    QAction* ZoomIn = new QAction("Zoom in FFT" , this );
    QAction* ZoomOut = new QAction("Zoom out FFT" ,this);



    plotTF->setText("plot IN->OUT TF");
    plotTF->setToolTip("plot In->Out transfer function of the system");
    plotSens->setText("plot Sens.");
    plotSens->setToolTip("plot the sensitivity of the system");
    menuSettings->addAction(plotTF);
    menuSettings->addAction(plotSens);
    menuSettings->addAction(ZoomIn);
    menuSettings->addAction(ZoomOut);


    Signal[OFF] = new QAction("OFF" ,this);
    Signal[MLS18] = new QAction("MLS 2^18" ,this);
    Signal[RND] = new QAction("Random" ,this);
    Signal[SINE] = new QAction("Sine-wave" ,this);
    Signal[OCTAVE] = new QAction("Octave" ,this);

    for(int i=0; i< 5 ; i++){
        Signal[i]->setData(QVariant(i));
        menuSignal->addAction( Signal[i]);
        connect(Signal[i], SIGNAL(triggered()) , this , SLOT(slot_signal()));
    }

    menuBar->addMenu(menuSettings );
    menuBar->addMenu(menuSignal);
    menuBar->addMenu(menuHelp);


    connect(plotTF , SIGNAL(triggered()) , this , SLOT(slot_plotSensitivity()));
    connect(plotSens , SIGNAL(triggered()) , this , SLOT(slot_plotTransferFunction()));
    connect(ZoomIn  , SIGNAL(triggered()) , this , SLOT(slot_ZoomIn()));
    connect(ZoomOut , SIGNAL(triggered()) , this , SLOT(slot_ZoomOut()));


    calcMLS();

    for(int i=0; i<FFT_N ; i++){
        fft_thread[i] = new QThread();
        fft[i] = new FFTworker(MLScorrLevel , this);
        fft[i]->moveToThread(fft_thread[i]);
    }
    chart1 =   new QChart();
    chart2 =  new QChart();
    FFT_chart = new QChart();
    IView  =    new QChartView(chart1);
    PIView =    new QChartView(chart2);
    FFTView =   new QChartView(FFT_chart);
    axisX1 = new QValueAxis();
    axisY1 = new QValueAxis();
    axisX2 = new QValueAxis();
    axisY2 = new QValueAxis();

    chart1 ->setTitle(QString("XMOS captured sensor data @ %1 kHz").arg(1/dt , 0, 'f' , 2) );
    chart2->setTitle(QString("XMOS PI controller @ %1 kHz").arg(1/dt , 0, 'f' , 2) );
    //FFT_chart->setTitle(QString("FFT with size %1 of correlated MLS").arg(FFT_LEN));
    slot_plotTransferFunction();
    for(int i=0; i<6 ; i++){
        QPointF pnt;
        list[i].reserve(1024);
        for(int k=0; k < 1024 ; k++){
            pnt.setX(N_MAG*360.0*k/1024.0);
            pnt.setY((qreal) 0);
            list[i].append(pnt);
        }
        series[i].replace( list[i]);
        series[i].setUseOpenGL(useOpenGL);
    }
    //Only use 2 values for setpoint, at least for now
    series[6].append(0 , 0);
    series[6].append(360,0);
    series[7].append(0,0.5);
    series[7].append(360,0.5);
    series[6].setUseOpenGL(useOpenGL);
    series[7].setUseOpenGL(useOpenGL);

    for(int i=0; i<3 ; i++){
        chart1 ->addSeries( &series[i]);
    }
    for(int i=3 ; i< 6; i++){
        chart2 ->addSeries(&series[i]);
    }
    for (int i=0; i<2; i++){
     int mode = plotMode[i];
     checkbox[i][mode]->setChecked(true);
     slot_Checkbox(plotChoiceN*i + mode);
    }

    FFTseries[FFT_IA].setName("I phase A");
    FFTseries[FFT_IC].setName("I phase C");
    FFTseries[FFT_IA].setUseOpenGL(useOpenGL);
    FFTseries[FFT_IC].setUseOpenGL(useOpenGL);
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

    //I_chart ->createDefaultAxes();

    axisX1->setTitleText("Space vector angle [°]");
    axisX1->setRange(0 ,N_MAG* 360);
    axisY1->setRange(-1 , 1);
    axisX1->setTickCount(2*N_MAG+1);

    chart1-> addAxis(axisX1 , Qt::AlignBottom);
    chart1-> addAxis(axisY1 , Qt::AlignLeft);

    for(int i=0; i<3 ; i++){
        series[i].attachAxis(axisX1);
        series[i].attachAxis(axisY1);
    }

    axisX2->setTitleText("Space vector angle [°]");
    axisX2->setRange(0 ,N_MAG* 360);
    axisY2->setRange(-1 , 1);
    axisX2->setTickCount(2*N_MAG+1);

    chart2-> addAxis(axisX2 , Qt::AlignBottom);
    chart2-> addAxis(axisY2 , Qt::AlignLeft);

    for(int i=3; i<6 ; i++){
        series[i].attachAxis(axisX2);
        series[i].attachAxis(axisY2);
    }

    QLogValueAxis* axisX = new QLogValueAxis();
    axisYFFT = new QValueAxis();
    axisX->setLabelFormat("%.0f");
    axisYFFT->setLabelFormat("%.0f");
    axisX->setRange(2*fs/FFT_LEN , ceil(fs/2));
    axisYFFT->setRange(-80 , 40);
    axisYFFT->setTickCount(1+(40-(-80))/20); // 20dB
    axisX->setMinorTickCount(8);// 2:9 20:10:90

    axisYFFT->setMinorTickCount(3);
    axisX->setTitleText("Frequency [Hz]");
    axisYFFT->setTitleText("Level [dB]");
    axisYFFT->setTickType(QValueAxis::TickType::TicksFixed);
    FFT_chart->addAxis( axisX , Qt::AlignBottom);
    FFT_chart->addAxis( axisYFFT , Qt::AlignLeft);
    //(FFTseries[0].pen().setWidth(3);
    //FFTseries[1].pen().setWidth(3);
    FFTseries[0].attachAxis(axisX);
    FFTseries[0].attachAxis(axisYFFT);
    FFTseries[1].attachAxis(axisX);
    FFTseries[1].attachAxis(axisYFFT);

    //FFT_chart->createDefaultAxes();

     IView ->setRenderHint(QPainter::RenderHint::Antialiasing);
     PIView->setRenderHint(QPainter::RenderHint::Antialiasing);
     FFTView ->setRenderHint(QPainter::RenderHint::Antialiasing);

     layout = new QGridLayout(this);
     layout ->addWidget(IView , 0 , 0);
     layout ->addWidget(choiceBox[0], 0 , 1, Qt::AlignCenter);
     layout ->addWidget(PIView , 1 , 0);
     layout ->addWidget(choiceBox[1], 1 , 1 , Qt::AlignCenter);
     layout ->addWidget(FFTView, 2 , 0);

     QWidget *placeholderWidget = new QWidget();
     placeholderWidget->setLayout(layout);

     /*** STAUS BAR ***/

     statusbar = new QStatusBar;
     statusbar->setObjectName("statusbar");
     statusbar->showMessage(tr("Not connected"));
     setStatusBar(statusbar);

     //box = new QGroupBox("Plots" , this);
     //box->setLayout(layout);
     this ->setMenuBar(menuBar);
     this -> setCentralWidget(placeholderWidget);

     this -> setMinimumWidth(1024);
     this -> setMinimumHeight(768);
     for(int i=0; i<FFT_N ; i++){
        connect(fft[i] , &FFTworker::resultReady , this ,  &MainWindow::update_FFT );
        connect(this ,   &MainWindow::useXCorr , fft[i] , &FFTworker::useXCorr );
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
    int fi;
    int pos=0;
    for(int i=DECIMATE/2; i<128 ; i+=DECIMATE){
        fi = block->samples[i];
        angle[pos++] = fi;
    }
    gaugeWindow->setShaftAngle((360.0f/1024.0f)*(fi>>3));
}

unsigned MainWindow::parse_lowspeed(){
    union block_t* block = Fifo->read();
    float temp = block->lowSpeed.temp;
    fuseStatus(!(bool) block->lowSpeed.states); //XMOS code is inverted
    int load = 3*block->lowSpeed.DSPload;
    statusbar->showMessage(QString("DSP-core load=%1%").arg(load/10));
    gaugeWindow->setTemp(temp);
    return block->lowSpeed.index;
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

void MainWindow::parse(enum plots_e plot){
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
            qreal ia = sum*scale.Current;
            //qDebug() << ia;

            iA[i]=ia;


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
            qreal ia = iA[i];
            qreal ic = sum*scale.Current;
            qreal ib = -(ia + ic);
            int fi = angle[i]>>3;

            //power-variant Clarke transform
            /*X = (2*A – B – C)*(1/3); = 2*IA +( IA + IC )- IC = 3*IA/3;
            Y = (B – C)*(1/sqrt(3));
            Z = (A + B + C)*(sqrt(2)/3); =0  */
            qreal X = ia;
            qreal Y = (ib-ic)/sqrt(3);
            qreal theta = N_MAG*2.0*M_PI/8192.0*fi;
            qreal co = cos(theta);
            qreal si = sin(theta);
            //Park transform
            qreal D = co*X + si*Y;
            qreal Q = co*Y - si*X;
            for(int mode=0; mode<2 ; mode++){
                int o=3*mode;
                switch(plotMode[mode]){
                case CurrentIn:
                    list[o][fi].setY(ia);
                    list[o+1][fi].setY(ib);
                    list[o+2][fi].setY(ic);
                    break;
                case DiffCurrentIn: //Diff
                    list[o][fi].setY(ia-ib);
                    list[o+1][fi].setY(ib-ic);
                    list[o+2][fi].setY(ic-ia);
                    break;
                case AlphaBetaIn:
                    list[o+0][fi].setY(X);
                    list[o+1][fi].setY(Y);
                    break;
                case DQIn:
                    list[o+0][fi].setY(D);
                    list[o+1][fi].setY(Q);
                    break;
                default:
                    break;
                }
            }
            updatePhaseCurrent(ia , current[IA] ,  IA);
            updatePhaseCurrent(ib , current[IB] ,  IB);
            updatePhaseCurrent(ic, current[IC] ,  IC);

        }
        break;
    case Torque:
        break;
    case Flux:
       break;
    case U:
        blockU = block;
        break;
    case Angle:
        for(int mode=0; mode<2 ; mode++){
            int o=3*mode;
            switch(plotMode[mode]){
            case AlphaBetaOut:
                for(int i=0; i<(128/DECIMATE) ; i++){
                    int fi = angle[i]>>3;
                    qreal u = blockU->samples[i*DECIMATE];
                    qreal a = block->samples[i*DECIMATE];
                    qreal arg = a*(2.0/6.0*M_PI/(1<<10));
                    //inverse Park Transform
                    qreal Q=0;
                    qreal alpha = scale.U*(u*cos(arg) - Q*sin(arg));
                    qreal beta =  scale.U*(u*sin(arg) + Q*cos(arg));
                    list[o][fi].setY( alpha);
                    list[o+1][fi].setY(beta);
                }
                break;
            case ABCOut:
                for(int i=0; i<(128/DECIMATE) ; i++){
                    int fi = angle[i]>>3;
                    qreal u = blockU->samples[i*DECIMATE];
                    qreal a = block->samples[i*DECIMATE];
                    qreal arg = a*(2.0/6.0*M_PI/(1<<10));
                    qreal UA = scale.U*u*cos(arg);
                    qreal UB = scale.U*u*cos(arg - (2*M_PI/3));
                    qreal UC = scale.U*u*cos(arg + (2*M_PI/3));
                    list[o][fi].setY(UA);
                    list[o+1][fi].setY(UB);
                    list[o+2][fi].setY(UC);
                }
            }
        }
        break;
     default:
        break;
    }
}



void MainWindow::update_data(){
    while( Fifo->getSize() >= 8){
        Fifo->checkSize();
        /* XMOS side
         * struct hispeed_vector_t{
                    int QE[PKG_SIZE/4];
                    int IA[PKG_SIZE/4];
                    int IC[PKG_SIZE/4];

                    int Torque[PKG_SIZE/4];
                    int Flux[PKG_SIZE/4];
                    int U[PKG_SIZE/4];
                    int angle[PKG_SIZE/4];// U*exp(j*angle);
                };*/
/*1*/   unsigned block = parse_lowspeed();
/*2*/   parse_angle();
/*3*/   parse(IA ); //2
/*4*/   parse(IC );

/*5*/   //parse(Torque , scale.Torque , listIndex);
/*6*/   //listIndex = parse(Flux   , scale.Flux , listIndex);

/*7*/   Fifo->read();
/*8*/   Fifo->read();


/*7*/   parse(U);
/*8*/   parse(Angle);

        listIndex +=128/DECIMATE;

        if(listIndex == 128/DECIMATE*ABUFFERS){
            listIndex=0;
            for(int i=0; i<6; i++)
                series[i].replace(list[i]);
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
    chart1-> axisY()->setRange(-current , current);
    chart2-> axisY()->setRange(-current , current);
}

void MainWindow::slot_plotSensitivity(){
    slot_plotTransferFunction();
    //emit SignalSource(1);
    //FFT_chart->setTitle(QString("FFT with size %1").arg(FFT_LEN));

}

void MainWindow::slot_plotTransferFunction(){
    emit SignalSource(0);
    //FFT_chart->setTitle(QString("FFT with size %1 of correlated MLS. Plotting OUTPUT sensitivity").arg(FFT_LEN));
    FFT_chart->setTitle(QString("Blackman windowed FFT with size %1").arg(FFT_LEN));

}

void MainWindow::slot_ZoomIn(){
    axisYFFT->setRange(-6 , 6);
    axisYFFT->setTickCount(7); // 2dB
    axisYFFT->setMinorTickCount(1);
    qDebug() << "Zoom in";

}

void MainWindow::slot_ZoomOut(){
    axisYFFT->setRange(-80 , 40);
    axisYFFT->setTickCount(1+(40-(-80))/20); // 20dB
    axisYFFT->setMinorTickCount(1);
    qDebug() << "Zoom out";
 }

void MainWindow::slot_signal(){
    QAction* obj = (QAction*) this->sender();
    int index = obj->data().toInt();
    QPen penA = FFTseries[FFT_IA].pen();
    QPen penC = FFTseries[FFT_IC].pen();
    switch(index){
    case MLS18:
        penA.setWidth(2);
        penC.setWidth(2);
        emit useXCorr(true);
    break;
    default:
        penA.setWidth(1);
        penC.setWidth(1);
        emit useXCorr(false);
        break;
    }
    FFTseries[FFT_IA].setPen(penA);
    FFTseries[FFT_IC].setPen(penC);
    emit SignalGenerator(index);
    qDebug() << index;
}

void MainWindow::slot_Checkbox(int index){
    int i=index/plotChoiceN;
    int o=3*i;
    int j=index%plotChoiceN;
    plotMode[i]=j;
    qDebug()<< "Plotmode:" << i << j;
    for(int k=0; k<plotChoiceN ; k++){
        if(k!=j)
            checkbox[i][k]->setChecked(false);
        else
            switch(k){
            case ABCOut:
                series[o].setName("Phase A out");
                series[o+1].setName("Phase B out");
                series[o+2].setName("Phase C out");
                series[o+1].show();
                series[o+1].show();
                series[o+2].show();
                if(i)
                    axisY2->setTitleText("Voltage");
                else
                    axisY1->setTitleText("Voltage");
                break;
            case AlphaBetaOut:
                series[o].setName("Alpha out");
                series[o+1].setName("Beta out");
                series[o].show();
                series[o+1].show();
                series[o+2].hide();
                if(i)
                    axisY2->setTitleText("Transformed voltage");
                else
                    axisY1->setTitleText("Transformed voltage");
                break;
            case CurrentIn:
                series[o].setName("Phase A");
                series[o+1].setName("Phase B");
                series[o+2].setName("Phase C");
                series[o].show();
                series[o+1].show();
                series[o+2].show();
                if(i)
                    axisY2->setTitleText("Current [A]");
                else
                    axisY1->setTitleText("Current [A]");

                break;
            case DiffCurrentIn:
                series[o].setName("Phase A-B");
                series[o+1].setName("Phase B-C");
                series[o+2].setName("Phase C-A");
                series[o].show();
                series[o+1].show();
                series[o+2].show();
                if(i)
                    axisY2->setTitleText("Current [A]");
                else
                    axisY1->setTitleText("Current [A]");
                break;
            case AlphaBetaIn:
                series[o].setName("Alpha in");
                series[o+1].setName("Beta in");
                series[o].show();
                series[o+1].show();
                series[o+2].hide();
                if(i)
                    axisY2->setTitleText("Transformed current");
                else
                    axisY1->setTitleText("Transformed current");

                break;
            case DQIn:
                series[o].setName("Direct in");
                series[o+1].setName("Quadrature in");
                series[o].show();
                series[o+1].show();
                series[o+2].hide();
                if(i)
                    axisY2->setTitleText("Transformed I [A]");
                else
                    axisY1->setTitleText("Transformed I [A]");

                break;
              }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


