#include "controlwindow.h"
#include "currentgague.h"


controlwindow::controlwindow(USBbulk* usb , MainWindow* w , QWidget *parent ) : QMainWindow(parent)
{
  setWindowTitle("Regulator");
  top_layout = new QGridLayout(this);

  torque = new EQChannel(this , 0 ,  "Torque");
  flux = new EQChannel(this   , 1 ,  "Flux" );
  bode = new bodeplot(this);

  knob_fuse = new Knob(linScale , this , true);
  knob_fuse->setFixedWidth(100);
   knob_fuse->setTitle("Fuse [A]");
   knob_fuse->setToolTip("Fuse current");
   knob_fuse->setRange(1 , 32 , 128);
   knob_fuse->setValue(20);
   knob_fuse->setDecimals(1);
   knob_fuse->setKnobColor("rgb(255, 200, 0)");


   button_reset = new QPushButton(this);
   icon_fuseOK.addFile(":/images/FuseOK.png");
   icon_fuseBurnt.addFile(":/images/FuseBURNT.png");
   button_reset->setIconSize(QSize(48,20));
   button_reset->setFixedWidth(80);
   button_reset->setFixedHeight(30);
   button_reset->setToolTip("Reset fuse");
   slot_ResetButtonState(false);

  top_layout->addWidget(torque, 0 , 0 , 1 , 2);
  top_layout->addWidget(flux ,  0 , 2);
  top_layout->addWidget(bode  , 0 , 3);
  top_layout->addWidget(knob_fuse , 1 , 0 , Qt::AlignRight);
  top_layout->addWidget(button_reset , 1 , 1 , Qt::AlignLeft);

  groupbox = new QGroupBox(this);
  groupbox->setLayout(top_layout);
  this->setCentralWidget(groupbox);
  connect(torque->PI    , &PISection::resetIntegrator  , usb  , &USBbulk::resetPIintegrator);
  connect(torque->PI    , &PISection::PIchanged        , bode , &bodeplot::PIchanged );
  connect(torque->PI    , &PISection::sendPIsettings   , usb  , &USBbulk::sendPIsettings);
  connect(flux->PI      , &PISection::resetIntegrator  , usb  , &USBbulk::resetPIintegrator);
  connect(flux->PI      , &PISection::PIchanged        , bode , &bodeplot::PIchanged );
  connect(flux->PI      , &PISection::sendPIsettings   , usb  , &USBbulk::sendPIsettings);

  for(int i=0; i<2 ; i++){
    connect(torque->EQ[i] , &EQsection::EQchanged      , bode , &bodeplot::EQchanged);
    connect(torque->EQ[i] , &EQsection::sendEQsettings , usb  , &USBbulk::sendEQsettings);
    connect(flux->EQ[i]   , &EQsection::EQchanged      , bode , &bodeplot::EQchanged);
    connect(flux->EQ[i]   , &EQsection::sendEQsettings , usb  , &USBbulk::sendEQsettings);

  }
  connect(knob_fuse       , &Knob::valueChanged        , usb  , &USBbulk::sendFuseCurrent);
  connect(knob_fuse       , &Knob::valueChanged        , w    , &MainWindow::currentRange);
  connect(knob_fuse       , &Knob::valueChanged        , w->gaugeWindow->currentGauge , &currentGague::setScale);
  connect(button_reset    , &QPushButton::clicked      , usb  , &USBbulk::sendFuseReset);
  connect(button_reset    , &QPushButton::clicked      , this , &controlwindow::slot_ResetButtonState);
  connect(w               , &MainWindow::fuseStatus     , this , &controlwindow::slot_ResetButtonState );
  w->currentRange(knob_fuse->Value());
  w->gaugeWindow->currentGauge->setScale(knob_fuse->Value());
  torque->PI->updateSettingsAndPlot(false);
  flux->PI->updateSettingsAndPlot(false);

 }



void controlwindow::slot_ResetButtonState(bool state){
    if(state == fusestate)
        return;
    if(state == false){
        button_reset->setIcon(icon_fuseOK);
        button_reset->setStyleSheet("background-color:lightgreen");
        fusestate = false;
    }
    else{
        button_reset->setIcon(icon_fuseBurnt);
        button_reset->setStyleSheet("background-color:red");
        fusestate = true;
    }
}

controlwindow::~controlwindow(){

}
