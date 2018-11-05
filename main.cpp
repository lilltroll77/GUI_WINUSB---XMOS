#include <QApplication>
#include "mainwindow.h"
#include "data_struct.h"
#include "usbbulk.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow* w = new MainWindow();
    USBbulk usb(w);
    QObject::connect(w , &MainWindow::restart_stream , &usb , &USBbulk::restart_stream );
    w->show();
    usb.start();
    return a.exec();
}

