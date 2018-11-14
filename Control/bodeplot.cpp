#include "bodeplot.h"
#include "transform.h"
#include "QDebug"

bodeplot::bodeplot(QWidget *parent) : QWidget(parent)
{

    chart = new QChart();
    chartview = new QChartView(chart);
    int len=256;
    freq.reserve(len);
    for(int i=0; i<len ; i++){
         double f = lin2log(i , len, 150, 0.001);
         //qDebug()<<f;
         bodeFlux.append(f , 0);
         bodeTorque.append(f , 0);
    }
    freqAxis.setMax(150);
    freqAxis.setMin(0.001);
    freqAxis.setMinorTickCount(8);
    freqAxis.setTitleText("Frequency [kHz]");
    freqAxis.setLabelFormat("%G");

    levelAxis.setMin(-50);
    levelAxis.setMax(10);
    levelAxis.setTickCount(7);
    levelAxis.setMinorTickCount(1);
    levelAxis.setTitleText("Level [dB]");

    bodeTorque.setName("Torque");
    bodeFlux.setName("Magnetic Flux");

    chart->addSeries(&bodeTorque);
    chart->addSeries(&bodeFlux);

    chart->addAxis(&freqAxis , Qt::AlignBottom);
    chart->addAxis(&levelAxis , Qt::AlignLeft);
    bodeTorque.attachAxis(&freqAxis);
    bodeTorque.attachAxis(&levelAxis);
    bodeFlux.attachAxis(&freqAxis);
    bodeFlux.attachAxis(&levelAxis);
    chartview->setChart(chart);
    chartview->setMinimumWidth(350);
    top_layout.addWidget(chartview);
    this->setLayout(&top_layout);





}
