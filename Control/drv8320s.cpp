#include "drv8320s.h"
#include "usbbulk.h"

#define HISIDE_REG 3
#define LOSIDE_REG 4
#define TDRIVE_REG 4
#define DEAD_TIME_REG 5
#define OCP_DEG_REG 5
#define VDS_LVL_REG 5


static inline int get_bitfield(quint32 y, quint32 shift, quint32 len)
{
    return (y>>shift) & ((1<<len)-1);
}

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
    /*
    iDrive.POS_HS.comboBox->setCurrentIndex(1);
    iDrive.NEG_HS.comboBox->setCurrentIndex(2);
    iDrive.POS_LS.comboBox->setCurrentIndex(1);
    iDrive.NEG_LS.comboBox->setCurrentIndex(2);
*/
    iDrive.box->setFont(font);
    iDrive.box->setLayout(iDrive.layout);


    // Tdrive
    tDrive.layout = new QVBoxLayout();
    tDrive.layout->setAlignment(Qt::AlignHCenter);
    tDrive.box = new QGroupBox();
    tDrive.box->setTitle("Peak gate-current drive time");
    tDrive.combo.comboBox = new QComboBox;
    tDrive.combo.comboBox->setMaximumWidth(100);
    //tDrive.combo.label = new QLabel("Peak gate-current drive time");
    for(int i=0; i<4 ; i++){
        QString str = QString("%1 ns").arg(tDrive.delay[i]);
        tDrive.combo.comboBox->addItem(str ,  i);
    }
    tDrive.layout->addWidget( tDrive.combo.label);
    tDrive.layout->addWidget( tDrive.combo.comboBox);
    tDrive.box->setFont(font);
    tDrive.box->setLayout(tDrive.layout);
    tDrive.box->setMaximumWidth(250);

    // OCP_DEG
    OCP_DEG.layout = new QVBoxLayout(this);
    OCP_DEG.layout->setAlignment(Qt::AlignBottom);
    OCP_DEG.box = new QGroupBox();
    OCP_DEG.box->setTitle("Overcurrent deglitch time");
    OCP_DEG.combo.comboBox = new QComboBox;
    OCP_DEG.combo.comboBox->setMaximumWidth(100);
    for(int i=0; i<4 ; i++){
        QString str = QString("%1 us").arg(OCP_DEG.delay[i]);
        OCP_DEG.combo.comboBox->addItem(str ,  i);
    }

    OCP_DEG.layout->addWidget(OCP_DEG.combo.comboBox);
    OCP_DEG.box->setFont(font);
    OCP_DEG.box->setLayout(OCP_DEG.layout);
    //ODT.box->setMaximumWidth(175);

    //Deadtime
    Deadtime.layout = new QVBoxLayout(this);
    Deadtime.box = new QGroupBox();
    Deadtime.layout->setAlignment(Qt::AlignHCenter);
    Deadtime.box->setTitle("Dead time");
    Deadtime.combo.comboBox = new QComboBox;
    Deadtime.combo.comboBox->setMaximumWidth(100);
    for(int i=0; i<4 ; i++){
        QString str = QString("%1 ns").arg(Deadtime.delay[i]);
        Deadtime.combo.comboBox->addItem(str ,  i);
    }
    Deadtime.layout->addWidget( Deadtime.combo.label);
    Deadtime.layout->addWidget( Deadtime.combo.comboBox);
    Deadtime.box->setFont(font);
    Deadtime.box->setLayout(Deadtime.layout);
    Deadtime.box->setMaximumWidth(150);

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
    masterLayout->addWidget(OCP_DEG.box , 2 , 2  );
    masterLayout->addWidget(VDS.box ,2 ,3);
    masterLayout->addWidget(Deadtime.box , 2, 4);
    masterBox->setLayout(masterLayout);
    masterBox->setTitle("DRV8320S settings");
    font.setBold(true);
    masterBox->setFont(font);
    QHBoxLayout top_layout(this);
    top_layout.addWidget(masterBox);
    this->setLayout(&top_layout);

    connect(VDS.combo.comboBox,     SIGNAL(currentIndexChanged(int))   , this, SLOT(slot_VDSChanged(int)) );
    connect(OCP_DEG.combo.comboBox,     SIGNAL(currentIndexChanged(int))   , this, SLOT(slot_OCP_DEGChanged(int)) );
    connect(tDrive.combo.comboBox,  SIGNAL(currentIndexChanged(int))   , this, SLOT(slot_TdriveChanged(int)));
    connect(Deadtime.combo.comboBox,  SIGNAL(currentIndexChanged(int))   , this, SLOT(slot_DeadtimeChanged(int)) );
    connect(iDrive.POS_HS.comboBox ,SIGNAL(currentIndexChanged(int))   , this, SLOT(slot_IDrive_P_HS(int)));
    connect(iDrive.NEG_HS.comboBox ,SIGNAL(currentIndexChanged(int))   , this, SLOT(slot_IDrive_N_HS(int)));
    connect(iDrive.POS_LS.comboBox ,SIGNAL(currentIndexChanged(int))   , this, SLOT(slot_IDrive_P_LS(int)));
    connect(iDrive.NEG_LS.comboBox ,SIGNAL(currentIndexChanged(int))   , this, SLOT(slot_IDrive_N_LS(int)));
    connect(reset_button ,          SIGNAL(clicked(bool) )             , this, SLOT(DRV_reset(bool)));

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

