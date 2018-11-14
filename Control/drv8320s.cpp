#include "drv8320s.h"

void addIdrive(box_t *gate , QGridLayout *layout, QString str  ,const unsigned short current[16] , int col , int mult){
    gate->comboBox = new QComboBox;
    gate->comboBox->setMaximumWidth(100);
    gate->label = new QLabel(str);
    for(int i=0; i<16 ; i++){
        QString str = QString("%1mA").arg(mult*current[i]);
        gate->comboBox->addItem(str , i);
    }
    layout->addWidget(gate->label , 1 , col);
    layout->addWidget(gate->comboBox , 2 , col);
}

DRV8320S::DRV8320S(QWidget *parent) : QWidget(parent)
{
    masterLayout = new QGridLayout(this);
    masterBox = new QGroupBox(this);
    QGridLayout* buttonLayout = new QGridLayout(this);
    QGroupBox* buttonBox = new QGroupBox(this);
    //Error "buttons"

    QPalette palette;
    QFont font_button;
    font_button.setPointSize(8);
    font_button.setBold(true);

    //const char stylesheet_disable[]="QPushButton{background-color:red ;color:black} ; QToolTip { color: #ffffff }";

    for(int reg=0; reg<2 ; reg++ )
        for(int bit=0; bit<10 ; bit++ ){
            error_buttons[reg][bit]= new QPushButton( reg_text[reg][bit]);
            error_buttons[reg][bit]->setToolTip(reg_tooltip[reg][bit]);
            error_buttons[reg][bit]->setFont(font_button);
            error_buttons[reg][bit]->setDisabled(true);
            if(((reg ==1) && (bit==0))){
             error_buttons[reg][bit]->setStyleSheet(stylesheet_error);
             //error_buttons[reg][bit]->setChecked(true);
            }
            buttonLayout->addWidget(error_buttons[reg][bit] , reg , bit+1);
        }
    reset_button = error_buttons[1][9];
    reset_button->setDisabled(false);
    reset_button->setStyleSheet(stylesheet_reset);
    buttonLayout->setSpacing(0);
    buttonBox->setLayout(buttonLayout);
    buttonBox->setTitle("Status");

    QFont font;
    font.setPointSize(10);
    font.setBold(false);


    // IDrive
    iDrive.layout =  new QGridLayout(this);
    iDrive.box = new QGroupBox();
    iDrive.box->setTitle("I Drive settings");
    addIdrive( &iDrive.POS_HS , iDrive.layout , "Positive High Side"  , iDrive.current , 1 , 1);
    addIdrive( &iDrive.NEG_HS , iDrive.layout , "Negative High Side"  , iDrive.current , 2 , 2);
    addIdrive( &iDrive.POS_LS , iDrive.layout , "Positive Low Side"   , iDrive.current , 3 , 1);
    addIdrive( &iDrive.NEG_LS , iDrive.layout , "Negative Low Side"   , iDrive.current , 4 , 2);
    iDrive.box->setFont(font);
    iDrive.box->setLayout(iDrive.layout);


    // Tdrive
    tDrive.layout = new QVBoxLayout();
    tDrive.layout->setAlignment(Qt::AlignHCenter);
    tDrive.box = new QGroupBox();
    tDrive.box->setTitle("T Drive settings");
    tDrive.combo.comboBox = new QComboBox;
    tDrive.combo.comboBox->setMaximumWidth(100);
    tDrive.combo.label = new QLabel("Minimum dead time");
    for(int i=0; i<4 ; i++){
        QString str = QString("%1 ns").arg(tDrive.delay[i]);
        tDrive.combo.comboBox->addItem(str ,  i);
    }
    tDrive.layout->addWidget( tDrive.combo.label);
    tDrive.layout->addWidget( tDrive.combo.comboBox);
    tDrive.box->setFont(font);
    tDrive.box->setLayout(tDrive.layout);
    tDrive.box->setMaximumWidth(150);

    // ODT
    ODT.layout = new QVBoxLayout(this);
    ODT.layout->setAlignment(Qt::AlignBottom);
    ODT.box = new QGroupBox();
    ODT.box->setTitle("Overcurrent deglitch time");
    ODT.combo.comboBox = new QComboBox;
    ODT.combo.comboBox->setMaximumWidth(100);
    for(int i=0; i<4 ; i++){
        QString str = QString("%1 us").arg(ODT.delay[i]);
        ODT.combo.comboBox->addItem(str ,  i);
    }

    ODT.layout->addWidget(ODT.combo.comboBox);
    ODT.box->setFont(font);
    ODT.box->setLayout(ODT.layout);
    //ODT.box->setMaximumWidth(175);


    //VDS
    VDS.layout = new QVBoxLayout(this);
    VDS.box = new QGroupBox();
    VDS.layout->setAlignment(Qt::AlignHCenter);
    VDS.box->setTitle("VDS overcurrent trip voltage");
    VDS.combo.comboBox = new QComboBox;
    VDS.combo.comboBox->setMaximumWidth(100);

    for(int i=0; i<16 ; i++){
        QString str = QString("%1 V").arg(VDS.voltage[i]);
        VDS.combo.comboBox->addItem(str ,  i);
    }
    VDS.layout->setAlignment(Qt::AlignBottom);
    VDS.layout->addWidget(VDS.combo.comboBox);
    VDS.box->setFont(font);
    VDS.box->setLayout(VDS.layout);

    masterLayout->addWidget(buttonBox , 0 , 1 ,1 , 4);
    masterLayout->addWidget(iDrive.box , 1 ,1 ,1,3);
    masterLayout->addWidget(tDrive.box , 2 ,1 );
    masterLayout->addWidget(ODT.box , 2 , 2  );
    masterLayout->addWidget(VDS.box ,2 ,3);
    masterBox->setLayout(masterLayout);
    masterBox->setTitle("DRV8320S settings");
    font.setBold(true);
    masterBox->setFont(font);
    QHBoxLayout top_layout(this);
    top_layout.addWidget(masterBox);
    this->setLayout(&top_layout);

    connect(VDS.combo.comboBox, SIGNAL(currentIndexChanged(int)) , this, SLOT(slot_VDSChanged(int)) );
    connect(ODT.combo.comboBox, SIGNAL(currentIndexChanged(int)) , this, SLOT(slot_ODTChanged(int)) );
    connect(tDrive.combo.comboBox, SIGNAL(currentIndexChanged(int)) , this,  SLOT(slot_TdriveChanged(int)));
    connect(iDrive.POS_HS.comboBox ,SIGNAL(currentIndexChanged(int)) , this,  SLOT(slot_IDrive_P_HS(int)));
    connect(iDrive.NEG_HS.comboBox ,SIGNAL(currentIndexChanged(int)) , this,  SLOT(slot_IDrive_N_HS(int)));
    connect(iDrive.POS_LS.comboBox ,SIGNAL(currentIndexChanged(int)) , this,  SLOT(slot_IDrive_P_LS(int)));
    connect(iDrive.NEG_LS.comboBox ,SIGNAL(currentIndexChanged(int)) , this,  SLOT(slot_IDrive_N_LS(int)));
    connect(reset_button , SIGNAL(clicked(bool) ), this , SLOT(DRV_reset(bool)));

}
/*
void DRV8320S::DRVerror_changed(short* data){
    QString str;
    for(int bit=0 ; bit<10 ; bit++)
        if((data[0]>>bit)&1)
            str=QString("Error").append(reg1[10-bit]);
    for(int bit=0 ; bit<8 ; bit++)
        if((data[1]>>bit)&1)
            str=QString("Error").append(reg2[8-bit]);
    errorMsgBox->setText(str);
    errorMsgBox->setButtonText(0 , "OK");
    errorMsgBox->show();
}*/

