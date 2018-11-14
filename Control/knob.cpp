#include "knob.h"
#include "QDebug"

void updateDialPosition(QDial *dial , QDoubleSpinBox *spinbox , Scale scale){
    double minSpin=spinbox->minimum();
    double maxSpin=spinbox->maximum();
    double maxDial=(double) dial->maximum();
    double valueSpin=spinbox->value();
    double norm;

    if(scale==logScale){
        //map f into [0 1] with logscale
        //( log10(x)/log10(min)-1 ) / ( log10(max)/log10(min)-1 )
        double A= log10(minSpin);
        double B= log10(maxSpin)/log10(minSpin) -1;
        norm =(log10(valueSpin)/ A-1 ) / B;
    }else{
        //Normalize the Spinbox value to a value [0 1]
        // y =kx+m -> x= (y-m)/k
        norm = (valueSpin - minSpin) / (maxSpin-minSpin )  ;
    }

    dial->setSliderPosition((int) round( maxDial * norm));
}

void updateSpinboxValue( QDial *dial , QDoubleSpinBox *spinbox , Scale scale){
    double minSpin=spinbox->minimum();
    double maxSpin=spinbox->maximum();
    double maxDial=(double) dial->maximum();
    double valueDial=(double) dial->value();
    //Normalize the dial value to a value [0 1]
    double norm = (double) valueDial/ maxDial;
    double value;
    if(scale==logScale){
        double A= log10(minSpin);
        double B= log10(maxSpin)/log10(minSpin) -1;
        value = ( pow(10 , A * (1 + B * norm)) );
    }
    else
        value = ( (maxSpin-minSpin)*norm + minSpin );
    // disable the spinbox to fire a event that the spinbox has changed
     spinbox->setValue(value);
}


Knob::Knob(const Scale scaletype ,QWidget *parent ,  bool useBox) :
    QWidget(parent)
{
    {
        groupBox = new QGroupBox(this);
        if(!useBox)
            groupBox->setStyleSheet("QGroupBox{border:0}");

        dial = new QDial(this);
        dial->setNotchesVisible(true);
        dial->setFixedWidth(65);
        scale=scaletype;
        spinbox  = new QDoubleSpinBox(this);
        spinbox->setAlignment(Qt::AlignCenter);

        layout = new QBoxLayout(QBoxLayout::TopToBottom , this);
        layout->addWidget(dial);
        layout->addWidget(spinbox);
        layout->setMargin(5);
        layout->setSpacing(0);
        groupBox->setLayout( layout);


        top_layout = new QBoxLayout(QBoxLayout::TopToBottom  , this);
        top_layout->addWidget(groupBox);
        this->setLayout(top_layout);
        //groupBox->setContentsMargins(5,5,5,5);
        connect(dial,   SIGNAL(valueChanged(int))      , this , SLOT(dial_changed(int)) );
        connect(spinbox,SIGNAL(valueChanged(double))  , this , SLOT(spinbox_changed(double)) );
    }
}
//SLOTS
void Knob::dial_changed(int value){
    updateSpinboxValue(dial , spinbox , scale);
    //Only update if dial is pressed,
  /*  int mid = dial->maximum() >>1;
    int max = dial->maximum();
    int min = dial->minimum();
    if(!atMax & !atMin){
        dial->setValue(value);
        updateSpinboxValue(dial , spinbox , scale);
    }
     if(value == max)
        atMax=true;
     if(value == min )
        atMin=true;
     if(atMax){
         if(value<mid)
            dial->setSliderPosition(max);
         else{
            atMax=false;
            dial->setValue(value);
            updateSpinboxValue(dial , spinbox , scale);
      }}
      if(atMin){
         if(value>mid )
             dial->setSliderPosition(min);
         else{
            atMin=false;
            dial->setValue(value);
            updateSpinboxValue(dial , spinbox , scale);
         }
      }*/
}

void Knob::spinbox_changed(double value){
    dial->blockSignals(true);
    updateDialPosition(dial , spinbox , scale);
    emit valueChanged(value);
    dial->blockSignals(false);
}


void updateWidth(QDoubleSpinBox *spinbox){
    //const int charWidth=10;
    QFont font = spinbox->font();
    int charWidth = 2+font.pointSize();
    double min = spinbox->minimum();
    double max = spinbox->maximum();
    int dec = spinbox->decimals();

    QString string1 = QString("%1").arg(min,1);
    QString string2 = QString("%1").arg(max,1);

    int len1 = string1.length();
    int len2 = string2.length();
    int pixels=15; //up/down arrow

    if (len1> len2)
        pixels+= charWidth*len1;
    else
        pixels+= charWidth*len2;

    if ( dec > 0)
        pixels+=charWidth*dec+charWidth;  //add room for decimals
    if ( min<0)
        pixels+=charWidth; //add rom for "-"
    //spinbox->setFixedWidth(pixels);
}

void Knob::setRange(double min , double max , int steps){
    spinbox->setRange(min,max);
    updateWidth(spinbox);
    dial->setRange(0,steps);

    updateDialPosition ( dial , spinbox , scale);
}

void Knob::setValue(double arg){
   spinbox->setValue(arg);
   updateDialPosition(dial , spinbox , scale);
}

double Knob::Value(){
    return spinbox->value();
}


void Knob::setDecimals(int prec){
    spinbox->setDecimals(prec);
    updateWidth(spinbox);
}


void Knob::setKnobColor(QString color){
    dial->setStyleSheet(QString("background-color:") + color);
}

void Knob::setTitle(const QString &title){
    groupBox->setTitle(title);
}

void Knob::setDisabled(bool state){
    dial->setDisabled(state);
    spinbox->setDisabled(state);
    groupBox->setDisabled(state);
}

void Knob::setSingleStep(double singleStep){
    spinbox->setSingleStep(singleStep);
}

