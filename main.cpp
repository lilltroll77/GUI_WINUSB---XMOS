#include <QApplication>
#include "mainwindow.h"
#include "data_struct.h"
#include "usbbulk.h"
#include "controlwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QQueue<union block_t>* fifo=new QQueue<union block_t>;
    fifo->reserve(64*ABUFFERS);
    MainWindow* w = new MainWindow(fifo);
    USBbulk usb(w , fifo);
    controlwindow control;




    //QObject::connect(w , &MainWindow::restart_stream , &usb , &USBbulk::restart_stream );
    w->show();
    usb.start(QThread::TimeCriticalPriority);
    control.show();
    return a.exec();
}

