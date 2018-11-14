#ifndef EQSECTION_H
#define EQSECTION_H

#include <complex>
#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include "knob.h"
#include "calcfilt.h"

#define DEFAULT_Q 0.71
#define DEFAULT_GAIN 0
#define DEFAULT_FILTER PI

class EQsection : public QWidget
{
    Q_OBJECT
public:
    enum FilterType{PI , LowShelf , HighSelf , PeakingEQ , Notch , AllPass , HighPass , LowPass , BandPass };
    explicit EQsection(QWidget *parent = nullptr , int fs=0);
    ~EQsection();
    void setSectionID(int newID);
    void setBoxTitle(const QString &title);
    void updateSettingsAndPlot(bool updatePlot, int new_fs);

    double getFc();
    void setFc(double , bool);
    double getQ();
    void setQ(double , bool);
    double getGain();
    void setGain(double , bool);
    filterType_t getFilterType();
    void setFilterType(filterType_t , bool);
    bool getFilterActive();
    void setFilterActive(bool , bool);

    double B[3];
    double A[2];


signals:
    void eqchanged();

private slots:
void slot_gainChanged(double gain);
void slot_Q_Changed(double Q);
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
 Knob *knob_Q;
 Knob *knob_gain;
 QGroupBox *groupBox;
 QComboBox *filterType;
 float fs;
};



#endif // EQSECTION_H
