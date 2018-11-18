#include "eqchannel.h"
#include <QDebug>
#include <QFont>

EQChannel::EQChannel(QWidget *parent , int new_channel, QString title ) :
    QWidget(parent){

    channel=new_channel;

    layout = new QGridLayout(this);
    top_layout = new QVBoxLayout(this);
    groupBox = new QGroupBox(this);

    PI = new PISection(this , channel);
    layout->addWidget(PI , 0 , 0 , 1 , 1 , Qt::AlignTop);
    for(int i=0; i<2 ; i++){
        int section = i;
        EQ[i] = new EQsection(this , channel , section);
        layout->addWidget(EQ[i] , 0 , 1+i , 2 , 1 , Qt::AlignTop);
    }
    layout->setSpacing(0);
    groupBox->setLayout(layout);
    groupBox->setTitle(title);

    QFont box_font = groupBox->font();
    box_font.setBold(true);
    groupBox->setFont(box_font);
    top_layout->addWidget(groupBox);


    this->setLayout(top_layout);


 }

EQChannel::~EQChannel(){

}
