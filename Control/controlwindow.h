#ifndef CONTROLWINDOW_H
#define CONTROLWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QGroupBox>
#include "knob.h"
#include "drv8320s.h"

class controlwindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit controlwindow(QWidget *parent = nullptr);

signals:

public slots:

private:
    QGridLayout* top_layout;
    QGroupBox* box;
    DRV8320S* drv8320;

};

#endif // CONTROL_H
