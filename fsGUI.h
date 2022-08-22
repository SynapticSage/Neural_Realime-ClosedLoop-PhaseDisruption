#ifndef __SPIKE_FS_GUI_H__
#define __SPIKE_FS_GUI_H__

#include <QtGui>
#include <QtWidgets>
#include <QtXml>

#include <QMainWindow>


#include <inttypes.h>
#include <configuration.h>
#include <trodesSocket.h>



#include "fsSharedStimControlDefines.h"
#include "fsConfigureStimulators.h"
#include "fsConfigureAOut.h"
#include "fsPulseTrainTab.h"
#include "fsFeedbackTab.h"
#include "fsLatencyTab.h"
#include "fsStimForm.h"

#define PI     3.1415
#define TWOPI  6.2830
#define MAX_WELLS	64
#define MAX_REST	90000	
#define MAX_REWARD_BITS	32	

#define DEFAULT_TMP_PULSE_COMMANDS_FILE "/tmp/tmp_pulse_commands"
#define CONFIG_STIMULATORS_TAB 1
#define CONFIG_ANALOG_OUT_TAB 2
#define OUTPUT_ONLY_TAB 3
#define REALTIME_FEEDBACK_TAB 4

#define SAMP_TO_TIMESTAMP 3

class PythonConfig {
public:
    bool pythonFSData = false;

    int loadFromXML(QDomNode &pythonNode);
    void saveToXML(QDomDocument &doc, QDomElement &rootNode);
};



class HPCConfig {
public:
    bool hpcEnabled = false;
    qint16 dataType; // the datatypes to send to the HPC framework

    int loadFromXML(QDomNode &hpcConfNode);
    void saveToXML(QDomDocument &doc, QDomElement &rootNode);
};


class FSGui : public QMainWindow {
	Q_OBJECT

public:
    FSGui(QStringList arguments, QWidget *parent = 0);
    ~FSGui();
    int loadFromXML(QDomNode &fsConfNode);
    void saveToXML(QDomDocument &doc, QDomElement &rootNode);


private slots:
    void about();
    int   readConfigFile(QString);
    int   sendConfigFile(QString);

    bool  writeConfigFile(QString);
    void  enableTabs(bool);
    void  loadSettings(void);
    void  saveSettings(void);

    void setupFile(QString filename);
    void startRecording();
    void stopRecording();
    void closeFile();

    void triggerSingleStim(void);
    void startOutputOnlyStim(void);
    void abortOutputOnlyStim(void);

    void resetRealtimeStim(void);
    void startRealtimeStim(void);
    void stopRealtimeStim(void);


    void startTrodesClient(void);
    void startDataSocket(DataClientInfo *dc);
    void startDataSockets(DataTypeSpec *dataTypeSpec, int dataType);

    void updateContNTrodeSelection(qint16 nTrodeIndex, bool selected);
    void updateSpikeNTrodeSelection(qint16 nTrodeIndex, bool selected);

    void startFSData(void);
    void startData(void);
    void stopData(void);

    void SendFSDataMessage(int message, char *data, int size);
    void setupFSDataMessageHandler();
    void processFSDataMessage(quint8 messageType, QByteArray message);

    void setStateScriptFunctionRange(int start, int end);

    void setStateScriptFunctionValid(uint16_t fNum, bool valid);

    void openLatencyDataFile();
    void closeLatencyDataFile();

    void updateAutoSettleSetting(bool autoSettle, int originID);


private:

    QString     configFileName;
    QString     fsDataPath;
    QGridLayout *mainLayout;
    QMenu		*fileMenu;
    QAction		*loadFileAction;
    QAction		*saveFileAction;
    QAction     *openLatencyFileAction;
    QAction     *closeLatencyFileAction;
    QAction		*exitAction;

    QMenu       *menuHelp;
    QAction     *actionAbout;

    QFile       latencyFile;
    QTextStream latencyStream;


    bool trodes;
    bool trodesClientStarted;
    bool fsDataRestarted;
    bool dataStreaming;

    TrodesModuleNetwork   *moduleNet;
    TrodesServer *fsDataServer;

    /** Disabling data menu selection of channels to stream to FSData.
     * Instead each realtime module should configure it's own channel settings.
     */
    QMenu       *dataMenu;
    QAction     *restartFSDataAction;
    QAction     *selectContinuousDataAction;
    QAction     *selectSpikeDataAction;
    QAction     *startDataAction;
    QAction     *stopDataAction;

    QProcess    *fsDataProcess;
    NTrodeSelectDialog *contDataDialog;
    NTrodeSelectDialog *spikeDataDialog;

    PythonConfig    pythonConfig;
    HPCConfig   hpcConfig;  // configuration information for real-time high performance computing

    //int getStateScriptFnNum(void);

    qint8     dataTypesConnected;
    int     numContNTrodesConnected;
    int     numSpikeNTrodesConnected;

    //int directDIOFuncNum;
    // Setup default FSGui state script providing
    // direct control of DIO ports.
    //void generateDirectDIOScript(int, QString *);


protected:
    int         	ntabs;

    QTabWidget *qtab;

    StimConfigTab *stimConfigTab;
    AOutConfigTab *aOutConfigTab;
    //PulseFileTab *pulseFileTabWidget;
    StimPulseTrainTab *stimPulseTrainTab;
    FeedbackTab *feedbackTab;
    LatencyTab *latencyTab;

signals:
    void contDataStarted();
    void sig_fsDataRestarted();
};




#endif
