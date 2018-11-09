#include "tachometer.h"

tachometer::tachometer(QWidget *parent) : QWidget(parent)
{

    // prevent round floor of maxRPM
    maxRPM += 0.01;

    //widget
        widget = new QcGaugeWidget(parent);
        widget->setMinimumWidth(125);
        widget->setMinimumHeight(125);

    //background
        bkg = widget -> addBackground(98 , 0 , 360);
        bkg->clearrColors();
        bkg->addColor(0 , Qt::white);
        bkg_center = widget -> addBackground(7.5, 0 , 360);
        bkg_center->clearrColors();
        bkg_center->addColor(0,Qt::black);
        bkg_needle = widget -> addBackground(6, 0 , 360);
        bkg_needle->clearrColors();
        bkg_needle->addColor(0 , Qt::red);

        //labels
        label = widget->addLabel(70);
        label->setText(label_str);

        //needle
        needle = widget->addNeedle(85);
        needle->setColor(Qt::red);
        needle->setNeedle(QcNeedleItem::DiamonNeedle);

        // Degree
        DegreeItem =  widget->addDegrees(92.5);
        DegreeItem->setSubDegree(false);
        DegreeItem->setStep( 100.0f*(500.0f/ maxRPM));
        DegreeItem->setStopAngle(360);
        DegreeSubItem = widget->addDegrees(93);
        DegreeSubItem->setSubDegree(true);
        DegreeSubItem->setStep( 100.0f*(100.0f/ maxRPM));

        //Values
        Values = widget->addValues(65);
        Values->setMinValue(0);
        Values->setMaxValue(maxRPM/1000);
        Values->setStep(0.5f);
        widget->addArc(97)->setDgereeRange(0,360);
        //widget->addGlass(100);

}

void tachometer::setSpeed(float RPM){
    needle->setCurrentValue(100*RPM/maxRPM);
}
