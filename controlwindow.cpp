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
   knob_fuse->setTitle("Scales plot");
   knob_fuse->setToolTip("Fuse current is disabled. Only scales plots");
   knob_fuse->setRange(0.25 , 32 , 128);
   knob_fuse->setValue(20);
   knob_fuse->setDecimals(1);
   knob_fuse->setKnobColor("rgb(255, 200, 0)");

    knob_amp = new Knob(linScale , this , true );
    knob_amp->setFixedWidth(100);
    knob_amp->setTitle("Modulation [%]");
    knob_amp->setToolTip("Output voltage modulation");
    knob_amp->setRange(-100 , 100 , 200);
    knob_amp->setValue(0);
    knob_amp->setDecimals(0);
    knob_amp->setKnobColor("rgb(200, 212, 225)");

    knob_trimAngle = new Knob(linScale , this , true );
    knob_trimAngle->setFixedWidth(100);
    knob_trimAngle->setTitle("QE trim [deg]");
    knob_trimAngle->setToolTip("QE absolute shaft angle trim");
    knob_trimAngle->setRange(-5 , 5 , 200);
    knob_trimAngle->setValue(0);
    knob_trimAngle->setSingleStep(360.0/8192.0);
    knob_trimAngle->setDecimals(2);
    knob_trimAngle->setKnobColor("rgb(225, 212, 200)");


   button_reset = new QPushButton(this);
   icon_fuseOK.addFile(":/images/FuseOK.png");
   icon_fuseBurnt.addFile(":/images/FuseBURNT.png");
   button_reset->setIconSize(QSize(48,20));
   button_reset->setFixedWidth(80);
   button_reset->setFixedHeight(30);
   //button_reset->setToolTip("Reset fuse");
   button_reset->setToolTip("Fuse is currently disabled");
   button_reset->setDisabled(true);
   slot_ResetButtonState(true);

   drv8320 = new DRV8320S(this);

  top_layout->addWidget(torque, 0 , 0 , 1 , 2);
  top_layout->addWidget(flux ,  0 , 2 , 1 , 2);
  top_layout->addWidget(bode  , 0 , 5);
  top_layout->addWidget(knob_fuse , 1 , 0 , Qt::AlignRight);
  top_layout->addWidget(button_reset , 1 , 1 , Qt::AlignLeft);
  top_layout->addWidget(knob_amp , 1 , 2 , Qt::AlignLeft);
  top_layout->addWidget(knob_trimAngle , 1 , 3 , Qt::AlignLeft);
  top_layout->addWidget(drv8320->masterBox , 1 , 5 ,2,2);

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
  connect(knob_amp        , &Knob::valueChanged        , usb   , &USBbulk::sendModulation);
  connect(knob_trimAngle  , &Knob::valueChanged        , usb   , &USBbulk::sendTrimAngle);
  connect(knob_fuse       , &Knob::valueChanged        , w    , &MainWindow::currentRange);
  connect(knob_fuse       , &Knob::valueChanged        , w->gaugeWindow->currentGauge , &currentGague::setScale);
  connect(button_reset    , &QPushButton::clicked      , usb  , &USBbulk::sendFuseReset);
  //connect(button_reset    , &QPushButton::clicked      , this , &controlwindow::slot_ResetButtonState);
  connect(w               , &MainWindow::fuseStatus    , this , &controlwindow::slot_ResetButtonState );
  connect(this->drv8320   , &DRV8320S::send_DRV8320S   , usb  , &USBbulk::send_DRV8320S);
  connect(w               , &MainWindow::set_statusRow    , this->drv8320 , &DRV8320S::set_statusRow);
  connect(w               , &MainWindow::decode_DRVregs   , this->drv8320 , &DRV8320S::decode_DRVregs);

  w->currentRange(knob_fuse->Value());
  w->gaugeWindow->currentGauge->setScale(knob_fuse->Value());
  torque->PI->updateSettingsAndPlot(false);
  flux->PI->updateSettingsAndPlot(false);

 }



void controlwindow::slot_ResetButtonState(bool state){
    if(state == true){
        button_reset->setIcon(icon_fuseOK);
        button_reset->setStyleSheet("background-color:lightgreen");
    }
    else{
        button_reset->setIcon(icon_fuseBurnt);
        button_reset->setStyleSheet("background-color:red");
        knob_amp->blockSignals(true);
        knob_amp->setValue(0);
        knob_amp->blockSignals(false);
    }
}

controlwindow::~controlwindow(){

}
