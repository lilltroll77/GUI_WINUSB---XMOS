#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "libusb.h"
#include "usb_helper_functions.h"
#include "data_struct.h"
#include "QDebug"
#include <QTime>
#include <stdio.h>
#include <QChartView>
#include <QLineSeries>
QT_CHARTS_USE_NAMESPACE

#define SUPERBLOCKS 1
#define ABUFFERS 64
#define BUFFERS 1

struct tx_t{
    int header;
    int stream;
};
QTime t;
static int dt;

int do_exit = 0;
struct libusb_transfer* In_transfer[BUFFERS] = {nullptr};
static int packages=0;
static int i_transfer=0;
static int block_count=0;
static int error_count=0;
static FILE* fid;
struct USBmem_t mem[BUFFERS][ABUFFERS]={0};

void cb_in(struct libusb_transfer *transfer)
{

    qDebug() << "Length:" << transfer->actual_length;
    if(transfer->actual_length == sizeof(mem[0]) ){
        //libusb_submit_transfer(In_transfer[block_count]);
        //block_count = (block_count+1)%BUFFERS;
        /*qDebug() << mem[i_transfer].checknumber << mem[i_transfer].index;
        if(block_count !=mem[i_transfer][0].index)
            error_count++;
        block_count+=ABUFFERS;
        //fwrite(transfer->buffer , sizeof(char) , sizeof(mem[0]) , fid );*/
        packages++;
    }
    else
        qDebug() << "USB problems";

    //i_transfer = (i_transfer+1)%BUFFERS;


    if(packages>=BUFFERS){
        dt = t.elapsed();
        do_exit=1;
         libusb_handle_events_completed(NULL , NULL);

    }

}

enum message_e{streamIN};


void printError(int res){
    switch(res){
    case 0:
        return;
    case LIBUSB_ERROR_TIMEOUT:
        qDebug() << "LIBUSB_ERROR_TIMEOUT";
        break;
    case LIBUSB_ERROR_PIPE:
       qDebug() <<  "LIBUSB_ERROR_PIPE";
        break;
    case LIBUSB_ERROR_OVERFLOW:
        qDebug() <<"LIBUSB_ERROR_OVERFLOW";
        break;
    case LIBUSB_ERROR_NO_DEVICE:
        qDebug() <<"LIBUSB_ERROR_NO_DEVICE";
        break;
    case LIBUSB_ERROR_BUSY:
        qDebug() <<"LIBUSB_ERROR_BUSY";
        break;
    default:
        qDebug() <<"LIBUSB_ERROR";
        return;
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    libusb_device **list;
    int err = 0;

    //This function must be called before calling any other libusb function.
    qDebug()<< "Init status:" << libusb_init(nullptr);

    libusb_set_debug(NULL , 1);

    //Returns a list of USB devices currently attached to the system.
    qDebug()<< "Number of connected USB devices:" << libusb_get_device_list(nullptr, &list);

    libusb_device *XMOSdev = print_devs(list);

    struct libusb_device_handle *handle = NULL;


    if(XMOSdev != NULL){
        err = libusb_open(XMOSdev, &handle);
    }
    int speed = libusb_get_device_speed(XMOSdev);
    int pkgSize= libusb_get_max_packet_size(XMOSdev , XMOS_BULK_EP_IN);
    qDebug()<< "Max packet size=" << pkgSize;
    int* config;
    //libusb_get_configuration(XMOSdev , config);
    qDebug() << speed_name[speed];
    libusb_free_device_list(list, 1);
    libusb_claim_interface(handle , 0);


     tx_t stream={streamIN, true};

    //unsigned char* mem = libusb_dev_mem_alloc(handle , blkSize*ABUFFERS);
 /*
     fid = fopen("C:\\Users\\micke\\Documents\\XMOS_USBbulk\\test_stream.bin" , "w+");
    t.start();
    for(int i=0; i<SUPERBLOCKS; i++)
        fwrite( &mem[0] ,1, sizeof(mem[0]) , fid );
    dt = t.elapsed();
    fclose(fid);
    qDebug() <<"File write time:" << dt << "ms =" << 4*ABUFFERS*SUPERBLOCKS/dt << "Mbyte/s";

    fid = fopen("C:\\Users\\micke\\Documents\\XMOS_USBbulk\\test_stream.bin" , "w+");
*/
     for(int buff=0; buff<BUFFERS ; buff++){
        In_transfer[buff]  = libusb_alloc_transfer(0);
        libusb_fill_bulk_transfer(       In_transfer[buff], handle, XMOS_BULK_EP_IN ,(unsigned char*) &mem[buff], sizeof(mem[buff]),  &cb_in , NULL , 0);
        libusb_submit_transfer(          In_transfer[buff]);
    }


    qDebug()<< "Recieving" << BUFFERS*ABUFFERS* SUPERBLOCKS*4/1024 << "MBytes data asynchronous in USB BULK mode, synchronous with DATA flow";
    libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &stream, sizeof stream, NULL , 0);
    t.start();

    while(!do_exit)
        libusb_handle_events_completed(NULL, NULL);

    stream.stream=false;
    libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &stream, sizeof stream, NULL , 0);

    for(int buff=0; buff < BUFFERS ; buff++){
        libusb_cancel_transfer(In_transfer[buff]);
        //libusb_free_transfer(In_transfer[buff]);
    }
    libusb_release_interface(handle , 0);
    if (handle)
        libusb_close( handle);
    libusb_exit(NULL);



    qDebug() << "Block lost=" << error_count;
