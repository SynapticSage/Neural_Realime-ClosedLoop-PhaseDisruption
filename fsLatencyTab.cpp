#include "fsLatencyTab.h"
#include "trodesSocket.h"

extern headerDisplayConfiguration *headerConf;


LatencyTab::LatencyTab (QWidget *parent)
    : QWidget(parent)
{

    QGridLayout *layout = new QGridLayout;

    // Select whether or not to do latency test
    latencyTestEnabled = new QCheckBox("Enable Latency Test", this);
    layout->addWidget(latencyTestEnabled, 1, 0, Qt::AlignCenter | Qt::AlignTop);
    //connect(latencyTestEnabled, SIGNAL(toggled(bool)), latencyTest, SLOT(setFilterEnabled(bool)));


    // latency test parameter settings
    paramGroupBox = new QGroupBox;
    paramGroupBox->setTitle("Latency Settings");
    QGridLayout *paramLayout = new QGridLayout;
    paramGroupBox->setLayout(paramLayout);
    paramGroupBox->setDisabled(true);
    connect(latencyTestEnabled, SIGNAL(toggled(bool)),
            this, SLOT(updateLatencyTestEnable()));


    internalLatencyConfigBox = new InternalLatencyConfigBox(this);
    paramLayout->addWidget(internalLatencyConfigBox,0, 0);
    // HPC latency test, requires cluster setup

    connect(this, SIGNAL(updateAllParam()), internalLatencyConfigBox, SLOT(updateInternalLatencyParam()));

    connect(internalLatencyConfigBox, SIGNAL(stateScriptUpdated(QString)), this,
                                             SLOT(updateMCUStateScript(QString)));

    connect(internalLatencyConfigBox, SIGNAL(paramUpdated(InternalLatencyParameters)), this,
            SLOT(setInternalLatencyParam(InternalLatencyParameters)));

    HPCTestButton = new QPushButton("HPC Latency");
    HPCTestButton->setCheckable(true);
    paramLayout->addWidget(HPCTestButton, 0, 1);

    HPCTestGroupBox = new QGroupBox;
    HPCTestGroupBox->setEnabled(false);
    QGridLayout *HPCTestGroupLayout = new QGridLayout;
    HPCTestGroupBox->setLayout(HPCTestGroupLayout);
    paramLayout->addWidget(HPCTestGroupBox, 1, 1);

    //connect(sendScriptButton, SIGNAL(clicked()), this, SLOT(updateAllParam()));

    // assuming guarenteed order of slot execution (QT4.6+), parameter updates must occur
    // before sending parameters to FSData.  Although all other GUI interactions should
    // automatically update local parameters anyways.
    //connect(sendScriptButton, SIGNAL(clicked()), this, SLOT(updateAllParam()));

    //TODO decide on how to generate state scripts to directly control MCU.  Maybe these
    // scripts are hardcoded in the future and not configured dynamically through FSGui based on custom
    // settings.
    //connect(sendScriptButton, SIGNAL(clicked()), this, SLOT(updateMCUStateScript()));

    //connect(sendScriptButton, SIGNAL(clicked()), this, SLOT(updateFSData()));

    // connect internal test button toggle to enable settings
    connect(HPCTestButton, SIGNAL(toggled(bool)), HPCTestGroupBox, SLOT(setEnabled(bool)));

    layout->addWidget(paramGroupBox, 2, 0, 4, 4, Qt::AlignLeft | Qt::AlignTop);

    connect(this, SIGNAL(paramUpdated()), this, SLOT(sendParamToFSData()));
    connect(HPCTestButton, SIGNAL(clicked(bool)), this, SLOT(sendParamToFSData()));


    latencyStatus = new QLabel("");

    statusGroupBox = new QGroupBox("Latency Test Disabled");
    statusGroupBox->setStyleSheet("QGroupBox{border: 1px solid black;border-radius: 5px; margin-top: 1ex;}" \
                                  "QGroupBox::title{subcontrol-origin: margin; subcontrol-position:top center; padding: 0 3px;}");

    QHBoxLayout *statusBoxLayout = new QHBoxLayout;
    statusBoxLayout->addWidget(latencyStatus, Qt::AlignCenter);
    statusGroupBox->setLayout(statusBoxLayout);
    layout->addWidget(statusGroupBox, 6, 0, 4, 4);

    startLatencyTestButton = new QPushButton("Start Test");
    layout->addWidget(startLatencyTestButton, 10, 0, Qt::AlignCenter);
    // click signal connected in spikeFSGUI
    startLatencyTestButton->setEnabled(false);
    // Assumes script order of execution for slots (QT4.6+)
    // Don't think this is true in QT 5.x
    //connect(startLatencyTestButton, SIGNAL(clicked()), this, SLOT(sendParamToFSData()));
    //connect(startLatencyTestButton, SIGNAL(clicked()), this, SLOT(updateFSData()));
    connect(startLatencyTestButton, SIGNAL(clicked()), this, SLOT(startLatencyTest()));

    stopLatencyTestButton = new QPushButton("Stop Test");
    layout->addWidget(stopLatencyTestButton, 10, 1, Qt::AlignCenter);
    // click signal connected in spikeFSGUI
    // disable until needed
    stopLatencyTestButton->setEnabled(false);
    connect(stopLatencyTestButton, SIGNAL(clicked()), this, SLOT(stopLatencyTest()));

    setLayout(layout);

    // update the Send Button to reflect the fact that the script has not been sent
    updateSendButton();

}

