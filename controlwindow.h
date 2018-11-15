#ifndef CONTROLWINDOW_H
#define CONTROLWINDOW_H


#include <QMainWindow>
#include <QWidget>
#include <QGroupBox>
#include "Control\\drv8320s.h"
#include "Control\\eqchannel.h"
#include "bodeplot.h"

class controlwindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit controlwindow(QWidget *parent = nullptr);
    ~controlwindow();
signals:

public slots:

private:
    QGridLayout* top_layout;
    QGroupBox* groupbox;
    DRV8320S* drv8320;
    EQChannel* torque;
    EQChannel* flux;
    bodeplot* bode;

};

#endif // CONTROL_H
