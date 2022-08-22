#ifndef FSLATENCYTAB_H
#define FSLATENCYTAB_H

#include <QtGui>
#include <QtXml>
#include <QtWidgets>
#include <QTextStream>
#include <fsSharedStimControlDefines.h>
#include <configuration.h>

class InternalLatencyConfigBox;

class LatencyTab : public QWidget
{
    Q_OBJECT

public:
    LatencyTab(QWidget *parent);
    int loadFromXML(QDomNode &stimConfNode);
    void saveToXML(QDomDocument &doc, QDomElement &scstNode);

    QCheckBox *latencyTestEnabled;
    QPushButton *openLatencyFileButton;

    QGroupBox *paramGroupBox;
    QButtonGroup *paramButtonGroup;

    InternalLatencyConfigBox *internalLatencyConfigBox;
    QPushButton *HPCTestButton;
    QGroupBox *HPCTestGroupBox;


    QLabel *latencyStatus;
    QGroupBox *statusGroupBox;

    QPushButton *startLatencyTestButton;
    QPushButton *stopLatencyTestButton;

    void setStateScriptFunctionNumber(int);

signals:
    void newStateScript(QString *);
    void SendFSDataMessage(int, char*, int);
    void startFSDataStream();
    void stopFSDataStream();
    void openLatencyFile();
    void paramUpdated();
    void updateAllParam();


public slots:
    void updateLatencyStatus(QString);

private slots:
    void updateLatencyTestEnable();
    void setInternalLatencyParam(InternalLatencyParameters);
    void showLatencyTestParam(void);
    void updateHPCLatencyParam(void);
    void sendParamToFSData(void);
    void updateMCUStateScript(QString);
    void startLatencyTest();
    void stopLatencyTest();
    void updateSendButton();


private:

    QFile *latencyFile;

    LatencyParameters latencyParam;
};

class InternalLatencyConfigBox : public QWidget
{
    Q_OBJECT
public:
    InternalLatencyConfigBox(QWidget *parent);
    int loadFromXML(QDomNode &stimConfNode);
    void saveToXML(QDomDocument &doc, QDomElement &scstNode);

    QPushButton *internalTestButton;

    QGroupBox *internalTestGroupBox;
    QSpinBox *internalOutChanSpinBox;
    QSpinBox *testIntervalSpinBox;
    QPushButton *sendScriptButton;

    void setStateScriptFunctionNumber(unsigned short fnNum);

signals:
    void stateScriptUpdated(QString);
    void paramUpdated(InternalLatencyParameters);

public slots:
    void enableInternalLatency(bool);
    void updateInternalLatencyParam(void);
    void newStateScript();

private:
    // Statescript function that provides direct write to
    // digital output state
    QString generateDirectTriggerScript();
    InternalLatencyParameters internalParam;
};


#endif // FSLATENCYTAB_H
