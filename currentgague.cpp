#include "currentgague.h"


currentGague::currentGague(QWidget *parent , float imax) : QWidget(parent)
{

    Imax = imax;

    // Widget
        widget = new QcGaugeWidget(parent);
        widget->setMinimumWidth(125);
        widget->setMinimumHeight(125);

    // Background
        bkg = widget -> addBackground(98 , DegreeStart , DegreeEnd);
        bkg->clearrColors();
        bkg->addColor(0 , Qt::white);
        bkg_center = widget -> addBackground(5 , 0 , 360);
        bkg_center->clearrColors();
        bkg_center->addColor(0 , Qt::black);

        // Arc
        widget->addArc(98)->setDgereeRange(DegreeStart , DegreeEnd);

        // Color Band
        ColorBand = widget->addColorBand(95);
        QList<QPair<QColor,float>> BandColors;
        for(int i=0; i< 4 ; i++){
            BandColors.append(pair[i]);
        }
        ColorBand->setColors(BandColors);
        ColorBand->setDgereeRange(DegreeStart , DegreeEnd);


    // Labels
        label = widget->addLabel(20);
        label->setText(label_str);
        ms_needle.label = widget->addLabel(20);
        ms_needle.label->setAngle(90);
        ms_needle.label->setText("True RMS");

    //Needles
        peak_needle.label = widget->addLabel(35);
        peak_needle.label-> setText("Peak");
        peak_needle.label->setColor(Qt::red);
        peak_needle.label->setAngle(90);

        ms_needle.Item =   widget->addNeedle(85);
        peak_needle.Item = widget->addNeedle(85);
        ms_needle.Item->setColor(Qt::black);
        peak_needle.Item->setColor(Qt::red);
        ms_needle.Item->setNeedle(QcNeedleItem::TriangleNeedle);
        peak_needle.Item->setNeedle(QcNeedleItem::TriangleNeedle);
        peak_needle.Item->setDgereeRange(DegreeStart , DegreeEnd);
        ms_needle.Item->setDgereeRange(DegreeStart , DegreeEnd);

    // Degree Items
        DegreeItem =    widget->addDegrees(92.5);
        DegreeItem->    setDgereeRange(DegreeStart , DegreeEnd);
        DegreeItem->    setSubDegree(false);
        DegreeSubItem = widget->addDegrees(93);
        DegreeSubItem-> setDgereeRange(DegreeStart , DegreeEnd);
        DegreeSubItem-> setSubDegree(true);

    //Values
        Values = widget->addValues(65);
        Values->setDgereeRange(DegreeStart , DegreeEnd);
        Values->setMinValue(0);
        Values->setMaxValue(Imax);
        setScale(Imax);
        widget->update();
}

void currentGague::setScale(float I){
    Imax = I;
    float Imax_scale =scale_overcurrent*I;
    Values->setMaxValue(Imax_scale);
    if(Imax_scale>10){
        Values->setStep(5);
        DegreeSubItem->setStep(100/Imax_scale);
        DegreeItem->setStep(500/Imax_scale);
    }else if(I<1){
        Values->setStep(0.1f);
        DegreeSubItem->setStep(2/Imax_scale);
        DegreeItem->setStep(10/Imax_scale);
   }else if(I<3){
        Values->setStep(0.5f);
        DegreeSubItem->setStep(10/Imax_scale);
        DegreeItem->setStep(50/Imax_scale);
    }else{
        Values->setStep(1);
        DegreeSubItem->setStep(20/Imax_scale);
        DegreeItem->setStep(100/Imax_scale);
    }
}

void currentGague::setCurrent(struct I_t *I){
    peak_needle.Item->setCurrentValue(I->peak * (100/(scale_overcurrent*Imax)));
    ms_needle.Item  ->setCurrentValue(I->ms * (100/(scale_overcurrent*Imax)));
}
