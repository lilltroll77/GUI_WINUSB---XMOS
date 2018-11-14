#include "eqsection.h"
#include <QDebug>

EQsection::EQsection(QWidget *parent, int fs):
      QWidget(parent)
  {

      topLayout = new QVBoxLayout(this);
      layout = new QBoxLayout(QBoxLayout::TopToBottom,this);
      groupBox = new QGroupBox(this);

      knob_fc = new Knob(logScale , this);
      knob_fc-> setTitle("Fc [Hz]");
      knob_fc-> setKnobColor("rgb(255, 127, 127)");
      knob_fc->setRange( FMIN , fs, 100);
      knob_fc->setDecimals(0);
      knob_fc->setSingleStep(1);
      knob_fc->setValue(DEFAULT_FC);
      //knob_fc->setFixedWidth(80);
      knob_Q = new Knob(logScale , this);
      knob_Q-> setTitle("Q");
      knob_Q-> setKnobColor("rgb(127, 255, 127)");
      knob_Q->setRange(0.1 ,30 , 100);
      knob_Q->setDecimals(3);
      knob_Q->setSingleStep(0.01);
      knob_Q->setValue(DEFAULT_Q);
      //knob_Q->setFixedWidth(80);
      knob_gain = new Knob(linScale , this);
      knob_gain-> setTitle("Gain [dB]");
      knob_gain-> setKnobColor("rgb(127, 127, 255)");
      knob_gain->setRange(-20,10,60);
      knob_gain->setDecimals(1);
      knob_gain->setSingleStep(0.1);
      knob_gain->setValue(DEFAULT_GAIN);
      filterType = new QComboBox(this);
      filterType-> setToolTip(tr("Filter type"));

      filterType->addItem(tr("1:st order LP"),QVariant(LP1));
      filterType->addItem(tr("2:nd order LP"),QVariant(LP2));
      filterType->addItem(tr("1:st order HP"),QVariant(HP1));
      filterType->addItem(tr("2:nd order HP"),QVariant(HP2));

      filterType->addItem(tr("Low Shelf (1:st)"),QVariant(LowShelf1));
      filterType->addItem(tr("Low Shelf (2:nd)"),QVariant(LowShelf2));
      filterType->addItem(tr("High Shelf (1:st)"),QVariant(HighShelf1));
      filterType->addItem(tr("High Shelf (2:nd)"),QVariant(HighShelf2));
      filterType->addItem(tr("Peaking EQ"),QVariant(PeakingEQ));
      filterType->addItem(tr("Notch"),QVariant(Notch));
      filterType->addItem(tr("AllPass"),QVariant(AllPass));
      filterType->addItem(tr("BandPass"),QVariant(BandPass));
      filterType->addItem(tr("Mute"),QVariant(Mute));
      filterType->setCurrentIndex(DEFAULT_FILTER);
      //filterType->setFixedWidth(110);
      layout->addWidget(filterType);
      layout->addWidget(knob_fc);
      layout->addWidget(knob_Q);
      layout->addWidget(knob_gain);
      layout->setContentsMargins(1,5,1,1);
      layout->setSpacing(0);
      groupBox->setFixedSize(layout->sizeHint());
      groupBox->setLayout(layout);
      //qDebug() << groupBox->sizeHint().height();
      groupBox->setCheckable(true);
      groupBox->setChecked(false);
      groupBox->setToolTip(tr("Filter ON/BYPASS"));
      groupBox->setTitle("Parametric");
      groupBox->setFixedWidth(110);
      QFont box_font = groupBox->font();
      box_font.setBold(false);
      groupBox->setFont(box_font);

      connect(knob_gain , SIGNAL(valueChanged(double)) , this ,   SLOT(slot_gainChanged(double)));
      connect(knob_Q ,    SIGNAL(valueChanged(double)) , this ,   SLOT(slot_Q_Changed(double)));
      connect(knob_fc ,   SIGNAL(valueChanged(double)) , this ,   SLOT(slot_fcChanged(double)));
      connect(filterType, SIGNAL(currentIndexChanged(int)) , this, SLOT(slot_filtertypeChanged(int)) );
      connect(groupBox ,  SIGNAL(clicked(bool))   ,      this ,    SLOT(slot_activeEQChanged(bool)) );


      topLayout->addWidget(groupBox);
      topLayout->setContentsMargins(3,5,3,5);
      this->setLayout(topLayout);
  }


  void EQsection::setSectionID(int newID){
      sectionID=newID;
  }


  void EQsection::setBoxTitle(const QString & title){
      groupBox->setTitle(title);
  }


  double EQsection::getFc(){
      return knob_fc->Value();
  }

  void EQsection::setFc(double Fc , bool blocked){
      if(blocked)
        knob_fc->blockSignals(true);

      //if(knob_fc->isEnabled()==true)
      knob_fc->setValue(Fc);
      if(blocked)
          knob_fc->blockSignals(false);
 }

  double EQsection::getQ(){
      return knob_Q->Value();
  }

  void EQsection::setQ(double Q , bool blocked){
      if(blocked)
          knob_Q->blockSignals(true);
      knob_Q->setValue(Q);
      if(blocked)
          knob_Q->blockSignals(false);
 }

  double EQsection::getGain(){
      return knob_gain->Value();
  }

  void EQsection::setGain(double gain , bool blocked){
      if(blocked)
          knob_gain->blockSignals(true);
      knob_gain->setValue(gain);
      if(blocked)
          knob_gain->blockSignals(false);
 }

  bool EQsection::getFilterActive(){
      return groupBox->isChecked();
  }

  void EQsection::setFilterActive(bool state , bool blocked){
      if(blocked)
          groupBox->blockSignals(true);
      groupBox->setChecked(state);
      if(blocked)
          groupBox->blockSignals(false);
      }

 filterType_t EQsection::getFilterType(){
       return (filterType_t) filterType->currentIndex();
   }

 void EQsection::setFilterType(filterType_t type ,  bool blocked){
    if(blocked)
        filterType->blockSignals(true);
    filterType->setCurrentIndex((int) type);
    if(blocked)
        filterType->blockSignals(false);
    }

  void::EQsection::updateSettingsAndPlot(bool updatePlot , int new_fs){
      fs = new_fs;
      EQ_section_t EQ;
      EQ.Fc = knob_fc->Value();
      EQ.Q =  knob_Q->Value();
      EQ.Gain = knob_gain->Value();
      EQ.type = (filterType_t)  filterType->currentIndex();
      calcFilt( EQ, fs , B  ,A );
     // freqz(B ,A , freq);
      if(updatePlot)
        emit eqchanged(); //Signal to parent its time to update plot
  }


  //SLOTS
  void EQsection::slot_gainChanged(double gain){

  }

  void EQsection::slot_Q_Changed(double Q){

  }

  void EQsection::slot_fcChanged(double fc){

  }

  void EQsection::slot_filtertypeChanged(int type){
      if(type == Notch || type == AllPass )
          knob_gain->setDisabled(true);
      else
          knob_gain->setDisabled(false);
      if(type == LP1|| type==HP1)
          knob_Q->setDisabled(true);
      else
          knob_Q->setDisabled(false);

  }



  void EQsection::slot_activeEQChanged(bool state){
  //

  }

  EQsection::~EQsection()
  {
  }

