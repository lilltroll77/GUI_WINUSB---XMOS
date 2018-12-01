#include <QDebug>
#include <QtCore/qglobal.h>
#include <QMessageBox>
#include "usbbulk.h"

static int last_actual_length;

/// ! Not QmainWindow!!
USBbulk::USBbulk(MainWindow* w , fifo *fifo_ptr){
    connect(this, &USBbulk::dataAvailable , w , &MainWindow::update_data);
    connect(this, &USBbulk::sendWarning   , w , &MainWindow::show_Warning);
    connect(w , &MainWindow::SignalSource , this , &USBbulk::sendSignalSource );
    Fifo= fifo_ptr;
}

void USBbulk::run(){
    //This function must be called before calling any other libusb function.
    qDebug()<< "Init status:" << libusb_init(nullptr);
    //libusb_set_debug(NULL , 1);
    //Returns a list of USB devices currently attached to the system.
    qDebug()<< "Number of connected USB devices:" << libusb_get_device_list(nullptr, &list);
    libusb_device *XMOSdev = print_devs(list , VID , PID);

    if(XMOSdev != nullptr){
        err = libusb_open(XMOSdev, &handle);
        switch(err){
        case 0:
            qDebug()<< "USB device open -> success";
            break;
        case LIBUSB_ERROR_NO_MEM:
            qDebug()<< "USB device open -> Memory allocation failure";
            break;
        case LIBUSB_ERROR_ACCESS:
            qDebug()<< "USB device open -> Insufficient permissions";
            break;
        case LIBUSB_ERROR_NO_DEVICE:
            qDebug()<< "USB device open -> Device has been disconnected";
            break;
        default:
            qDebug()<< "USB device open -> Failure";
           break;
        }
    }else{
      libusb_exit(NULL);
      emit sendWarning(QString("No XMOS USB device found with correct Vendor ID and Product ID\n"
                       "Check that your XMOS device has enumerated correctly and that you are running a compatible version of the XMOS code\n"
                       "This code version will only connect to VID = 0x%1 | PID = 0x%2\n").arg(VID,0,16).arg(PID,0,16));
      return;
    }
     libusb_free_device_list(list, 1);
     libusb_claim_interface(handle , 0);
     int speed = libusb_get_device_speed(XMOSdev);
     int pkgSize= libusb_get_max_packet_size(XMOSdev , XMOS_BULK_EP_IN);

    //stop_stream();
    //wait(100);
    Out_transfer = libusb_alloc_transfer(0);
    for(int buff=0; buff<BUFFERS ; buff++){
       In_transfer[buff]  = libusb_alloc_transfer(0);
       libusb_fill_bulk_transfer(       In_transfer[buff], handle, XMOS_BULK_EP_IN ,(unsigned char*) &mem[buff], sizeof(mem[buff]), &USBbulk::callback  , nullptr , 0);
       libusb_submit_transfer(          In_transfer[buff]);
    }
    start_stream();
    unsigned syncPnt=0;
    while(!do_exit){
        //int completed;
        int resync=0;
        libusb_handle_events(nullptr);
        if(last_actual_length == (8*ABUFFERS*PKG_SIZE)){
            union block_t* mem_block = (union block_t*) &mem[block];

            for(int j=0; j< (8*ABUFFERS) ; j++){
                if ( (j % 8)== syncPnt && mem_block->lowSpeed.checknumber != pi ){ // every 8 block should have the check number
                    syncPnt = (syncPnt+1)&7;
                    if(!resync){
                        resync=1;
                        qDebug() << "Resyncing";
                    }
                }
                else{
                    Fifo->write(mem_block);
                    if(resync){
                        resync=0;
                        qDebug() << "Synced";
                    }
                }
                mem_block++;
            }
            if( Fifo->getSize() >= 8){ //
                emit dataAvailable();
            }

            block = (block+1)%BUFFERS;
            //Set last_actual_length to zero, Only put the data on the FIFO once.
            last_actual_length=0;
        }
        else if(last_actual_length >0)
            qDebug() <<"Unexpected package length" <<last_actual_length;
    }
}

void USBbulk::restart_stream(){
    stop_stream();
    for(int buff=0; buff < BUFFERS ; buff++)
        libusb_cancel_transfer(In_transfer[buff]);
    wait(500);
    for(int buff=0; buff < BUFFERS ; buff++){
        libusb_submit_transfer(In_transfer[buff]);
    }
    block=0;
    libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &streamON, sizeof streamON, NULL , 0);
}

void USBbulk::empty_callback(struct libusb_transfer *transfer){
   // process = false;
    last_actual_length = -transfer->actual_length;
}