void LatencyTab::updateLatencyTestEnable() {
    bool enable = latencyTestEnabled->isChecked();
    paramGroupBox->setEnabled(enable);
    startLatencyTestButton->setEnabled(enable);
    internalLatencyConfigBox->enableInternalLatency(enable);
}

void LatencyTab::setStateScriptFunctionNumber(int fnNum) {
    // set the number for the go function.  The stop function is fnNum + 1
    internalLatencyConfigBox->setStateScriptFunctionNumber(fnNum);
}


void LatencyTab::updateSendButton() {
    //sendScriptButton->setText("Send script");
    //sendScriptButton->setStyleSheet("color: rgb(255, 0, 0)");
    startLatencyTestButton->setEnabled(false);
}


void LatencyTab::updateMCUStateScript(QString script) {
    qDebug() << "LatencyTab: Sending new State Script to MCU";
    QString directOutScript;
    //generateDirectTriggerScript(&directOutScript);

    emit newStateScript(&script);
    if (latencyTestEnabled->isChecked()) {
        startLatencyTestButton->setEnabled(true);
    }

    return;
}

void LatencyTab::setInternalLatencyParam(InternalLatencyParameters internalParam){
    latencyParam.internal = internalParam;
    emit paramUpdated();

}

void LatencyTab::showLatencyTestParam() {
    return;
}



void LatencyTab::updateHPCLatencyParam() {
    /** Sends latencyParam settings to FSData **/

    return ;
}

void LatencyTab::sendParamToFSData() {
    /** Updates all settings in latencyParam.
     * Does not send new param to FSData
     **/
    qDebug() << "LatencyTab: Updating all Latency Parameters";
    emit SendFSDataMessage(FS_SET_LATENCY_TEST_PARAMS, (char*) &latencyParam, sizeof(LatencyParameters));
    return ;
}

void LatencyTab::updateLatencyStatus(QString status) {
    latencyStatus->setText(status);
    return;
}


void LatencyTab::startLatencyTest() {
    /** Begin latency test, disable setting modifications, and notify FSGui
     * (and subsequently FSData and Trodes) to begin streaming data.
     */
    sendParamToFSData();
    startLatencyTestButton->setEnabled(false);
    stopLatencyTestButton->setEnabled(true);
    latencyTestEnabled->setEnabled(false);
    paramGroupBox->setEnabled(false);
    //emit startFSDataStream();
    emit SendFSDataMessage(FS_START_LATENCY_TEST, NULL, 0);
    statusGroupBox->setTitle("Latency Test Enabled");

    return;
}

void LatencyTab::stopLatencyTest() {
    startLatencyTestButton->setEnabled(true);
    stopLatencyTestButton->setEnabled(false);
    latencyTestEnabled->setEnabled(true);
    paramGroupBox->setEnabled(true);
    emit SendFSDataMessage(FS_STOP_LATENCY_TEST, NULL, 0);
    statusGroupBox->setTitle("Latency Test Disabled");
    return;
}


