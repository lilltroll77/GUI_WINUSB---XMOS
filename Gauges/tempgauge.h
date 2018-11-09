#ifndef TEMPGAUGE_H
#define TEMPGAUGE_H
#include"qcgaugewidget.h"

#include <QWidget>

class tempgauge : public QWidget
{
    Q_OBJECT
public:
    explicit tempgauge(QWidget *parent = nullptr );
    QcGaugeWidget* widget;
signals:

public slots:
    void setTemp(float T);

private slots:
    float temp2percent(float T);

private:
    QcBackgroundItem *bkg;
    QcBackgroundItem *bkg_center;
    QcNeedleItem* needle;
    QcValuesItem* Values;
    QcDegreesItem* DegreeSubItem;
    QcDegreesItem* DegreeItem;
    QColor color[4]={Qt::green, Qt::darkGreen , Qt::yellow , Qt::red };
    float temp_colorregions[5]={ 20 , 50 , 85 , 100, 120 };
    QList<QPair<QColor,float>> BandColors;
    QcColorBand* ColorBand;
    QcLabelItem* label;
    QcLabelItem* temp_label;
    const QString label_str = "Temperature";
    int bkg_colorstate=0;

};

#endif // TEMPGAUGE_H
