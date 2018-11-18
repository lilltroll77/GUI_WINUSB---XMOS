#include "PIsection.h"

#include <QDebug>

PISection::PISection(QWidget *parent, int ID):
      QWidget(parent)
  {

      channelID = ID;
      topLayout = new QVBoxLayout(this);
      layout = new QBoxLayout(QBoxLayout::TopToBottom,this);
      groupBox = new QGroupBox(this);

      knob_fc = new Knob(logScale , this);
      knob_fc-> setTitle("Fc [Hz]");
      knob_fc-> setKnobColor("rgb(255, 127, 127)");
      knob_fc->setRange( 2 , 5000, 100);
      knob_fc->setDecimals(0);
      knob_fc->setSingleStep(1);
      knob_fc->setValue(DEFAULT_FC);
      knob_gain = new Knob(linScale , this);
      knob_gain-> setTitle("Gain [dB]");
      knob_gain-> setKnobColor("rgb(127, 127, 255)");
      knob_gain->setRange(-50 , 50 , 101);
      knob_gain->setDecimals(1);
      knob_gain->setSingleStep(0.1);
      knob_gain->setValue(DEFAULT_GAIN);

      layout->addWidget(knob_fc);
      layout->addWidget(knob_gain);
      layout->setMargin(0);
      layout->setSpacing(0);
      groupBox->setLayout(layout);
      groupBox->setFixedWidth(100);
      groupBox->setContentsMargins(0,0,0,0);
      QFont box_font = groupBox->font();
      box_font.setBold(false);
      groupBox->setFont(box_font);

      //qDebug() << groupBox->sizeHint().height();
      groupBox->setCheckable(false);
      groupBox->setChecked(true);
      groupBox->setToolTip(tr("PI controller"));
      groupBox->setTitle("PI");

      connect(knob_gain , SIGNAL(valueChanged(double)) , this ,   SLOT(slot_gainChanged(double)));
      connect(knob_fc ,   SIGNAL(valueChanged(double)) , this ,   SLOT(slot_fcChanged(double)));


      topLayout->addWidget(groupBox);
      topLayout->setContentsMargins(3,10,3,10);
      this->setLayout(topLayout);


  }


  void PISection::setBoxTitle(const QString & title){
      groupBox->setTitle(title);
  }


  double PISection::getFc(){
      return knob_fc->Value();
  }

  void PISection::setFc(double Fc , bool blocked){
      if(blocked)
        knob_fc->blockSignals(true);

      //if(knob_fc->isEnabled()==true)
      knob_fc->setValue(Fc);
      if(blocked)
          knob_fc->blockSignals(false);
 }


  double PISection::getGain(){
      return knob_gain->Value();
  }

  void PISection::setGain(double gain , bool blocked){
      if(blocked)
          knob_gain->blockSignals(true);
      knob_gain->setValue(gain);
      if(blocked)
          knob_gain->blockSignals(false);
 }

  bool PISection::getFilterActive(){
      return groupBox->isChecked();
  }

  void PISection::setFilterActive(bool state , bool blocked){
      if(blocked)
          groupBox->blockSignals(true);
      groupBox->setChecked(state);
      if(blocked)
          groupBox->blockSignals(false);
      }


  void::PISection::updateSettingsAndPlot(bool updatePlot){
      //PIsettings.Fc   = knob_fc->Value();
      //PIsettings.Gain = knob_gain->Value();
      calc_PI(PIsettings , B , A );
      if(updatePlot)
        emit PIchanged(B,A , channelID); //Signal to parent its time to update plot
      emit sendPIsettings(PIsettings , channelID);
  }


  //SLOTS
  void PISection::slot_gainChanged(double gain){
        PIsettings.Gain = gain;
        calc_PI(PIsettings , B , A );
        emit PIchanged(B , A , channelID);
        emit sendPIsettings(PIsettings , channelID);
  }


  void PISection::slot_fcChanged(double fc){
       PIsettings.Fc=fc;
       calc_PI(PIsettings , B , A );
       emit PIchanged(B , A , channelID);
       emit sendPIsettings(PIsettings , channelID);

  }

  void PISection::slot_filtertypeChanged(int type){
      if(type == Notch || type == AllPass )
          knob_gain->setDisabled(true);
      else
          knob_gain->setDisabled(false);

  }



  void PISection::slot_activeEQChanged(bool state){
  //

  }

  PISection::~PISection()
  {
  }