/*
    for(int j=0; j< BUFFERS ; j++)
        for(int i=0; i< ABUFFERS ; i++)
            qDebug()<<mem[j][i].index <<mem[j][i].slow.temp <<mem[j][i].mid.pos;
*/
    QLineSeries *seriesIA = new QLineSeries(this);
    QLineSeries *seriesIB = new QLineSeries(this);
    QLineSeries *seriesIC = new QLineSeries(this);
    QLineSeries *seriesQE = new QLineSeries(this);
    QLineSeries *seriesAngle = new QLineSeries(this);
    QList<QPointF> IA , IB , IC , QE , angle;
    IA.reserve(128*ABUFFERS);
    IB.reserve(128*ABUFFERS);
    IC.reserve(128*ABUFFERS);
    QE.reserve(128*ABUFFERS);
    angle.reserve(128*ABUFFERS);
    qreal dt= 26*64/5e5;
    int block=0;
    for(int block=0; block<BUFFERS; block++){
        for(int ablock=0; ablock<ABUFFERS; ablock++){
            for(int i=0; i<128 ; i++){
                QPointF point;
                point.setX(dt*(i+128*ablock));
                hispeed_vector_t* fast = &mem[block][ablock].fast;

                point.setY(fast->IA[i]);
                IA.append(point);

                point.setY(fast->IC[i]);
                IC.append(point);

                point.setY(-fast->IA[i] - fast->IC[i]);
                IB.append(point);

                point.setY(fast->QE[i]);
                QE.append(point);
                //qDebug() << mem[0][x].fast.IA[i] << mem[0][x].checknumber;

                point.setY(fast->angle[i]);
                angle.append(point);

            }
        }
    seriesIA->  append(IA);
    seriesIB->  append(IB);
    seriesIC->  append(IC);
    seriesQE->  append(QE);
   seriesAngle->append(angle);

    seriesIA->setName("I phase A");
    seriesIB->setName("I phase B");
    seriesIC->setName("I phase C");
    seriesQE->setName("QE");
 seriesAngle->setName("Angle Out");

    QChart *chart = new QChart();
    chart->addSeries(seriesIA);
    chart->addSeries(seriesIB);
    chart->addSeries(seriesIC);
    chart->addSeries(seriesQE);
    chart->addSeries(seriesAngle);

    chart->createDefaultAxes();
    QAbstractAxis* Xaxis =chart->axisX();
    QAbstractAxis* Yaxis =chart->axisY();
    Xaxis->setRange(0 , 30);
    Xaxis->setTitleText("Time [ms]");
    Yaxis->setRange(-10000 , 10000);


    chart->setTitle(QString("XMOS captured sensor data @ %1 kHz").arg(1/dt , 0, 'f' , 2) );

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter:: Antialiasing);
    this->setCentralWidget(chartView);
    /*
    rewind(fid );
    int b=0;

    qDebug()<<sizeof(mem);
    int bytes= fread((void*) mem , sizeof(char) , sizeof(mem) , fid);
    qDebug() << bytes << "bytes read";
    fclose(fid);
*/


    qDebug() << SUPERBLOCKS*BUFFERS*4096*ABUFFERS/dt <<"kByte/s with"  << 4096*ABUFFERS  << "bytes multi buffer on the host side for disk write";
     }
}


