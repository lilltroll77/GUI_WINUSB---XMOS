#ifndef TACHOMETER_H
#define TACHOMETER_H

#include <QWidget>
#include "qcgaugewidget.h"

class tachometer : public QWidget
{
    Q_OBJECT
public:
    explicit tachometer(QWidget *parent = nullptr);
    QcGaugeWidget* widget;

signals:

public slots:
    void setSpeed(float RPM);

private:
    QcBackgroundItem *bkg;
    QcBackgroundItem *bkg_center;
    QcBackgroundItem *bkg_needle;
    QcNeedleItem* needle;
    QcNeedleItem* needle_small;
    QcValuesItem* Values;
    QcDegreesItem* DegreeSubItem;
    QcDegreesItem* DegreeItem;
    QcLabelItem* label;
    QcLabelItem* temp_label;
    const QString label_str = "RPM x1000";
    int bkg_colorstate=0;
    float maxRPM = 3000;

};

#endif // TACHOMETER_H
