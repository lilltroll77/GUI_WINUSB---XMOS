#include "QEgauge.h"

QEgauge::QEgauge(QWidget *parent) : QWidget(parent)
{
    //widget
        widget = new QcGaugeWidget(parent);
        widget->setMinimumWidth(125);
        widget->setMinimumHeight(125);

    //background
        bkg = widget -> addBackground(98 , 0 , 360);
        bkg->clearrColors();
        bkg->addColor(0 , Qt::white);
        bkg_center = widget -> addBackground(7 , 0 , 360);
        bkg_center->clearrColors();
        bkg_center->addColor(0 , Qt::black);

    // Arc
        //widget->addArc(98);


    //labels
        label = widget->addLabel(25);
        label->setText(label_str);
        label->setAngle(90);


     // Degree
        DegreeItem = widget->addDegrees(92.5);
        DegreeItem->setSubDegree(false);
        DegreeItem->setDgereeRange(0 , 360);
        DegreeItem->setStep( 100.0f* 45.0f/360.0f );
        DegreeSubItem = widget->addDegrees(93);
        DegreeSubItem->setSubDegree(true);
        DegreeSubItem->setStep( 100.0f* 15.0f/360.0f);
        DegreeSubItem->setDgereeRange(0 , 360);

        //Values
        Values = widget->addValues(65);
        Values->setMinValue(0);
        Values->setMaxValue(315);
        Values->setStep(45.0f);
        Values->setDgereeRange(90,315+90);


       // Values->setStartAngle(90);
       // Values->setStopAngle(270);

        //needle
        needle = widget->addNeedle(85);
        needle->setColor(Qt::red);
        needle->setNeedle(QcNeedleItem::CompassNeedle);
        needle->setDgereeRange(0 , 360);

        widget->addArc(97)->setDgereeRange(0,360);
        //widget->addGlass(100);
}

void QEgauge::setAngle(float deg){
    needle->setCurrentValue(deg/3.6f);
}