void DRV8320S::set_Deadtime(int index){
    QComboBox* box = Deadtime.combo.comboBox;
    bool blocked = box->signalsBlocked();
    box->blockSignals(true);
    box->setCurrentIndex(index);
    box->blockSignals(blocked);
}

void DRV8320S::set_OCP_DEG(int index){
   QComboBox* box = OCP_DEG.combo.comboBox;
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

void DRV8320S::decode_DRVregs(void* regptr){
    quint16* reg = (quint16*)regptr;
    qDebug() << QString().sprintf("Reg3=%x, Reg4=%x , Reg5=%x", reg[3], reg[4] , reg[5]);
    //qDebug() <<"Low side" <<reg->reg4.IDRIVEN_LS << reg->reg4.IDRIVEP_LS;
    //qDebug() <<"LVL" << reg->reg5.VDS_LVL;

    set_GateDrivePosHiSide( get_bitfield(reg[HISIDE_REG], 4, 4));
    set_GateDriveNegHiSide( get_bitfield(reg[HISIDE_REG], 0, 4));
    set_GateDrivePosLoSide( get_bitfield(reg[LOSIDE_REG], 4, 4));
    set_GateDriveNegLoSide( get_bitfield(reg[LOSIDE_REG], 0, 4));
    set_TDrive(             get_bitfield(reg[TDRIVE_REG], 8 , 2));
    set_Deadtime(           get_bitfield(reg[DEAD_TIME_REG] , 8 , 2));
    set_OCP_DEG(            get_bitfield(reg[OCP_DEG_REG] , 6 , 2));
    set_VDS_LVL(            get_bitfield(reg[VDS_LVL_REG] , 0  , 4));
 }

void DRV8320S::set_statusRow(int row , quint16 val){
    if(row>1){
        qDebug() << "ERROR in set_statusRow";
        return;
    }
    for(int i=0; i<10 ; i++)
        set_status(row , i , (bool)(val>>i)&1);
}

void DRV8320S::set_status(int reg , int bit, bool status){
    if(reg==1)
        bit++;
    if(status)
        error_buttons[reg][9-bit]->setStyleSheet(stylesheet_error);
    else
        error_buttons[reg][9-bit]->setStyleSheet(stylesheet_ok);

    //error_buttons[1][0]->setStyleSheet(stylesheet_ok);



}

void DRV8320S::DRV_reset(bool state){
    emit send_DRV8320S(USBbulk::DRV_RESET , state);
}

void DRV8320S::slot_OCP_DEGChanged(int index){
    emit send_DRV8320S(USBbulk::DRV_OCP_DEG , index);
}

void DRV8320S::slot_DeadtimeChanged(int index){
    emit send_DRV8320S(USBbulk::DRV_DEADTIME , index);
}

void DRV8320S::slot_TdriveChanged(int index){
    emit send_DRV8320S(USBbulk::DRV_TDRIVE , index);
}

void DRV8320S::slot_VDSChanged(int index){
    emit send_DRV8320S(USBbulk::DRV_VDS_LVL , index);
}

void DRV8320S::slot_IDrive_P_HS(int index){
    emit send_DRV8320S(USBbulk::DRV_IDRIVE_P_HS , index);
}

void DRV8320S::slot_IDrive_N_HS(int index){
    emit send_DRV8320S(USBbulk::DRV_IDRIVE_N_HS , index);
}

void DRV8320S::slot_IDrive_P_LS(int index){
    emit send_DRV8320S(USBbulk::DRV_IDRIVE_P_LS , index);
}

void DRV8320S::slot_IDrive_N_LS(int index){
    emit send_DRV8320S(USBbulk::DRV_IDRIVE_N_LS , index);
}
