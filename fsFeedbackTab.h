#ifndef __SPIKE_FS_REALTIME_FEEDBACK_H__
#define __SPIKE_FS_REALTIME_FEEDBACK_H__

#include <QtGui>
#include <QtWidgets>
#include <QtXml>
#include <tuple>

#include <fsSharedStimControlDefines.h>
#define DEFAULT_TMP_PULSE_COMMANDS_FILE "/tmp/tmp_pulse_commands"



class ThetaPhaseStim : public QWidget
{
    Q_OBJECT

public:
    ThetaPhaseStim (QWidget *parent);

//public slots:
//    void setFilterEnabled(bool);
};

class RippleFilter : public QDialog
{
    Q_OBJECT

public:
    RippleFilter (QWidget *parent);

    typedef std::tuple<QLabel *, QLineEdit *, QLineEdit *> QBaselineValueLine;

    QGroupBox *algorithmParametersGroupBox;
    QSpinBox *sampDivisor;
    QLineEdit *ripCoeff1;
    QLineEdit *ripCoeff2;
    QLineEdit *ripThresh;
    QSpinBox *nAboveThreshold;
    QSpinBox *lockoutTime;
    QCheckBox *detectNoRipples;
    QSpinBox *detectNoRippleTime;

    QButtonGroup *rippleValueUpdateButtonGroup;
    QPushButton *pullRippleValuesButton;
    //QPushButton *stopResetRippleValuesButton;
    QPushButton *pushRippleValuesButton;

    QGroupBox *baselineChannelValuesGroupBox;

    QWidget *baselineChannelValuesGroup;
    QGridLayout *baselineChannelValuesLayout;

    QMap<int, QBaselineValueLine> *baselineGroupMap;

    bool enabled;

    int loadFromXML(QDomElement &nt);
    void saveToXML(QDomDocument &doc, QDomElement &rootNode);


public slots:
    void setFilterEnabled(bool);
    void updateRippleData(void);
    void setBaselineValuesSelect(qint16 ind, bool selected);
    void setBaselineValuesListEditable(bool editable);
    void updateBaselineValuesListMean(double *means);
    void updateBaselineValuesListStd(double *stds);

    void updateBaselineValueMeanBuffer(int nTrodeID, QString mean);
    void updateBaselineValueStdBuffer(int nTrodeID, QString std);

    void setEnableCustomBaselineValues(bool enabled);



private slots:
    void updateCustomBaselineMean(void);
    void updateCustomBaselineStd(void);

signals:
    void SendFSDataMessage(int message, char *data, int size);

private:
    double *baselineValuesMeanBuffer;
    double *baselineValuesStdBuffer;
};

class SpatialFilter : public QDialog
{
    Q_OBJECT

public:
    SpatialFilter (QWidget *parent);
    QSpinBox *lowerLeftX;
    QSpinBox *lowerLeftY;
    QSpinBox *upperRightX;
    QSpinBox *upperRightY;
    QDoubleSpinBox *cmPerPix;
    QLineEdit *minSpeedThresh;
    QLineEdit *maxSpeedThresh;
    QSpinBox *lockoutTime;
    bool enabled;

    int loadFromXML(QDomElement &nt);
    void saveToXML(QDomDocument &doc, QDomElement &rootNode);



public slots:
    void setFilterEnabled(bool);
    void updateSpatialData(void);

signals:
    void SendFSDataMessage(int message, char *data, int size);
};

class PhaseFilter : public QDialog
{
    Q_OBJECT

public:
    PhaseFilter (QWidget *parent);

    /* -------------------------- */

    QGroupBox *filterParametersGroupBox;
    QGroupBox *targetParametersGroupBox;
    QGroupBox *enableDisableGroupBox;
    QGroupBox * smoothingParametersGroupBox;

    /* -------------------------- */

    QGridLayout *fParamLayout;
    QGridLayout *tParamLayout;
    QGridLayout *eParamLayout;
    QGridLayout *sParamLayout;

    /* -------------------------- */

    // Filter Parameters
    QComboBox *filterType;
    QSpinBox *filterOrder;

    QLineEdit *freqLow;
    QLineEdit *freqHigh;

    QSpinBox *bufferSize;
    QLabel *bufferSizeSuggest;

    // Smoothing Parameters
    QCheckBox *smoothingEnabled;
    QSpinBox  *smoothingSize;

    // Target Parameters
    QComboBox *targetPhase;
    QLineEdit *targetPower;
    QSpinBox  *delay;
    QSpinBox *lockoutTime;

    // Enable/Disable Parameters
    QSpinBox *nAtRequirement;
    QCheckBox *enablePhase;
    QCheckBox *enablePower;
    QCheckBox *enable;

    bool enabled;

//    int loadFromXML(QDomElement &nt);
//    void saveToXML(QDomDocument &doc, QDomElement &rootNode);



public slots:
    void setFilterEnabled(bool);
    void updateData(void);

signals:
    void SendFSDataMessage(int message, char *data, int size);
};


class FeedbackTab : public QWidget
{
    Q_OBJECT

public:
    FeedbackTab (QWidget *parent);
    int loadFromXML(QDomNode &feedbackConfNode);
    void saveToXML(QDomDocument &doc, QDomElement &rootNode);

    RippleFilter     *rippleFilt;
    SpatialFilter    *spatialFilt;
    PhaseFilter      *phaseFilt;

    /* For each type of filter, we have a checkbox to enable it and a button to launch
     * the configuration GUI */
    QCheckBox *rippleFilterEnabled;
    QPushButton *setRippleFilterParams;

    QCheckBox *spatialFilterEnabled;
    QPushButton *setSpatialFilterParams;

    QLabel *phaseFilterEnabled;
    QPushButton *setPhaseFilterParams;

    QCheckBox *gpFilterEnabled;
    QPushButton *setGpFilterParams;

    QLabel *digOutGateLabel;
    QSpinBox *digOutGatePort;



    QLabel *rippleStatus;
    QLabel *spatialStatus;
    QLabel *thetaStatus;
    QLabel *phaseStatus;



    //QLabel *latencyStatus;
    QGroupBox *statusGroupBox;


    QPushButton *resetFeedbackButton;
    QPushButton *startFeedbackButton;
    QPushButton *stopFeedbackButton;

public slots:
    void updateStimulatorStatus(bool stimReady);
    //void updateActiveAOut(int aOutIndex, int aOut1Mode, int aOut2Mode);
    void setFeedbackEnabled(void);
    void updateRippleStatus(QString);
    void updateSpatialStatus(QString);
    void updateThetaStatus(QString);
    void updatePhaseStatus(QString);
    void updateDigOutGate(int);
    void updateAllFilters(void);
    //void updateLatencyStatus(QString);
    //void updateLatencyPortA(int);
    //void updateLatencyPortB(int);

private slots:
    void showRippleFilter(void);
    void showSpatialFilter(void);
    void showPhaseFilter(void);
    //void showLatencyTest(void);

    void checkRealtimeStatus(void);
    void updateRealtimeStatus(char *s);

private:
    bool            stimulatorReady;
    bool            aOutReady;
    //LatencyTest      *latencyTest;

signals:
    void SendFSDataMessage(int message, char *data, int size);


protected:
};
#endif
