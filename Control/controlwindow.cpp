#include "controlwindow.h"

controlwindow::controlwindow(QWidget *parent) : QMainWindow(parent)
{
  setWindowTitle("Controls");
  top_layout = new QGridLayout(this);

  drv8320 = new DRV8320S();
  top_layout->addWidget(drv8320);
  box = new QGroupBox("Controls" , this);
  box->setLayout(top_layout);
  this->setCentralWidget(box);
  this->setMinimumWidth(850);
  this->setMinimumHeight(600);

}
