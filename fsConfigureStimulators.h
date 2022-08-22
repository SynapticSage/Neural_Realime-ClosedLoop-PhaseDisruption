#ifndef __SPIKE_FS_CONFIGURE_STIMULATORS_H__
#define __SPIKE_FS_CONFIGURE_STIMULATORS_H__

#include <QtGui>
#include <QtWidgets>
#include <QtXml>
#include <fsSharedStimControlDefines.h>

class StimConfigureWidget : public QWidget
{
    Q_OBJECT

public:
    StimConfigureWidget(QWidget *parent = 0);
    int loadFromXML(QDomElement &sConf);
    void saveToXML(QDomDocument &doc, QDomElement &rootNode);

    PulseCommand stimPulseCmd;
    void setScriptFunctionNum(uint16_t num);
    uint16_t getScriptFunctionNum(void);
    bool isHardwareReady(void);

    inline void setID(int newID) {originID = newID;};


public slots:
    void frequencyChanged(void);
    void periodChanged(void);
    void ablePulseSequence(void);

    void ableBiphasicStimulation(int);

    void updateStimParameters(void);
    void sendScript(void);


private:
    int originID;
    uint16_t scriptFunctionNum;
    bool hardwareReady;

    QDoubleSpinBox *pulseLengthSpinBox;
    QSpinBox *nPulsesSpinBox;
    QSpinBox *preDelaySpinBox;
    QGroupBox *multiPulseGroup;
    QDoubleSpinBox *sequenceFrequencySpinBox;
    QDoubleSpinBox *sequencePeriodSpinBox;

    QGroupBox *pulseTrainGroup;
    QSpinBox *trainIntervalSpinBox;
    QSpinBox *nTrainsSpinBox;

    QSpinBox *primaryStimPinSpinBox;
    QSpinBox *secondaryStimPinSpinBox;
    QLabel *secondaryStimPinLabel;
    QCheckBox *biphasicCheckBox;

    QLabel      *scriptStatus;
    QPushButton *sendScriptButton;

    QLabel      *labelAutoSettle;
    QCheckBox   *setAutoSettle;

    void generateStateScript(int functionNum, QString *scriptString);

private slots:
    void autoSettleToggled(bool turnOn);

signals:
    void newStateScript(QString *script);
    void stimulatorReadyChanged(void);
    void stimulatorPortSelected(int);
    void sig_setAutoSettle(bool autoSettle, int originID);
};


class StimConfigTab : public QWidget
{
    Q_OBJECT

public:
    StimConfigTab(QWidget *parent = 0);
    int loadFromXML(QDomNode &stimConfNode);
    void saveToXML(QDomDocument &doc, QDomElement &scstNode);
    int activeStimulator;
    StimConfigureWidget *stimConfigA;
    StimConfigureWidget *stimConfigB;
    QPushButton *stimulatorAButton;
    QPushButton *stimulatorBButton;


public slots:
    void updateStimulatorStatus(void);

signals:
    void stimulatorsReady(bool);
    void newStateScript(QString *script);
    void stateScriptFunctionValid(uint16_t, bool);
    void sig_setAutoSettle(bool autoSettle, int originID);


};
#endif
