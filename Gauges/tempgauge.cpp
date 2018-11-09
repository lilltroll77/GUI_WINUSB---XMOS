#include "tempgauge.h"

tempgauge::tempgauge(QWidget *parent) : QWidget(parent)
{


    //widget
        widget = new QcGaugeWidget(parent);
        widget->setMinimumWidth(125);
        widget->setMinimumHeight(125);

    //background
        bkg = widget -> addBackground(98 , 0 , 360);
        bkg->clearrColors();
        bkg->addColor(0 , Qt::white);
        bkg_center = widget -> addBackground(5 , 0 , 360);
        bkg_center->clearrColors();
        bkg_center->addColor(0 , Qt::black);

    // Arc
        //widget->addArc(98);


        //Colorband
        ColorBand = widget->addColorBand(95);
        QPair<QColor,float> pair;
        for(int i=0; i< 4 ; i++){
            pair.first= color[i];
            pair.second = temp2percent(temp_colorregions[i+1]);
            BandColors.append(pair);
        }
        ColorBand->setColors(BandColors);


    //labels
        label = widget->addLabel(70);
        label->setText(label_str);
        temp_label =  widget->addLabel(40);
        temp_label->setColor(Qt::black);

    //needle
        needle = widget->addNeedle(85);
        needle->setColor(Qt::red);

    // Degree
        DegreeItem =    widget->addDegrees(92.5);
        DegreeItem->setSubDegree(false);
        DegreeSubItem = widget->addDegrees(93);
        DegreeSubItem->setSubDegree(true);
        DegreeSubItem->setStep( 2/100.0f*(temp_colorregions[4]-temp_colorregions[0]));

    //Values
        Values = widget->addValues(65);
        Values->setMinValue(temp_colorregions[0]);
        Values->setMaxValue(temp_colorregions[4]);
        widget->addArc(97)->setDgereeRange(0,360);
        //widget->addGlass(100);

}
float tempgauge::temp2percent(float T){
    return 100.0f*(T-temp_colorregions[0])/(temp_colorregions[4]-temp_colorregions[0]);
}

void tempgauge::setTemp(float T){
    needle->setCurrentValue(temp2percent(T));
    temp_label->setText(QString("%1 C°").arg(T , 0 , 'f' , 1));
    if(T < temp_colorregions[3]){
        if(bkg_colorstate){
            temp_label->setColor( Qt::black);
            bkg->clearrColors();
            bkg->addColor(0 , Qt::white);
            bkg_colorstate=0;
        }
    }else if(!bkg_colorstate){
        temp_label->setColor( color[3]);
        bkg->clearrColors();
        bkg->addColor(0 , QColor(255,225,225));
        bkg_colorstate=1;
    }
}

