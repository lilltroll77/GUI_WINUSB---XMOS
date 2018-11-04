#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include "usbbulk.h"
#include <QThread>
#include <QTimer>
#include <QTime>
#include <QCoreApplication>
#include <QtCore/qglobal.h>
#include <QObject>
#include "data_struct.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); 
    MainWindow* w = new MainWindow();
    USBbulk usb(w);
    w->show();
    usb.start();
    return a.exec();
}

