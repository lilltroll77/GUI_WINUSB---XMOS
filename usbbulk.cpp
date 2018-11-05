#include <QDebug>
#include <QtCore/qglobal.h>
#include <QMessageBox>
#include "usbbulk.h"

USBbulk::USBbulk(MainWindow* w){
    connect(this, &USBbulk::dataAvailable , w , &MainWindow::update_data);
    connect(this, &USBbulk::sendWarning  , w , &MainWindow::show_Warning);
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
    for(int buff=0; buff<BUFFERS ; buff++){
       In_transfer[buff]  = libusb_alloc_transfer(0);
       libusb_fill_bulk_transfer(       In_transfer[buff], handle, XMOS_BULK_EP_IN ,(unsigned char*) &mem[buff], sizeof(mem[buff]), &USBbulk::callback  , nullptr , 0);
       libusb_submit_transfer(          In_transfer[buff]);
    }
    start_stream();

    while(!do_exit){
        //int completed;
        libusb_handle_events_completed(NULL , NULL);
        //if(!completed)
        emit dataAvailable((struct USBmem_t**) mem[block]);
        block = !block;
        //qDebug()<<'.';
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