/*
  struct tx_t a={A,2.2f};
     struct tx_t b={B,3.3f};
     float p[2];

     struct group_t{
        struct tx_t a;
        struct tx_t b;
     };

     struct group_t block[BLOCKSIZE];
     for(int i=0; i<BLOCKSIZE  ; i++){
         block[i].a.data = (float)i*1.1;
         block[i].a.header = A;
         block[i].b.data = (float)i*2.2;
         block[i].b.header = B;
     }



     //libusb_context* ctx=NULL;

     int trans[4];
     if(err==0){
         int res = libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT , (unsigned char*) &a, sizeof (a), &trans[0] , 1200);
         printError(res);

         res = libusb_bulk_transfer(handle , XMOS_BULK_EP_IN ,  (unsigned char*) &p[0]  , sizeof (p[0]) , &trans[1] , 1200);
         printError(res);
         res = libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT , (unsigned char*) &b     , sizeof (b)    , &trans[2] , 1200);
         printError(res);
         res = libusb_bulk_transfer(handle , XMOS_BULK_EP_IN ,  (unsigned char*) &p[1]  , sizeof (p[1]) , &trans[3] , 1200);
         printError(res);
         //qDebug() << p[0] << p[1];
         //qDebug() << trans[0] << trans[1] << trans[2] << trans[3];

    }
    else
         qDebug() << "Could not open device";

     t.start();
     for(int i=0; i<2000; i++){
         libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT , (unsigned char*) &a, sizeof (a), &trans[0] , 1200);
         libusb_bulk_transfer(handle , XMOS_BULK_EP_IN ,  (unsigned char*) &p[0]  , sizeof (p[0]) , &trans[1] , 1200);

         libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT , (unsigned char*) &b, sizeof (b), &trans[0] , 1200);
         libusb_bulk_transfer(handle , XMOS_BULK_EP_IN ,  (unsigned char*) &p[1]  , sizeof (p) , &trans[3] , 1200);
     }
     dt = t.elapsed();
     qDebug() << dt/4<<"us roundtrip time with synchronous (blocking) API";


     int block_data[BLOCKSIZE]={0};
     qDebug() << "Starting data stream from XMOS in bulk mode";
     libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &stream, sizeof(stream), NULL , 1200);
     qDebug() << "Recieving" << BLOCKS << "USB bulk mode packages";
     t.start();
     for(int i=0; i<BLOCKS; i++)
        libusb_bulk_transfer(handle , XMOS_BULK_EP_IN ,  (unsigned char*) &block_data  , sizeof(block_data) , NULL , 1200);
     dt = t.elapsed();
     stream.data=false; //stop stream
     libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &stream, sizeof(stream), NULL , 1200);

     qDebug() << BLOCKS*sizeof(block_data)/dt <<"kByte/s with synchronous (blocking) API on the host";
     qDebug() << "***********************************";
     dt=0;

 struct DSPmem_t async_data[ABUFFERS];

    unsigned char ep[1]={XMOS_BULK_EP_IN };
    for(int i=0; i<ABUFFERS ; i++){
        In_transfer[i]  = libusb_alloc_transfer(i);
        libusb_fill_bulk_transfer(In_transfer[i], handle, XMOS_BULK_EP_IN ,(unsigned char*) &async_data[i] ,
                                  sizeof(async_data[i]) , cb_in , NULL , 1200);
        libusb_submit_transfer(In_transfer[i]);
    }

    //libusb_submit_transfer(In_transfer[1]);

    int blkSize=sizeof(struct DSPmem_t);
    qDebug()<< "Recieving" << BLOCKS* blkSize/1024/1024 << "MBytes data asynchronous in USB bulk mode";
    stream.data=true;
    t.start();
    libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &stream, sizeof stream, NULL , 1200);

    while(!do_exit)
     libusb_handle_events_completed(NULL, NULL);


    stream.data=false; //stop stream
    libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &stream, sizeof(stream), NULL , 1200);

    qDebug() << BLOCKS*blkSize/dt <<"kByte/s with"  << blkSize  << "bytes dual buffer on the host side";

   // for(int i=0; i<ABLOCKSIZE; i++)
   //     qDebug() <<i<<":"<< async_data[0][i];

    for(int i=0; i<ABUFFERS ; i++)
      libusb_free_transfer(In_transfer[i]);

    libusb_release_interface(handle , 0);
    libusb_close( handle);


/*    for(int i=-9; i<0; i++)
        qDebug() <<BLOCKS*BLOCKSIZE + i<<":"<<  block_data[i+BLOCKS*BLOCKSIZE];
*/

   /*  if(!libusb_open(XMOSdev , devh))
             qDebug() << "LIBUSB_ERROR";

*/
//Should be called after closing all open devices and before your application terminates.


     //ui->setupUi(this);


MainWindow::~MainWindow()
{
    delete ui;
}




/*
    cnt = libusb_get_device_list(nullptr, &devs);
    if (cnt < 0)
        return (int) cnt;

   // print_devs(devs);
    libusb_free_device_list(devs, 1);

    libusb_exit(nullptr);
*/

// qDebug() << cnt;

/* while(bytes > 0){
     if(bytes < blockSize)
         blockSize = bytes;
     //libusb_bulk_transfer(handle , XMOS_BULK_EP_IN , (unsigned char*) block_data, BLOCKSIZE, &bytes_read, 500);
     libusb_transfer()
     bytes -= bytes_read;
     qDebug()<< bytes;
     }
     dt = t.elapsed();


*/
