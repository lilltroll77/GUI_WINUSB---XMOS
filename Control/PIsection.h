#ifndef PISECTION_H
#define PISECTION_H

#include <complex>
#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include "knob.h"
#include "calcfilt.h"

#define DEFAULT_GAIN -50
#define DEFAULT_FILTER PI

class PISection : public QWidget
{
    Q_OBJECT
public:
    explicit PISection(QWidget *parent = nullptr , int ID=0);
    ~PISection();
    void setSectionID(int newID);
    void setBoxTitle(const QString &title);
    void updateSettingsAndPlot(bool updatePlot);

    void updateLinked();
    double getFc();
    void setFc(double , bool);
    double getGain();
    void setGain(double , bool);
    bool getFilterActive();
    void setFilterActive(bool , bool);

signals:
    void PIchanged(double B[3] , double A[2] , int channel);
    void sendPIsettings(PI_section_t &PIsection , int channel);

private slots:
void slot_gainChanged(double gain);
void slot_fcChanged(double fc);
void slot_filtertypeChanged(int type);
void slot_activeEQChanged(bool state);

private:
 quint16 *port;
 int channelID;
 QVBoxLayout *topLayout;
 QBoxLayout *layout;
 Knob *knob_fc;
 Knob *knob_gain;
 QGroupBox *groupBox;
 PI_section_t PIsettings={DEFAULT_FC , DEFAULT_GAIN};
 double B[3];
 double A[2];
};



#endif // EQSECTION_H
