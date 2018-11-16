#include "controlwindow.h"


controlwindow::controlwindow(QWidget *parent) : QMainWindow(parent)
{
  setWindowTitle("Regulator");
  top_layout = new QGridLayout(this);

  torque = new EQChannel(this , 0 ,  "Torque");
  flux = new EQChannel(this   , 1 ,  "Flux" );
  bode = new bodeplot(this);

  top_layout->addWidget(torque, 0 , 0);
  top_layout->addWidget(flux ,  0 , 1);
  top_layout->addWidget(bode  , 0 , 2);

  groupbox = new QGroupBox(this);
  groupbox->setLayout(top_layout);
  this->setCentralWidget(groupbox);
  connect(torque->PI    , &PISection::PIchanged , bode , &bodeplot::PIchanged );
  connect(torque->EQ[0] , &EQsection::EQchanged , bode , &bodeplot::EQchanged);
  connect(torque->EQ[1] , &EQsection::EQchanged , bode , &bodeplot::EQchanged);
  connect(flux->PI      , &PISection::PIchanged , bode , &bodeplot::PIchanged );
  connect(flux->EQ[0]   , &EQsection::EQchanged , bode , &bodeplot::EQchanged);
  connect(flux->EQ[1]   , &EQsection::EQchanged , bode , &bodeplot::EQchanged);

  torque->PI->updateSettingsAndPlot(true);
  flux->PI->updateSettingsAndPlot(true);
}

controlwindow::~controlwindow(){

}
