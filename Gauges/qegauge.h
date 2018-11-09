#ifndef QEGAUGE_H
#define QEGAUGE_H

#include <QWidget>
#include "qcgaugewidget.h"

class QEgauge : public QWidget
{
    Q_OBJECT
public:
    explicit QEgauge(QWidget *parent = nullptr);
    QcGaugeWidget* widget;

signals:

public slots:
    void setAngle(float deg);

private:
    QcBackgroundItem *bkg;
    QcBackgroundItem *bkg_center;
    QcNeedleItem* needle;
    QcValuesItem* Values;
    QcDegreesItem* DegreeSubItem;
    QcDegreesItem* DegreeItem;
    QcLabelItem* label;
    const QString label_str = "Shaft angle";

};

#endif // QEGAUGE_H
