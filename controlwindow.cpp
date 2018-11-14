#include "controlwindow.h"


controlwindow::controlwindow(QWidget *parent) : QMainWindow(parent)
{
  setWindowTitle("Regulator");
  top_layout = new QGridLayout(this);

  torque = new EQChannel(this , 0 , fs , "Torque");
  flux = new EQChannel(this   , 1 , fs , "Flux");
  bode = new bodeplot(this);

  top_layout->addWidget(torque, 0 , 0);
  top_layout->addWidget(flux ,  0 , 1);
  top_layout->addWidget(bode , 0 , 2 );

  groupbox = new QGroupBox(this);
  groupbox->setLayout(top_layout);
  this->setCentralWidget(groupbox);

}

controlwindow::~controlwindow(){

}
