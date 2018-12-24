#ifndef DRV8320S_H
#define DRV8320S_H

#include <QWidget>
#include <QGridLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QFont>
#include <QDebug>
#include <QPushButton>
#include <QToolTip>


struct reg2_t{
    quint16 _RES:6;
    quint16 CLR_FLT:1;
    quint16 BRAKE:1;
    quint16 COAST:1;
    quint16 PWM_DIR:1;
    quint16 PWM_COM :1;
    quint16 PWM_MODE:2;
    quint16 OTW_REP:1;
    quint16 DIS_GDF:1;
    quint16 DIS_CPUV:1;
};

struct reg3_t{
    quint16 _RES:5;
    quint16 LOCK :3;
    quint16 IDRIVEN_HS:4;
    quint16 IDRIVEP_HS:4;

};

struct reg4_t{
    quint16 _RES:5;
    quint16 CBC:1;
    quint16 TDRIVE:2;
    quint16 IDRIVEP_LS:4;
    quint16 IDRIVEN_LS:4;


};

struct reg5_t{   
    quint16 TRETRY:1;
    quint16 DEAD_TIME:2;
    quint16 OCP_MODE:2;
    quint16 OCP_DEG:2;
    quint16 VDS_LVL:4;
    quint16 _RES:5;
};


struct reg_t{
    quint16 reg0;
    quint16 reg1;
    reg2_t reg2;
    reg3_t reg3;
    reg4_t reg4;
    reg5_t reg5;
};


struct box_t{
   QComboBox* comboBox;
   QLabel* label;

};

struct DRV8320S_tdrive_t{
    box_t combo;
    QVBoxLayout* layout;
    QGroupBox* box;
    const unsigned short delay[4]={500 , 1000 , 2000 , 4000};
};

struct DRV8320S_ODT_t{
    box_t combo;
    QVBoxLayout* layout;
    QGroupBox* box;
    const unsigned short delay[4]={2 , 4 , 6 , 8};
};

// Overcurrent deglitch time
struct DRV8320S_VDS_t{
    box_t combo;
    QVBoxLayout* layout;
    QGroupBox* box;
    const double voltage[16]={
                0.06 , 0.13 , 0.2 , 0.26,
                0.31 , 0.45 , 0.53 , 0.6,
                0.68 , 0.75 , 0.94 , 1.13,
                1.3 , 1.5 , 1.7 , 1.88
    };
};



struct DRV8320S_idrive_t{
    QGridLayout* layout;
    QGroupBox* box;
    box_t POS_HS;
    box_t NEG_HS;
    box_t POS_LS;
    box_t NEG_LS;
    const unsigned short current[16]={10, 30, 60, 80, 120 , 140 , 170 , 190 , 260 , 330 , 370 , 440 , 570 , 680 , 820 , 1000 };
};

enum gate{POS_HS , NEG_HS , POS_LS , NEG_LS};

void addIdrive(box_t *gate , QGridLayout *layout, unsigned short current[16] , int col);


class DRV8320S : public QWidget
{
    Q_OBJECT
public:
    explicit DRV8320S(QWidget *parent = nullptr );
    DRV8320S_idrive_t iDrive;
    DRV8320S_tdrive_t tDrive;
    DRV8320S_ODT_t ODT;
    DRV8320S_VDS_t VDS;
    QGridLayout* masterLayout;
    QGroupBox* masterBox;
    QString reg_tooltip[2][10]={{
       "VDS monitor overcurrent fault condition",
       "Gate drive fault condition",
       "Undervoltage lockout fault condition",
       "Overtemperature shutdown",
       "VDS overcurrent fault on the A high-side MOSFET",
       "VDS overcurrent fault on the A low-side MOSFET",
        "VDS overcurrent fault on the B high-side MOSFET",
        "VDS overcurrent fault on the B low-side MOSFET",
        "VDS overcurrent fault on the C high-side MOSFET",
        "VDS overcurrent fault on the C low-side MOSFET"
        },{
        "Enabled",
        "Indicates overtemperature warning",
        "Charge pump undervoltage fault condition",
        "Gate drive fault on the A high-side MOSFET",
        "Gate drive fault on the A low-side MOSFET",
        "Gate drive fault on the B high-side MOSFET",
        "Gate drive fault on the B low-side MOSFET",
        "Gate drive fault on the C high-side MOSFET",
        "Gate drive fault on the C low-side MOSFET",
        "Reset error"
        }};

    QString reg_text[2][10]={{
       "VDS OCP",
       "GDF",
       "UVLO",
       "OTSD",
       "VDS HA",
       "VDS LA",
       "VDS HB",
       "VDS LB",
       "VDS HC",
       "VDS LC"
    },{
        "ENA",
        "OTW",
        "CPUV",
        "VGS HA",
        "VGS LA",
        "VGS HB",
        "VGS LB",
        "VGS HC",
        "VGS LC",
        "Reset"
  }};




private slots:
    void slot_VDSChanged(int);
    void slot_ODTChanged(int);
    void slot_TdriveChanged(int);
    void slot_IDrive_P_HS(int);
    void slot_IDrive_N_HS(int);
    void slot_IDrive_P_LS(int);
    void slot_IDrive_N_LS(int);
    void DRV_reset(bool state);

private:
    QPushButton* error_buttons[2][10];
    QPushButton* reset_button = nullptr;
    QString stylesheet_ok=     "QPushButton{background-color:green ;color:white} ; QToolTip { color: #ffffff }";
    QString stylesheet_error=  "QPushButton{background-color:red ;color:black} ; QToolTip { color: #ffffff }";
    QString stylesheet_reset=  "QPushButton{background-color:blue ;color:white} ; QToolTip { color: #ffffff }";

signals:
    void DRV_reset_signal();
    void send_DRV8320S(int command , int index);

public slots:
    //void DRVerror_changed(short* data);
    void set_GateDriveNegHiSide(int index);
    void set_GateDrivePosHiSide(int index);
    void set_GateDriveNegLoSide(int index);
    void set_GateDrivePosLoSide(int index);
    void set_TDrive(int index);
    void set_VDS_LVL(int index);
    void set_ODT(int index);
    void set_status(int reg , int bit, bool status);
    void set_statusRow(int row , quint16 val);
    void decode_DRVregs(void *regptr);
};

#endif // DRV8320S_H
