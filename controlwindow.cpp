#include "controlwindow.h"


controlwindow::controlwindow(USBbulk* usb , QWidget *parent ) : QMainWindow(parent)
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
  connect(torque->PI    , &PISection::resetIntegrator  , usb  , &USBbulk::resetPIintegrator);
  connect(torque->PI    , &PISection::PIchanged , bode , &bodeplot::PIchanged );
  connect(torque->PI    , &PISection::sendPIsettings , usb , &USBbulk::sendPIsettings);
  connect(flux->PI    , &PISection::resetIntegrator  , usb  , &USBbulk::resetPIintegrator);
  connect(flux->PI      , &PISection::PIchanged , bode , &bodeplot::PIchanged );
  connect(flux->PI      , &PISection::sendPIsettings , usb , &USBbulk::sendPIsettings);

  for(int i=0; i<2 ; i++){
    connect(torque->EQ[i] , &EQsection::EQchanged    , bode , &bodeplot::EQchanged);
    connect(torque->EQ[i] , &EQsection::sendEQsettings , usb , &USBbulk::sendEQsettings);
    connect(flux->EQ[i]   , &EQsection::EQchanged    , bode , &bodeplot::EQchanged);
    connect(flux->EQ[i] ,   &EQsection::sendEQsettings , usb , &USBbulk::sendEQsettings);

  }
  torque->PI->updateSettingsAndPlot(false);
  flux->PI->updateSettingsAndPlot(false);

 }

controlwindow::~controlwindow(){

}
