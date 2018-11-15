#ifndef EQCHANNEL_H
#define EQCHANNEL_H

#include <QWidget>
#include <QVector>
#include "knob.h"
#include "calcfilt.h"
#include <complex.h>
#include "eqsection.h"
#include "PIsection.h"

class EQChannel : public QWidget
{
    Q_OBJECT
public:
    explicit EQChannel(QWidget *parent = nullptr , int new_channel=0 , QString title="");
    ~EQChannel();
    PISection *PI;
    EQsection *EQ[2];


signals:

public slots:

private:

QGridLayout *layout;
QVBoxLayout *top_layout;
QGroupBox *groupBox;
int channel;



};

#endif // EQCHANNEL_H

