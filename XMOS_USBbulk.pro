#-------------------------------------------------
#
# Project created by QtCreator 2018-10-19T04:17:24
#
#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = XMOS_USBbulk
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        usb_helper_functions.cpp \
        usbbulk.cpp \
        fftworker.cpp \
        Gauges/currentgague.cpp \
        Gauges/gaugewindow.cpp \
        Gauges/qcgaugewidget.cpp \
        Gauges/qegauge.cpp \
        Gauges/tachometer.cpp \
        Gauges/tempgauge.cpp \
        Control/knob.cpp \
        Control/drv8320s.cpp \
        Control/eqsection.cpp \
        Control/calcfilt.cpp \
        Control/PIsection.cpp \
        Control/eqchannel.cpp \
        controlwindow.cpp \
        Control/bodeplot.cpp \
        transform.cpp \
        fifo.cpp


HEADERS += \
        mainwindow.h \
        libusb.h \
        usb_helper_functions.h \
        data_struct.h \
        usbbulk.h \
        ffft/FFTReal.h \
        ffft/FFTRealFixLen.h \
        fftworker.h \
        Gauges/currentgague.h \
        Gauges/gaugewindow.h \
        Gauges/qcgaugewidget.h \
        Gauges/qegauge.h \
        Gauges/tachometer.h \
        Gauges/tempgauge.h \
        Control/knob.h \
        Control/drv8320s.h \
        Control/eqsection.h \
        Control/calcfilt.h \
        Control/PIsection.h \
        Control/eqchannel.h \
        controlwindow.h \
        Control/bodeplot.h \
        transform.h \
        global_enums.h \
        global_defines.h \
        fifo.h


#FORMS += \
#        mainwindow.ui

LD


win32: LIBS += -L$$PWD/MinGW64/dll/ -llibusb-1.0.dll -lusb-1.0

INCLUDEPATH += $$PWD/MinGW64/dll $$PWD/Control
DEPENDPATH += $$PWD/MinGW64/dll

RESOURCES += \
    image.qrc
