#ifndef PISECTION_H
#define PISECTION_H

#include <complex>
#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include "knob.h"
#include "calcfilt.h"

#define DEFAULT_GAIN 0
#define DEFAULT_FILTER PI

class PISection : public QWidget
{
    Q_OBJECT
public:
    explicit PISection(QWidget *parent = nullptr , int fs=0);
    ~PISection();
    void setSectionID(int newID);
    void setBoxTitle(const QString &title);
    void updateSettingsAndPlot(bool updatePlot, int new_fs);

    void updateLinked();
    double getFc();
    void setFc(double , bool);
    double getGain();
    void setGain(double , bool);
    bool getFilterActive();
    void setFilterActive(bool , bool);




signals:
    void PIchanged(double B[2] , double A);

private slots:
void slot_gainChanged(double gain);
void slot_fcChanged(double fc);
void slot_filtertypeChanged(int type);
void slot_activeEQChanged(bool state);

private:
 quint16 *port;
 int channelID;
 int sectionID;
 QVBoxLayout *topLayout;
 QBoxLayout *layout;
 Knob *knob_fc;
 Knob *knob_gain;
 QGroupBox *groupBox;
 float fs;
 PI_section_t PIsettings={DEFAULT_FC , DEFAULT_GAIN};
 double B[2];
 double A;
};



#endif // EQSECTION_H
