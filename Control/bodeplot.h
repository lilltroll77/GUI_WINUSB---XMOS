#ifndef BODEPLOT_H
#define BODEPLOT_H

#include <QWidget>
#include <QtCharts>
#include <QChartView>
#include <QVector>

QT_CHARTS_USE_NAMESPACE

class bodeplot : public QWidget
{
    Q_OBJECT
public:
    explicit bodeplot(QWidget *parent = nullptr);

signals:

public slots:

private:
    QChart* chart;
    QChartView* chartview;
    QLineSeries bodeTorque;
    QLineSeries bodeFlux;
    QLogValueAxis freqAxis;
    QValueAxis levelAxis;
    QVector<float> freq;
    QVBoxLayout top_layout;

};

#endif // BODEPLOT_H
