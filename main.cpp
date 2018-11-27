#include <QApplication>
#include "mainwindow.h"
#include "usbbulk.h"
#include "controlwindow.h"
#include <stdint.h>
#include "fifo.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    fifo* Fifo = new fifo;
    MainWindow* w = new MainWindow(Fifo);
    USBbulk usb(w , Fifo);
    controlwindow control(&usb);

    //QObject::connect(w , &MainWindow::restart_stream , &usb , &USBbulk::restart_stream );
    w->show();
    usb.start(QThread::TimeCriticalPriority);
    control.show();
    return a.exec();

}

