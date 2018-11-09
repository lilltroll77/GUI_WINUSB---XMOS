#ifndef CURRENTGAGUE_H
#define CURRENTGAGUE_H

#include <QWidget>
#include "qcgaugewidget.h"

struct needle_t{
   QcLabelItem* label;
   QcNeedleItem* Item;
};

struct I_t{
    float peak;
    float ms; //Mean square
};

class currentGague : public QWidget
{
    Q_OBJECT
public:
    explicit currentGague(QWidget *parent = nullptr, float imax=25.0f);
    QcGaugeWidget* widget;
signals:

public slots:
    void setCurrent(struct I_t *I);
    void setScale(float I);


private:
    QcBackgroundItem *bkg;
    QcBackgroundItem *bkg_center;
    needle_t peak_needle;
    needle_t ms_needle;
    QcValuesItem* Values;
    QcDegreesItem* DegreeSubItem;
    QcDegreesItem* DegreeItem;
    int DegreeStart = 0;
    int DegreeEnd =   180;
    QPair<QColor,float> pair[4]={{Qt::green , 20},{Qt::darkGreen , 70},{Qt::yellow , 80},{Qt::red , 100}};
    QcColorBand* ColorBand;
    QcLabelItem* label;
    const QString label_str = "CoilCurrent [A]";
    float Imax;
    float scale_overcurrent=100.0f/pair[2].second; // How much larger is the scale compared to Imax;

};

#endif // CURRENTGAGUE_H
