#ifndef KNOB_H
#define KNOB_H

#include <QWidget>
#include <QLabel>
#include <QDial>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QLineEdit>
#include <math.h>
#include <QString>
#include <QGroupBox>
#include <QFormLayout>


enum Scale{linScale,logScale};

class Knob : public QWidget
{
    Q_OBJECT
public:
  explicit Knob(const Scale scaletype = linScale,  QWidget *parent = 0 , bool useBox=true);
  void setRange(double min , double max , int steps);
  double Value();
  void setValue(double value);
  void setKnobColor(QString color);
  void setTitle(const QString &title);
  void setDecimals(int prec);
  void setSingleStep(double singleStep);


signals:
void valueChanged(double Value);

public slots:
void dial_changed(int value);
void spinbox_changed(double value);
void setDisabled(bool state);


private:
QBoxLayout *layout;
QBoxLayout *top_layout;
bool atMax=false;
bool atMin=false;
Scale scale;
QWidget *widget;
QLabel *label_title;
QGroupBox *groupBox;
//QLineEdit *lineEdit_min;
//QLineEdit *lineEdit_max;
QLabel *label_unit;
QDial *dial;
QDoubleSpinBox *spinbox;




};

#endif // KNOB_H