void DRV8320S::set_GateDriveNegHiSide(int index){
    QComboBox* box = iDrive.NEG_HS.comboBox;
    bool blocked = box->signalsBlocked();
    box->blockSignals(true);
    box->setCurrentIndex(index);
    box->blockSignals(blocked);
    }

void DRV8320S::set_GateDriveNegLoSide(int index){
    QComboBox* box = iDrive.NEG_LS.comboBox;
    bool blocked = box->signalsBlocked();
    box->blockSignals(true);
    box->setCurrentIndex(index);
    box->blockSignals(blocked);
    }

void DRV8320S::set_GateDrivePosHiSide(int index){
    QComboBox* box=iDrive.POS_HS.comboBox;
    bool blocked = box->signalsBlocked();
    box->blockSignals(true);
    box->setCurrentIndex(index);
    box->blockSignals(blocked);
    }

void DRV8320S::set_GateDrivePosLoSide(int index){
    QComboBox* box=iDrive.POS_LS.comboBox;
    bool blocked = box->signalsBlocked();
    box->blockSignals(true);
    box->setCurrentIndex(index);
    box->blockSignals(blocked);
    }

void DRV8320S::set_TDrive(int index){
    QComboBox* box = tDrive.combo.comboBox;
    bool blocked = box->signalsBlocked();
    box->blockSignals(true);
    box->setCurrentIndex(index);
    box->blockSignals(blocked);
}

void DRV8320S::set_VDS_LVL(int index){
    QComboBox* box = VDS.combo.comboBox;
    bool blocked = box->signalsBlocked();
    box->blockSignals(true);
    box->setCurrentIndex(index);
    box->blockSignals(blocked);
}

void DRV8320S::set_status(int reg , int bit, bool status){
    if(reg==1)
        bit++;
    if(status)
        error_buttons[reg][9-bit]->setStyleSheet(stylesheet_error);
    else
        error_buttons[reg][9-bit]->setStyleSheet(stylesheet_ok);

    error_buttons[1][0]->setStyleSheet(stylesheet_ok);



}

void DRV8320S::DRV_reset(bool){
    //writeSerialCommand(COM_DRV_RESET , serial);
}

void DRV8320S::slot_ODTChanged(int index){
    //writeSerialUint(COM_ODT , index , serial);
}

void DRV8320S::slot_TdriveChanged(int index){
    //writeSerialUint(COM_TDRIVE , index , serial);
}

void DRV8320S::slot_VDSChanged(int index){
    //writeSerialUint(COM_VDS , index , serial);
}

void DRV8320S::slot_IDrive_P_HS(int index){
    //writeSerialUint(COM_IDRIVE_P_HS , index , serial);
}

void DRV8320S::slot_IDrive_N_HS(int index){
    //writeSerialUint(COM_IDRIVE_N_HS , index , serial);
}

void DRV8320S::slot_IDrive_P_LS(int index){
    //writeSerialUint(COM_IDRIVE_P_LS , index , serial);
}

void DRV8320S::slot_IDrive_N_LS(int index){
    //writeSerialUint(COM_IDRIVE_N_LS , index , serial);
}
