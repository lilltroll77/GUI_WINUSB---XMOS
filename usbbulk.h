#ifndef USBBULK_H
#define USBBULK_H
#include "data_struct.h"
#include "libusb.h"
#include "usb_helper_functions.h"
#include "mainwindow.h"
#include <QObject>

#include <QThread>

#define ABUFFERS 64
#define BUFFERS 2

enum message_e{streamIN};

struct tx_t{
    int header;
    int stream;
};

class USBbulk : public QThread {
    Q_OBJECT
public:
    USBbulk(MainWindow* w);
    ~USBbulk();

signals:
    void dataAvailable(struct USBmem_t **usb);
    void sendWarning(QString str);

private:
    void run();
    static void callback(struct libusb_transfer *transfer);
    libusb_device **list;
    struct libusb_transfer* In_transfer[BUFFERS] = {nullptr};
    int err = 0;
    struct USBmem_t mem[BUFFERS][ABUFFERS]={0};
    struct libusb_device_handle *handle = NULL;
    int block=0;
    unsigned packets=0;
    int do_exit=0;
    tx_t stream={streamIN, true};
    MainWindow* mainWindow;
    const int VID=0x20b1;
    const int PID=0x00da;
    //struct USBmem_t* buffer;

};

#endif // USBBULK_H
