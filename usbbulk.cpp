#include <QDebug>
#include <QtCore/qglobal.h>
#include <QMessageBox>
#include "usbbulk.h"

static int last_actual_length;

USBbulk::USBbulk(MainWindow* w , QQueue<union block_t>* fifo_ptr){
    connect(this, &USBbulk::dataAvailable , w , &MainWindow::update_data);
    connect(this, &USBbulk::sendWarning  , w , &MainWindow::show_Warning);
    fifo= fifo_ptr;
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

    stop_stream();
    wait(100);
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
        libusb_handle_events_completed(NULL , NULL);
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
                fifo->enqueue(*mem_block);
                if(resync){
                    resync=0;
                    qDebug() << "Synced";
                }
            }
        mem_block++;
        }
        while(fifo->count() >= (48*ABUFFERS)){
            for(int i=0; i<(8*ABUFFERS) ;i++)
                fifo->removeFirst();
            qDebug() << "Removing" << 8*ABUFFERS*sizeof(union block_t) << "bytes in FIFO to prevent overfill";
        }

        if( fifo->count() >= (8*ABUFFERS)){ // Full data struct
            emit dataAvailable();
        }

        block = !block;
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

void USBbulk::callback(struct libusb_transfer *transfer){
    libusb_submit_transfer(transfer);
    last_actual_length = transfer->actual_length;
    //static struct USBmem_t* buffer = (struct USBmem_t*) transfer->buffer;
}
void USBbulk::start_stream(){
    libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &streamON, sizeof streamON, NULL , 0);
    block=0;
}
void USBbulk::stop_stream(){
    libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &streamOFF, sizeof streamOFF, NULL , 0);
}

USBbulk::~USBbulk()
{
    stop_stream();
    libusb_release_interface(handle , 0);
    if (handle)
        libusb_close( handle);
    libusb_exit(NULL);
}