InternalLatencyConfigBox::InternalLatencyConfigBox(QWidget *parent)
    : QWidget(parent)
{

    QGridLayout *layout = new QGridLayout;


    // internal test: timed only on timestamp between packet from hardware
    // and raising an output bit
    // button to enable internal test
    internalTestButton = new QPushButton("Internal Latency");
    internalTestButton->setCheckable(true);
    // Auto select the internal timing test
    internalTestButton->setChecked(false);
    layout->addWidget(internalTestButton, 0, 0);

    connect(internalTestButton, SIGNAL(toggled(bool)), this, SLOT(enableInternalLatency(bool)));

    // internal test setting if selected
    internalTestGroupBox = new QGroupBox;
    internalTestGroupBox->setEnabled(internalTestButton->isChecked());
    QGridLayout *internalTestLayout = new QGridLayout;
    internalTestGroupBox->setLayout(internalTestLayout);
    layout->addWidget(internalTestGroupBox);


    internalTestLayout->addWidget(new QLabel("Test Interval (ms)"), 1, 0);
    testIntervalSpinBox = new QSpinBox;
    testIntervalSpinBox->setRange(10,10000);
    testIntervalSpinBox->setValue(100);
    internalTestLayout->addWidget(testIntervalSpinBox, 1, 1);

    internalTestLayout->addWidget(new QLabel("Digital Out Port"), 2, 0);
    //internalOutChanSpinBox = new DigitalIOSpinBox(this, false);
    internalOutChanSpinBox = new QSpinBox(this);
    internalOutChanSpinBox->setValue(1);

    internalTestLayout->addWidget(internalOutChanSpinBox, 2, 1);

    sendScriptButton = new QPushButton("Send Script");
    internalTestLayout->addWidget(sendScriptButton, 3, 0);

    connect(testIntervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateInternalLatencyParam()));
    connect(internalOutChanSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateInternalLatencyParam()));

    connect(sendScriptButton, SIGNAL(clicked()), this, SLOT(newStateScript()));

    connect(internalTestGroupBox,SIGNAL(toggled(bool)), this, SLOT(updateInternalLatencyParam()));

    setLayout(layout);

}

int InternalLatencyConfigBox::loadFromXML(QDomNode &stimConfNode) {

}

void InternalLatencyConfigBox::saveToXML(QDomDocument &doc, QDomElement &scstNode) {

}

void InternalLatencyConfigBox::setStateScriptFunctionNumber(unsigned short fnNum) {
    internalParam.stateScriptFnNum = fnNum;
    //internalParam.stateScriptFnNum = 1;

}

void InternalLatencyConfigBox::newStateScript() {
    emit stateScriptUpdated(generateDirectTriggerScript());
}

QString InternalLatencyConfigBox::generateDirectTriggerScript() {
    //Generate a script that raises and lowers the selected port for 1 ms.
    QString scriptString;
    QTextStream script(&scriptString, QIODevice::WriteOnly);
    script << "function " << internalParam.stateScriptFnNum << endl;
    script << "    portout[" << internalOutChanSpinBox->value() << "]=flip" << endl;
    script << "end;" << endl;

    /*
    script << "function " << internalParam.stateScriptFnNum << endl;
    script << "    portout[" << internalOutChanSpinBox->value() << "]=1" << endl;
    script << "    do in 1" << endl;
    script << "        portout[" << internalOutChanSpinBox->value() << "]=0" << endl;
    script << "    end" << endl;
    script << "end;" << endl;
    */

    return scriptString;
}

void InternalLatencyConfigBox::enableInternalLatency(bool pressed) {
    internalTestButton->setChecked(pressed);
    internalTestGroupBox->setEnabled(pressed);
    updateInternalLatencyParam();
    return ;
}

void InternalLatencyConfigBox::updateInternalLatencyParam() {
    // convert the test interval to timestamp units
    internalParam.enabled = internalTestButton->isChecked();
    internalParam.testInterval = (hardwareConf->sourceSamplingRate  * testIntervalSpinBox->value()) / 1000;
    internalParam.outputDIOPort = internalOutChanSpinBox->value();

    emit paramUpdated(internalParam);
    return ;
}
