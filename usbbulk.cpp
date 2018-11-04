#include "usbbulk.h"
#include <QDebug>
#include <QtCore/qglobal.h>


USBbulk::USBbulk(MainWindow* w){
    connect(this, &USBbulk::dataAvailable , w , &MainWindow::update_data);

    //constructor
}

void USBbulk::run(){
    //This function must be called before calling any other libusb function.
    qDebug()<< "Init status:" << libusb_init(nullptr);
    //libusb_set_debug(NULL , 1);
    //Returns a list of USB devices currently attached to the system.
    qDebug()<< "Number of connected USB devices:" << libusb_get_device_list(nullptr, &list);
    libusb_device *XMOSdev = print_devs(list);

    if(XMOSdev != NULL){
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
      qDebug() << "XMOS USB device not found";
      libusb_free_device_list(list, 1);
    }
     libusb_free_device_list(list, 1);
     libusb_claim_interface(handle , 0);
     int speed = libusb_get_device_speed(XMOSdev);
     int pkgSize= libusb_get_max_packet_size(XMOSdev , XMOS_BULK_EP_IN);


    for(int buff=0; buff<BUFFERS ; buff++){
       In_transfer[buff]  = libusb_alloc_transfer(0);
       libusb_fill_bulk_transfer(       In_transfer[buff], handle, XMOS_BULK_EP_IN ,(unsigned char*) &mem[buff], sizeof(mem[buff]), &USBbulk::callback  , nullptr , 0);
       libusb_submit_transfer(          In_transfer[buff]);
    }
    struct tx_t stream={streamIN, true};
    libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &stream, sizeof stream, NULL , 0);

    while(!do_exit){
        //int completed;
        libusb_handle_events_completed(NULL , NULL);
        //if(!completed)
        emit dataAvailable((struct USBmem_t**) mem[block]);
        block = !block;
        //qDebug()<<'.';
    }
}


void USBbulk::callback(struct libusb_transfer *transfer){
    libusb_submit_transfer(transfer);
    //static struct USBmem_t* buffer = (struct USBmem_t*) transfer->buffer;
}


USBbulk::~USBbulk()
{
    stream.stream=false;
    libusb_bulk_transfer(handle , XMOS_BULK_EP_OUT ,(unsigned char*) &stream, sizeof stream, NULL , 0);
    for(int buff=0; buff < BUFFERS ; buff++)
        libusb_cancel_transfer(In_transfer[buff]);
    libusb_release_interface(handle , 0);
    if (handle)
        libusb_close( handle);
    libusb_exit(NULL);
}