void USBbulk::callback(struct libusb_transfer *transfer){
    libusb_submit_transfer(transfer);
    last_actual_length = transfer->actual_length;
   // process = true;
    //static struct USBmem_t* buffer = (struct USBmem_t*) transfer->buffer;
}
void USBbulk::start_stream(){
    if(handle == nullptr){
        testHandleMsg();
        return;
    }
    libusb_fill_bulk_transfer(Out_transfer, handle, XMOS_BULK_EP_OUT , (unsigned char*) &streamON, sizeof streamON, &USBbulk::empty_callback   , nullptr , 0);
    libusb_submit_transfer(   Out_transfer);
    block=0;
}
void USBbulk::stop_stream(){
    if(handle == nullptr){
        testHandleMsg();
        return;
    }
     libusb_fill_bulk_transfer(Out_transfer, handle, XMOS_BULK_EP_OUT , (unsigned char*) &streamOFF, sizeof streamOFF, &USBbulk::empty_callback , nullptr, 0);
     libusb_submit_transfer(   Out_transfer);
}

void::USBbulk::sendFuseReset(){
    struct fuse_t{
        qint32 header;
        qint32 state;
    };
    struct fuse_t fuse = {FuseStatus , 1};
    libusb_fill_bulk_transfer( Out_transfer, handle, XMOS_BULK_EP_OUT ,(unsigned char*) &fuse, sizeof(fuse), &USBbulk::empty_callback  , nullptr , 0);
    libusb_submit_transfer(    Out_transfer);

}

void::USBbulk::sendFuseCurrent(float current){
    struct fuse_t{
        qint32 header;
        float current;
    };
    struct fuse_t fuse = {FuseCurrent , current};
    libusb_fill_bulk_transfer( Out_transfer, handle, XMOS_BULK_EP_OUT ,(unsigned char*) &fuse, sizeof(fuse), &USBbulk::empty_callback  , nullptr , 0);
    libusb_submit_transfer(    Out_transfer);
    //qDebug() << fuse.current;
}

void USBbulk::sendSignalSource(int source){
    quint32 data[2] = {SignalSource , (quint32) source};
    libusb_fill_bulk_transfer( Out_transfer, handle, XMOS_BULK_EP_OUT ,(unsigned char*) data, sizeof(data), &USBbulk::empty_callback  , nullptr , 0);
    libusb_submit_transfer(    Out_transfer);
}

void USBbulk::resetPIintegrator(int channel){
    quint32 data[2] = {resetPI , (quint32) channel};
    libusb_fill_bulk_transfer( Out_transfer, handle, XMOS_BULK_EP_OUT ,(unsigned char*) data, sizeof(data), &USBbulk::empty_callback  , nullptr , 0);
    libusb_submit_transfer(    Out_transfer);
    qDebug() << "reset PI ch"<<channel;
}

void USBbulk::sendPIsettings(PI_section_t &PIsection , int channel){
    struct USB_PIsection_t pi;
    pi.channel =channel;
    memcpy(&pi.section , &PIsection , sizeof(PI_section_t));
    if(handle == nullptr){
        testHandleMsg();
        return;
    }

    libusb_fill_bulk_transfer( Out_transfer, handle, XMOS_BULK_EP_OUT ,(unsigned char*) &pi, sizeof(pi), &USBbulk::empty_callback  , nullptr , 0);
    libusb_submit_transfer(    Out_transfer);

    //libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &pi, sizeof(pi), NULL , 0);
    //qDebug()<<"Sent PI settings with len=" <<sizeof(pi) << "bytes. Ch"<< channel <<"Fc="<<pi.section.Fc << "Gain=" << pi.section.Gain;
}

void USBbulk::sendEQsettings(EQ_section_t &EQ , int channel , int section ){
    struct USB_EQsection_t eq;
    eq.channel = channel;
    eq.section = section;
    memcpy(&eq.data , &EQ , 11*sizeof(qint32));
    //libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &eq, 14*sizeof(qint32), NULL , 0);
    libusb_fill_bulk_transfer( Out_transfer, handle, XMOS_BULK_EP_OUT ,(unsigned char*) &eq, 15*sizeof(qint32), &USBbulk::empty_callback  , nullptr , 0);
    libusb_submit_transfer(    Out_transfer);
}

void USBbulk::testHandleMsg(){
     qInfo("USB link has not been established!");
}

USBbulk::~USBbulk()
{
    stop_stream();
    libusb_release_interface(handle , 0);
    if (handle)
        libusb_close( handle);
    libusb_exit(NULL);
}
