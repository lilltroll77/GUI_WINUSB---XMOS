#ifndef USBBULK_H
#define USBBULK_H
#include <QObject>
#include <QThread>
#include <QMainwindow>
#include "data_struct.h"
#include "libusb.h"
#include "usb_helper_functions.h"
#include "global_defines.h"
#include "mainwindow.h"
#include "calcfilt.h"
#include "fifo.h"

class USBbulk : public QThread {
    Q_OBJECT
public:
    USBbulk(MainWindow* w, fifo* Fifo);
    ~USBbulk();
    enum message_e{streamIN, PIsection , EQsection , resetPI , resetEQsec , resetEQ , FuseCurrent , NewFuse , FuseStatus , SignalSource , SignalGenerator,
                  DRV_VDS , DRV_ODT , DRV_TDRIVE , DRV_IDRIVE_P_HS , DRV_IDRIVE_N_HS , DRV_IDRIVE_P_LS , DRV_IDRIVE_N_LS , DRV_RESET};


signals:
    void dataAvailable();
    void sendWarning(QString str);

public slots:
     void restart_stream(void);
     void start_stream(void);
     void stop_stream(void);
     void sendPIsettings(PI_section_t &PIsection , int channel);
     void sendEQsettings(EQ_section_t &EQ , int channel , int section );
     void resetPIintegrator(int channel);
     void sendFuseCurrent(float current);
     void sendFuseReset();
     void sendSignalSource(int source);
     void sendSignalGenerator(int index);
     void send_DRV8320S(int command , int index);



private:
    //enum COMMAND{LINK_DOWN , LINK_UP , COM_CURRENT , COM_STOP, COM_DRV , COM_DRV_ERROR , COM_NEWTEMP , COM_FUSE , COM_RESET , COM_VDS , COM_ODT , COM_TDRIVE , COM_IDRIVE_P_HS , COM_IDRIVE_N_HS , COM_IDRIVE_P_LS , COM_IDRIVE_N_LS , COM_SET_TORQUE , COM_SET_FLUX , COM_PI_FUSE , COM_PI_TORQUE_FREQ , COM_PI_TORQUE_GAIN , COM_PI_FLUX_FREQ , COM_PI_FLUX_GAIN};

    struct USB_PIsection_t{
        int header = USBbulk::PIsection;
        int channel;
        PI_section_t section;
    };

    struct USB_EQsection_t{
        qint32 header = USBbulk::EQsection;
        quint32 channel;
        quint32 section;
        quint32 _padding;
        EQ_section_t data;
    };

    struct tx_t{
        int header;
        int stream;
    };
    void run();
    static void callback(struct libusb_transfer *transfer);
    static void empty_callback(struct libusb_transfer *transfer);
    void testHandleMsg();
    libusb_device **list;
    struct libusb_transfer* In_transfer[BUFFERS] = {nullptr};
    struct libusb_transfer* Out_transfer={nullptr};
    int err = 0;
    struct USBmem_t mem[BUFFERS][ABUFFERS]={0};
    struct libusb_device_handle *handle = NULL;
    int block=0;
    unsigned packets=0;
    int do_exit=0;
    tx_t streamON= {USBbulk::streamIN, true};
    tx_t streamOFF={USBbulk::streamIN, false};
    MainWindow* mainWindow;
    const int VID=0x20b1;
    const int PID=0x00da;
    //struct USBmem_t* buffer;
    const quint64 pi=3141592543358979324ull;
    const quint32 codeVersion=2;
    fifo* Fifo;


};

#endif // USBBULK_H
