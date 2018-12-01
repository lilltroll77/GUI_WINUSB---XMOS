#ifndef CONTROLWINDOW_H
#define CONTROLWINDOW_H


#include <QMainWindow>
#include <QWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QIcon>
#include "Control\\drv8320s.h"
#include "Control\\eqchannel.h"
#include "bodeplot.h"
#include "usbbulk.h"
#include "knob.h"


class controlwindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit controlwindow(USBbulk *usb=nullptr, MainWindow *w= nullptr, QWidget *parent = nullptr );
    ~controlwindow();
signals:
    void resetFuse();

public slots:
    void slot_ResetButtonState(bool state);

private:
    QGridLayout* top_layout;
    QGroupBox* groupbox;
    DRV8320S* drv8320;
    EQChannel* torque;
    EQChannel* flux;
    bodeplot* bode;
    Knob* knob_fuse;
    QPushButton* button_reset;
    QIcon icon_fuseOK;
    QIcon icon_fuseBurnt;
    bool fusestate=true;

};

#endif // CONTROL_H
