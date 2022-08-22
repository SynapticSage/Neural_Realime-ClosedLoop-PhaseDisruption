#include "fsGUI.h"
#include "fsFeedbackTab.h"
#include <QFormLayout>

FeedbackTab::FeedbackTab (QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *layout = new QGridLayout;

    // create the ripple and spatial filters.  Note that these are not shown until the buttons for them are pressed
    rippleFilt = new RippleFilter(this);
    connect(rippleFilt, SIGNAL(SendFSDataMessage(int, char*, int)), this, SIGNAL(SendFSDataMessage(int, char*, int)));

    spatialFilt = new SpatialFilter(this);
    connect(spatialFilt, SIGNAL(SendFSDataMessage(int, char*, int)), this, SIGNAL(SendFSDataMessage(int, char*, int)));

    phaseFilt = new PhaseFilter(this);
    connect(phaseFilt,SIGNAL(SendFSDataMessage(int,char*,int)), this, SIGNAL(SendFSDataMessage(int, char*, int)));

    //latencyTest = new LatencyTest(this);
    //connect(latencyTest, SIGNAL(SendFSDataMessage(int, char*, int)), this, SIGNAL(SendFSDataMessage(int, char*, int)));

    rippleFilterEnabled = new QCheckBox("Enable Ripple Filter", this);
    layout->addWidget(rippleFilterEnabled, 1, 0, Qt::AlignCenter | Qt::AlignVCenter);
    connect(rippleFilterEnabled, SIGNAL(toggled(bool)), rippleFilt, SLOT(setFilterEnabled(bool)));


    setRippleFilterParams = new QPushButton("Show Ripple Filter Parameters", this);
    layout->addWidget(setRippleFilterParams, 1, 1, Qt::AlignCenter | Qt::AlignVCenter);
    connect(setRippleFilterParams, SIGNAL(pressed()), this, SLOT(showRippleFilter()));

    spatialFilterEnabled = new QCheckBox("Enable Spatial Filter", this);
    layout->addWidget(spatialFilterEnabled, 2, 0, Qt::AlignCenter | Qt::AlignVCenter);
    connect(spatialFilterEnabled, SIGNAL(toggled(bool)), spatialFilt, SLOT(setFilterEnabled(bool)));

    setSpatialFilterParams = new QPushButton("Show Spatial Filter Parameters", this);
    layout->addWidget(setSpatialFilterParams, 2, 1, Qt::AlignCenter | Qt::AlignVCenter);
    connect(setSpatialFilterParams, SIGNAL(pressed()), this, SLOT(showSpatialFilter()));

//    phaseFilterEnabled = new QLabel("<font color=grey>Theta Phase Filter",this);
//    layout->addWidget(phaseFilterEnabled,3,0,Qt::AlignCenter | Qt::AlignVCenter );

//    setPhaseFilterParams = new QPushButton("Show Theta Phase Parameters",this);
//    layout->addWidget(setPhaseFilterParams,3,1,Qt::AlignCenter | Qt::AlignVCenter);

    gpFilterEnabled = new QCheckBox("General Phase Filter",this);
    layout->addWidget(gpFilterEnabled,4,0,Qt::AlignCenter | Qt::AlignVCenter );
    connect(gpFilterEnabled,SIGNAL(toggled(bool)),phaseFilt,SLOT(setFilterEnabled(bool)));

    setGpFilterParams = new QPushButton("Show General Phase Parameters", this);\
    layout->addWidget(setGpFilterParams,4,1,Qt::AlignCenter | Qt::AlignVCenter);
    connect(setGpFilterParams, SIGNAL(pressed()), this, SLOT(showPhaseFilter()));

    digOutGateLabel = new QLabel("Digital Out Gate Port", this);
    layout->addWidget(digOutGateLabel, 5,0, Qt::AlignCenter | Qt::AlignVCenter);


    digOutGatePort = new QSpinBox();
    digOutGatePort->setAlignment(Qt::AlignRight);
    digOutGatePort->setRange(0, headerConf->maxDigitalPort(false));
    digOutGatePort->setValue(0);
    digOutGatePort->setSpecialValueText("Off");
    digOutGatePort->setToolTip("Output port to gate stimulation; port high stops stim.");
    layout->addWidget(digOutGatePort, 5,1, Qt::AlignCenter | Qt::AlignVCenter);
    connect(digOutGatePort, SIGNAL(valueChanged(int)), this, SLOT(updateDigOutGate(int)));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkRealtimeStatus()));
    timer->start(500);

    /* create boxes for the status of each filter */
    rippleStatus = new QLabel("Ripple Filter Disabled");
    spatialStatus = new QLabel("Spatial Filter Disabled");
    thetaStatus = new QLabel("Theta Filter Disabled");
    phaseStatus = new QLabel("GP Filter Disabled");
    //latencyStatus = new QLabel("Latency Test Disabled");
    statusGroupBox = new QGroupBox("Stimulation Status");
    statusGroupBox->setStyleSheet("QGroupBox{border: 1px solid black;border-radius: 5px; margin-top: 1ex;}" \
                                  "QGroupBox::title{subcontrol-origin: margin; subcontrol-position:top center; padding: 0 3px;}");
    QHBoxLayout *statusBoxLayout = new QHBoxLayout;
    statusBoxLayout->addWidget(rippleStatus, Qt::AlignCenter);
    statusBoxLayout->addWidget(spatialStatus, Qt::AlignCenter);
    //statusBoxLayout->addWidget(thetaStatus, Qt::AlignCenter);
    statusBoxLayout->addWidget(phaseStatus,Qt::AlignCenter);
    //statusBoxLayout->addWidget(latencyStatus, Qt::AlignCenter);

    statusGroupBox->setLayout(statusBoxLayout);
    layout->addWidget(statusGroupBox, 6, 0, 2, 2);

    resetFeedbackButton = new QPushButton("Reset Feedback");
    layout->addWidget(resetFeedbackButton, 7, 0, 1, 2, Qt::AlignCenter);
    // click signal connected in spikeFSGUI

    startFeedbackButton = new QPushButton("Start Feedback");
    startFeedbackButton->setEnabled(true);

    layout->addWidget(startFeedbackButton, 8, 0, Qt::AlignCenter);
    //connect(startFeedbackButton, SIGNAL(clicked(bool)), rippleFilterEnabled, SLOT(setDisabled(bool)));
    //connect(startFeedbackButton, SIGNAL(clicked(bool)), spatialFilterEnabled, SLOT(setDisabled(bool)));
    // click signal connected to other objects in spikeFSGUI

    connect(startFeedbackButton, SIGNAL(clicked(bool)), digOutGatePort, SLOT(setEnabled(bool)));
    connect(startFeedbackButton, SIGNAL(clicked(bool)), rippleFilt, SLOT(setEnabled(bool)));
    connect(startFeedbackButton, SIGNAL(clicked(bool)), spatialFilt, SLOT(setEnabled(bool)));
    connect(startFeedbackButton, SIGNAL(clicked(bool)), phaseFilt, SLOT(setEnabled(bool)));

    //connect(startFeedbackButton, SIGNAL(clicked(bool)), setRippleFilterParams, SLOT(setDisabled(bool)));


    stopFeedbackButton = new QPushButton("Stop Feedback");
    layout->addWidget(stopFeedbackButton, 8, 1, Qt::AlignCenter);
    // click signal connected in spikeFSGUI
    // disable until needed
    stopFeedbackButton->setEnabled(false);
    connect(stopFeedbackButton, SIGNAL(clicked(bool)), rippleFilt, SLOT(setDisabled(bool)));
    connect(stopFeedbackButton, SIGNAL(clicked(bool)), spatialFilt, SLOT(setDisabled(bool)));
    connect(stopFeedbackButton, SIGNAL(clicked(bool)), digOutGatePort, SLOT(setDisabled(bool)));
    connect(startFeedbackButton, SIGNAL(clicked(bool)), phaseFilt, SLOT(setDisabled(bool)));
    //connect(stopFeedbackButton, SIGNAL(clicked(bool)), setRippleFilterParams, SLOT(setEnabled(bool)));


    stimulatorReady = false;
    aOutReady = false;
    setFeedbackEnabled();

    setLayout(layout);
}

// Right now this is never called.
void FeedbackTab::updateStimulatorStatus(bool stimReady) {
    stimulatorReady = stimReady;
    setFeedbackEnabled();
}

void FeedbackTab::updateDigOutGate(int port) {
    // send a message to FSData with the new status
    SendFSDataMessage(FS_DIG_OUT_GATE_STATUS, (char *) &port, sizeof(int));

}

/*
void FeedbackTab::updateLatencyPortA(int portNum) {
    latencyTest->stimAPort = portNum;
}

void FeedbackTab::updateLatencyPortB(int portNum) {
    latencyTest->stimBPort = portNum;
}
*/

void FeedbackTab::setFeedbackEnabled() {
    if (stimulatorReady || aOutReady) {
        startFeedbackButton->setEnabled(true);
    }
    else {
        startFeedbackButton->setEnabled(false);
    }
}

void FeedbackTab::updateAllFilters() {
    // set the status of all filters to update the parameters of any filters that are enabled.
    // This is called if we restart FSData
    qDebug() << "Updating all filters";
    rippleFilt->updateRippleData();
    spatialFilt->updateSpatialData();
    phaseFilt->updateData();
}

void FeedbackTab::updateRippleStatus(QString status) {
    rippleStatus->setText(status);
}

void FeedbackTab::updateSpatialStatus(QString status) {
    spatialStatus->setText(status);
}

void FeedbackTab::updateThetaStatus(QString status) {
    thetaStatus->setText(status);
}

void FeedbackTab::updatePhaseStatus(QString status)
{
    phaseStatus->setText(status);
}

/*
void FeedbackTab::updateLatencyStatus(QString status) {
    latencyStatus->setText(status);
}
*/

void FeedbackTab::showRippleFilter(void)
{
    if (!rippleFilt->isVisible()) {
        rippleFilt->setVisible(true);
    }
    rippleFilt->raise();
}

void FeedbackTab::showSpatialFilter(void)
{
    if (!spatialFilt->isVisible()) {
        spatialFilt->setVisible(true);
    }
    spatialFilt->raise();
}

/*
void FeedbackTab::showLatencyTest(void)
{
    if (!latencyTest->isVisible()) {
        latencyTest->setVisible(true);
    }
    latencyTest->raise();
}
*/

void FeedbackTab::showPhaseFilter(void)
{
    if (!phaseFilt->isVisible()) {
        phaseFilt->setVisible(true);
    }
    phaseFilt->raise();
}

void FeedbackTab::checkRealtimeStatus(void)
/* check the status of real time processing */
{
    /* query spike_fsdata to get the status of ripple disruption */
    SendFSDataMessage(FS_QUERY_RT_FEEDBACK_STATUS, NULL, 0);

    return;
}

void FeedbackTab::updateRealtimeStatus(char *s)
{
    //status->setText(QString(s));
}

void FeedbackTab::saveToXML(QDomDocument &doc, QDomElement &rootNode)
{
    QDomElement rtConf = doc.createElement("feedbackConfiguration");

    rootNode.appendChild(rtConf);

    rtConf.setAttribute("rippleFilterEnabled", rippleFilterEnabled->isChecked());
    rtConf.setAttribute("spatialFilterEnabled", spatialFilterEnabled->isChecked());



    rippleFilt->saveToXML(doc, rtConf);
    spatialFilt->saveToXML(doc, rtConf);
}

int FeedbackTab::loadFromXML(QDomNode &rtNode)
{
    int error;
    QDomElement nt;

    rippleFilterEnabled->setChecked((bool)rtNode.toElement().attribute("rippleFilterEnabled").toInt());
    spatialFilterEnabled->setChecked((bool)rtNode.toElement().attribute("spatialFilterEnabled").toInt());

    QDomNodeList nList = rtNode.childNodes();
    for (int i = 0; i < nList.length(); i++) {
        nt = nList.at(i).toElement();
        if (nList.at(i).nodeName() == "rippleConfiguration") {
            error = rippleFilt->loadFromXML(nt);
        }
        else if (nList.at(i).nodeName() == "spatialConfiguration") {
            error = spatialFilt->loadFromXML(nt);
        }
        //else if (nList.at(i).nodeName() == "latencyTestConfiguration") {
        //    latencyTest->loadFromXML(nt);
        //}
        else {
            qDebug() << "Unknown configuration type within feedbackConfiguration";
            return -1;
        }
        if(error < 0) {
            return error;
        }
    }
    return 1;
}

/*
LatencyTest::LatencyTest(QWidget *parent)
    : QDialog(parent)
{
    QGroupBox *algorithmParametersGroupBox = new QGroupBox("Latency Test Parameters");
    QFormLayout *parametersLayout = new QFormLayout;

    //threshold = new QSpinBox;
    //threshold->setRange(0, 65535);
    //threshold->setValue(65535);
    //threshold->setAlignment(Qt::AlignRight);
    //parametersLayout->addRow("Threshold", threshold);

    //connect(threshold, SIGNAL(valueChanged(int)), this, SLOT(updateLatencyData(void)));

    algorithmParametersGroupBox->setLayout(parametersLayout);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(algorithmParametersGroupBox, Qt::AlignCenter);
    setLayout(layout);
    show();
}

void LatencyTest::setFilterEnabled(bool value)
{
    enabled = value;
    updateLatencyData();
}


void LatencyTest::updateLatencyData(void)
{
    LatencyTestParameters data;

    data.enabled = enabled;
    //data.threshold = threshold->value();

    data.portA = stimAPort;
    data.portB = stimBPort;

    emit SendFSDataMessage(FS_SET_LATENCY_TEST_PARAMS, (char*)&data, sizeof(LatencyTestParameters));
}

void LatencyTest::saveToXML(QDomDocument &doc, QDomElement &rtConf)
{
    QDomElement nt = doc.createElement("latencyTestConfiguration");

    //nt.setAttribute("threshold", threshold->value());
    rtConf.appendChild(nt);

}

int LatencyTest::loadFromXML(QDomElement &nt)
{
    //threshold->setValue(nt.attribute("threshold").toInt());

    return 1;
}
*/


ThetaPhaseStim::ThetaPhaseStim(QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *algorithmParametersGroupBox = new QGroupBox("Theta Phase Stimulation Parameters");
    QFormLayout *parametersLayout = new QFormLayout;

    QComboBox *StimChan = new QComboBox(this);

    //StimChan->insertItems(0, *(daq_io_widget->ChannelStrings));
    parametersLayout->addRow("Tetrode / channel", StimChan);

    QLineEdit *speedThresh = new QLineEdit(QString::number(FS_RT_DEFAULT_THETA_VEL), this);
    speedThresh->setValidator(new QDoubleValidator(0.0, 100.0, 2, this));
    speedThresh->setAlignment(Qt::AlignRight);
    parametersLayout->addRow("Speed threshold (cm/s)", speedThresh);

    QSpinBox *filterDelay = new QSpinBox;
    filterDelay->setRange(0, 20000);
    filterDelay->setValue(FS_RT_DEFAULT_THETA_FILTER_DELAY);
    filterDelay->setAlignment(Qt::AlignRight);
    parametersLayout->addRow("Filter Delay (msec)", filterDelay);

    QSpinBox *thetaPhase = new QSpinBox;
    thetaPhase->setRange(0, 360 * 4);
    thetaPhase->setAlignment(Qt::AlignRight);
    parametersLayout->addRow("Desired phase of stimulation (deg)", thetaPhase);

    algorithmParametersGroupBox->setLayout(parametersLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(algorithmParametersGroupBox, Qt::AlignCenter);
    setLayout(layout);
}


RippleFilter::RippleFilter(QWidget *parent)
    : QDialog(parent)
{

    baselineValuesMeanBuffer = (double *) calloc(spikeConf->ntrodes.length(), sizeof(double));
    baselineValuesStdBuffer = (double *) calloc(spikeConf->ntrodes.length(), sizeof(double));

    algorithmParametersGroupBox = new QGroupBox("Ripple Disruption Parameters");
    QFormLayout *parametersLayout = new QFormLayout;

    /* This needs to be updated to get this list of channels from Trodes */
/*  QComboBox *StimChan = new QComboBox( false, this, "Channel Combo Box" );
   StimChan->insertStringList(*(daq_io_widget->ChannelStrings));
   parametersLayout->addRow("Tetrode / channel", StimChan); */

    sampDivisor = new QSpinBox;
    sampDivisor->setRange(0, 1000000);
    sampDivisor->setValue(FS_RT_DEFAULT_SAMP_DIVISOR);
    sampDivisor->setAlignment(Qt::AlignRight);
    parametersLayout->addRow("Sample Divisor", sampDivisor);

    ripCoeff1 = new QLineEdit(QString::number(FS_RT_DEFAULT_RIPPLE_COEFF1));
    ripCoeff1->setValidator(new QDoubleValidator(0.0, 2.0, 3, this));
    ripCoeff1->setAlignment(Qt::AlignRight);
    parametersLayout->addRow("Rip. Coeff 1", ripCoeff1);

    ripCoeff2 = new QLineEdit(QString::number(FS_RT_DEFAULT_RIPPLE_COEFF2));
    ripCoeff2->setValidator(new QDoubleValidator(0.0, 2.0, 3, this));
    ripCoeff2->setAlignment(Qt::AlignRight);
    parametersLayout->addRow("Rip. Coeff 2", ripCoeff2);

    ripThresh = new QLineEdit(QString::number(FS_RT_DEFAULT_RIPPLE_THRESHOLD));
    ripThresh->setValidator(new QDoubleValidator(0.0, 20.0, 2, this));
    ripThresh->setAlignment(Qt::AlignRight);
    parametersLayout->addRow("Ripple Threshold (sd)", ripThresh);

    nAboveThreshold = new QSpinBox;
    nAboveThreshold->setRange(0, 128);
    nAboveThreshold->setValue(FS_RT_DEFAULT_RIPPLE_N_ABOVE_THRESH);
    nAboveThreshold->setAlignment(Qt::AlignRight);
    parametersLayout->addRow("Num Above Threshold", nAboveThreshold);

    lockoutTime = new QSpinBox;
    lockoutTime->setRange(0, 30000);
    lockoutTime->setValue(FS_RT_DEFAULT_RIPPLE_LOCKOUT);
    lockoutTime->setAlignment(Qt::AlignRight);
    parametersLayout->addRow("Lockout period (timestamps)", lockoutTime);

    detectNoRipples = new QCheckBox;
    detectNoRipples->setChecked(false);
    parametersLayout->addRow("Detect Ripple Absence", detectNoRipples);

    detectNoRippleTime = new QSpinBox;
    detectNoRippleTime->setRange(0, 300000);
    detectNoRippleTime->setValue(60000);
    detectNoRippleTime->setAlignment(Qt::AlignRight);
    parametersLayout->addRow("No ripples window length (timestamps)", detectNoRippleTime);


    baselineChannelValuesGroupBox = new QGroupBox("Baseline Ripple Values");
    QGridLayout *baselineChannelValuesGroupBoxLayout = new QGridLayout;
    baselineChannelValuesGroupBox->setLayout(baselineChannelValuesGroupBoxLayout );

    rippleValueUpdateButtonGroup = new QButtonGroup();
    pullRippleValuesButton = new QPushButton("Live Update");
    pullRippleValuesButton->setCheckable(true);
    pullRippleValuesButton->setChecked(true);
    //stopResetRippleValuesButton = new QPushButton("Stop Update");
    //stopResetRippleValuesButton->setCheckable(true);
    pushRippleValuesButton = new QPushButton("Use Custom");
    pushRippleValuesButton->setCheckable(true);
    rippleValueUpdateButtonGroup->addButton(pullRippleValuesButton);
    //rippleValueUpdateButtonGroup->addButton(stopResetRippleValuesButton);
    rippleValueUpdateButtonGroup->addButton(pushRippleValuesButton);
    baselineChannelValuesGroupBoxLayout->addWidget(pullRippleValuesButton, 0, 0);
    //baselineChannelValuesGroupBoxLayout->addWidget(stopResetRippleValuesButton, 0, 1);
    baselineChannelValuesGroupBoxLayout->addWidget(pushRippleValuesButton, 0, 1);

    connect(pushRippleValuesButton, SIGNAL(toggled(bool)), this, SLOT(setBaselineValuesListEditable(bool)));
    connect(rippleValueUpdateButtonGroup, SIGNAL(buttonToggled(int,bool)), this, SLOT(updateRippleData(void)));


    baselineChannelValuesGroup = new QWidget;
    baselineChannelValuesLayout = new QGridLayout;
    baselineChannelValuesGroup->setLayout(baselineChannelValuesLayout);

    baselineChannelValuesGroupBoxLayout->addWidget(baselineChannelValuesGroup, 1, 0, 1, 2);
    //baselineChannelValuesGroup->setEnabled(false);


    qDebug() << "FSGUI: 1";
    baselineGroupMap = new QMap<int,QBaselineValueLine>;

    baselineChannelValuesLayout->addWidget(new QLabel("ID"), 0, 0);
    baselineChannelValuesLayout->addWidget(new QLabel("Mean"), 0, 1);
    baselineChannelValuesLayout->addWidget(new QLabel("Std"), 0, 2);


    for(int ii = 0; ii < spikeConf->ntrodes.length(); ii++) {
        int tmpNTrodeId = spikeConf->ntrodes[ii]->nTrodeId;

        QLabel *ntrodeIDLabel = new QLabel(QString::number(tmpNTrodeId));
        ntrodeIDLabel->setVisible(false);
        baselineChannelValuesLayout->addWidget(ntrodeIDLabel, ii+1, 0);

        QLineEdit *newBaselineMeanEdit = new QLineEdit("0");
        newBaselineMeanEdit->setValidator(new QDoubleValidator(-100.0, 100.0, 4, this));
        newBaselineMeanEdit->setAlignment(Qt::AlignRight);
        newBaselineMeanEdit->setVisible(false);
        baselineChannelValuesLayout->addWidget(newBaselineMeanEdit, ii+1, 1);
        // Using new QT5 style of connect to allow C++11 lambda functions
        connect(newBaselineMeanEdit, &QLineEdit::textChanged,
                [=](QString text){this->updateBaselineValueMeanBuffer(ii,text);});

        QLineEdit *newBaselineStdEdit = new QLineEdit("0");
        newBaselineStdEdit->setValidator(new QDoubleValidator(-100.0, 100.0, 4, this));
        newBaselineStdEdit->setAlignment(Qt::AlignRight);
        newBaselineStdEdit->setVisible(false);
        baselineChannelValuesLayout->addWidget(newBaselineStdEdit, ii+1, 2);
        // Using new QT5 style of connect to allow C++11 lambda functions
        connect(newBaselineStdEdit, &QLineEdit::textChanged,
                [=](QString text){this->updateBaselineValueStdBuffer(ii,text);});

        baselineGroupMap->insert(ii, QBaselineValueLine(ntrodeIDLabel,newBaselineMeanEdit, newBaselineStdEdit));
    }
    qDebug() << "FSGUI: 2";

    setBaselineValuesListEditable(false);
    qDebug() << "FSGUI: 3";

    parametersLayout->addRow(baselineChannelValuesGroupBox);

    connect(sampDivisor, SIGNAL(valueChanged(int)), this, SLOT(updateRippleData(void)));
    connect(ripCoeff1, SIGNAL(textChanged(const QString &)), this, SLOT(updateRippleData(void)));
    connect(ripCoeff2, SIGNAL(textChanged(const QString &)), this, SLOT(updateRippleData(void)));
    connect(ripThresh, SIGNAL(textChanged(const QString &)), this, SLOT(updateRippleData(void)));

    connect(lockoutTime, SIGNAL(valueChanged(int)), this, SLOT(updateRippleData(void)));
    connect(nAboveThreshold, SIGNAL(valueChanged(int)), this, SLOT(updateRippleData(void)));
    connect(detectNoRipples, SIGNAL(clicked()), this, SLOT(updateRippleData(void)));
    connect(detectNoRippleTime, SIGNAL(valueChanged(int)), this, SLOT(updateRippleData(void)));


    algorithmParametersGroupBox->setLayout(parametersLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(algorithmParametersGroupBox, Qt::AlignCenter);
    setLayout(layout);

}

void RippleFilter::setEnableCustomBaselineValues(bool enabled) {
    baselineChannelValuesGroup->setEnabled(enabled);
}

void RippleFilter::setBaselineValuesListEditable(bool editable) {
    for(int ii = 0; ii < spikeConf->ntrodes.length(); ii++) {
        (std::get<1>(baselineGroupMap->value(ii)))->setReadOnly(!editable);
        (std::get<2>(baselineGroupMap->value(ii)))->setReadOnly(!editable);
        if(editable) {
            (std::get<1>(baselineGroupMap->value(ii)))->setStyleSheet("color: rgb(0,0,0);");
            (std::get<2>(baselineGroupMap->value(ii)))->setStyleSheet("color: rgb(0,0,0);");
        } else {
            (std::get<1>(baselineGroupMap->value(ii)))->setStyleSheet("color: rgb(150,150,150);");
            (std::get<2>(baselineGroupMap->value(ii)))->setStyleSheet("color: rgb(150,150,150);");
        }
    }
}

void RippleFilter::setBaselineValuesSelect(qint16 ind, bool selected) {
    //qDebug() << "Updating Baseline Values Ripple";
    std::get<0>(baselineGroupMap->value(ind))->setVisible(selected);
    std::get<1>(baselineGroupMap->value(ind))->setVisible(selected);
    std::get<2>(baselineGroupMap->value(ind))->setVisible(selected);


}

void RippleFilter::setFilterEnabled(bool value)
{
    enabled = value;
    updateRippleData();
}

void RippleFilter::updateBaselineValuesListMean(double *means) {
    for(int ii = 0; ii < spikeConf->ntrodes.length(); ii++) {
        (std::get<1>(baselineGroupMap->value(ii)))->setText(QString::number(means[ii]));
    }
}

void RippleFilter::updateBaselineValuesListStd(double *stds) {
    for(int ii = 0; ii < spikeConf->ntrodes.length(); ii++) {
        (std::get<2>(baselineGroupMap->value(ii)))->setText(QString::number(stds[ii]));
    }

}

void RippleFilter::updateBaselineValueMeanBuffer(int nTrodeIndex, QString mean) {
    baselineValuesMeanBuffer[nTrodeIndex] = mean.toDouble();
    updateCustomBaselineMean();
    //qDebug() << "mean " << spikeConf->ntrodes[nTrodeIndex]->nTrodeId << " " << mean;
}

void RippleFilter::updateBaselineValueStdBuffer(int nTrodeIndex, QString std) {
    baselineValuesStdBuffer[nTrodeIndex] = std.toDouble();
    updateCustomBaselineStd();
    //qDebug() << "std " <<nTrodeID << " " << std;
}

void RippleFilter::updateCustomBaselineMean(void) {
    if(pushRippleValuesButton->isChecked()) {
        emit SendFSDataMessage(FS_SET_CUSTOM_RIPPLE_BASELINE_MEAN, (char *)baselineValuesMeanBuffer,
                               spikeConf->ntrodes.length()*sizeof(double));
    }
}

void RippleFilter::updateCustomBaselineStd(void) {
    if(pushRippleValuesButton->isChecked()) {
        emit SendFSDataMessage(FS_SET_CUSTOM_RIPPLE_BASELINE_STD, (char *)baselineValuesStdBuffer,
                       spikeConf->ntrodes.length()*sizeof(double));
    }
}

void RippleFilter::updateRippleData(void)
{
    RippleFilterParameters data;
    qDebug() << "Update ripple param";
    data.sampDivisor = sampDivisor->value();
    data.ripCoeff1 = ripCoeff1->text().toDouble();
    data.ripCoeff2 = ripCoeff2->text().toDouble();
    data.ripple_threshold = ripThresh->text().toDouble();
    data.n_above_thresh = nAboveThreshold->value();
    data.lockoutTime = lockoutTime->value();
    data.detectNoRipples = detectNoRipples->isChecked();
    data.detectNoRippleTime = detectNoRippleTime->value();
    data.enabled = enabled;
    data.useCustomBaseline = pushRippleValuesButton->isChecked();
    data.updateCustomBaseline = pullRippleValuesButton->isChecked();
    qDebug() << "FSGUI RippleFilterParameters update custom baseline " << data.useCustomBaseline << " " << data.updateCustomBaseline;

    emit SendFSDataMessage(FS_SET_RIPPLE_STIM_PARAMS, (char*)&data, sizeof(RippleFilterParameters));
}

void RippleFilter::saveToXML(QDomDocument &doc, QDomElement &rtConf)
{
    QDomElement nt = doc.createElement("rippleConfiguration");

    nt.setAttribute("sampleDivisor", sampDivisor->value());
    nt.setAttribute("ripCoeff1", ripCoeff1->text().toDouble());
    nt.setAttribute("ripCoeff2", ripCoeff2->text().toDouble());
    nt.setAttribute("ripThresh", ripThresh->text().toDouble());
    nt.setAttribute("nAboveThresh", nAboveThreshold->value());
    nt.setAttribute("lockoutTime", lockoutTime->value());
    nt.setAttribute("lockoutTime", lockoutTime->value());
    nt.setAttribute("detectNoRipples", detectNoRipples->isChecked());
    nt.setAttribute("detectNoRippleTime", detectNoRippleTime->value());

    QDomElement rippleBaselineList = doc.createElement("BaselineList");
    if(pullRippleValuesButton->isChecked()) {
        rippleBaselineList.setAttribute("mode", "live");
    } else if(pushRippleValuesButton->isChecked()) {
        rippleBaselineList.setAttribute("mode", "custom");
    }
    nt.appendChild(rippleBaselineList);

    //baselineGroupList->iterator
    //QListIterator<QBaselineValueLine> baselineGroupIter(baselineGroupList);
    for(QMap<int,QBaselineValueLine>::const_iterator item = baselineGroupMap->begin();
          item != baselineGroupMap->end(); ++item) {
        QDomElement nTrodeRippleBaselineValue = doc.createElement("nTrodeValue");

        nTrodeRippleBaselineValue.setAttribute("index", item.key());
        nTrodeRippleBaselineValue.setAttribute("mean", std::get<1>(item.value())->text().toDouble());
        nTrodeRippleBaselineValue.setAttribute("std", std::get<2>(item.value())->text().toDouble());
        rippleBaselineList.appendChild(nTrodeRippleBaselineValue);
    }
    rtConf.appendChild(nt);
}

int RippleFilter::loadFromXML(QDomElement &nt)
{
    sampDivisor->setValue(nt.attribute("sampleDivisor").toInt());
    ripCoeff1->setText(nt.attribute("ripCoeff1"));
    ripCoeff2->setText(nt.attribute("ripCoeff2"));
    ripThresh->setText(nt.attribute("ripThresh"));
    nAboveThreshold->setValue(nt.attribute("nAboveThresh").toInt());
    lockoutTime->setValue(nt.attribute("lockoutTime").toInt());
    detectNoRipples->setChecked((bool) nt.attribute("detectNoRipples").toInt());
    detectNoRippleTime->setValue(nt.attribute("detectNoRippleTime").toInt());

    QDomNodeList nList = nt.childNodes();
    int rippleBaselineValueEntryCount = 0;  // To make sure only one ripple baseline entry exists
    for(int ii = 0; ii < nList.length(); ii ++) {
        qDebug() << nList.at(ii).toElement().tagName() << rippleBaselineValueEntryCount;
        if(nList.at(ii).toElement().tagName().compare(QString("BaselineList")) == 0 &&
                rippleBaselineValueEntryCount < 1) {
            rippleBaselineValueEntryCount ++;
            QString rippleBaselineMode = nList.at(ii).toElement().attribute("mode");
            if(rippleBaselineMode.compare(QString("live")) == 0) {
                pullRippleValuesButton->setChecked(true);
                // don't load baseline values, in config file for recordkeeping purposes
            } else if(rippleBaselineMode.compare(QString("custom")) == 0) {
                pushRippleValuesButton->setChecked(true);
                // load custom baseline values

                QDomNodeList ntrodeBaselineList = nList.at(ii).childNodes();
                for(int ntrodeInd = 0; ntrodeInd < ntrodeBaselineList.length(); ntrodeInd ++) {
                    QDomElement ntrodeBaselineElem = ntrodeBaselineList.at(ntrodeInd).toElement();
                    if(ntrodeBaselineElem.tagName().compare(QString("nTrodeValue")) == 0) {
                        int nTrodeIndex = ntrodeBaselineElem.attribute("index").toInt();
                        std::get<1>(baselineGroupMap->value(nTrodeIndex))->setText(ntrodeBaselineElem.attribute("mean"));
                        std::get<2>(baselineGroupMap->value(nTrodeIndex))->setText(ntrodeBaselineElem.attribute("std"));
                    } else {
                        qDebug() << "fsFeedbackTab: Config Error expected nTrodeValue.";
                        return -1;
                    }
                }

            } else {
                qDebug() << "fsFeedbackTab: Config Error BaselineList mode not live or custom.";
                return -1;
            }

        } else {
            qDebug() << "fsFeedbackTab: Config Error expecting single BaselineList entry.";
            return -1;
        }
    }
    return 1;
}


SpatialFilter::SpatialFilter(QWidget *parent)
    : QDialog(parent)
{
    QGroupBox *algorithmParametersGroupBox = new QGroupBox("Spatial Filter Parameters");
    QGridLayout *parametersLayout = new QGridLayout;

    int row = 0;

    lowerLeftX = new QSpinBox;
    /* FIX RANGES TO IMAGE SIZE */
    lowerLeftX->setRange(0, 2000);
    lowerLeftX->setValue(0);
    lowerLeftX->setAlignment(Qt::AlignRight);
    lowerLeftY = new QSpinBox;
    lowerLeftY->setRange(0, 2000);
    lowerLeftY->setValue(0);
    lowerLeftY->setAlignment(Qt::AlignRight);
    parametersLayout->addWidget(new QLabel("Lower Left (X,Y)"), row, row, 1, 1, Qt::AlignRight);
    parametersLayout->addWidget(lowerLeftX, row, 1);
    parametersLayout->addWidget(lowerLeftY, row, 2);

    upperRightX = new QSpinBox;
    upperRightX->setRange(0, 2000);
    upperRightX->setValue(0);
    upperRightX->setAlignment(Qt::AlignRight);
    upperRightY = new QSpinBox;
    upperRightY->setRange(0, 2000);
    upperRightY->setValue(0);
    upperRightY->setAlignment(Qt::AlignRight);
    parametersLayout->addWidget(new QLabel("Upper Right (X,Y)"), ++row, 0, 1, 1, Qt::AlignRight);
    parametersLayout->addWidget(upperRightX, row, 1);
    parametersLayout->addWidget(upperRightY, row, 2);

    cmPerPix = new QDoubleSpinBox;
    cmPerPix->setRange(0, 10);
    cmPerPix->setValue(0.2);
    cmPerPix->setAlignment(Qt::AlignRight);
    parametersLayout->addWidget(new QLabel("cm per pixel"), ++row, 0, 1, 1, Qt::AlignRight);
    parametersLayout->addWidget(cmPerPix, row, 1);


    minSpeedThresh = new QLineEdit(QString::number(-1), this);
    minSpeedThresh->setValidator(new QDoubleValidator(-1, 200.0, 2, this));
    minSpeedThresh->setAlignment(Qt::AlignRight);
    parametersLayout->addWidget(new QLabel("Minimum Speed"), ++row, 0, 1, 1, Qt::AlignRight);
    parametersLayout->addWidget(minSpeedThresh, row, 1, 1, 1);
    parametersLayout->addWidget(new QLabel("cm/sec"), row, 2, 1, 1);

    maxSpeedThresh = new QLineEdit(QString::number(500), this);
    maxSpeedThresh->setValidator(new QDoubleValidator(0.0, 4.0, 2, this));
    maxSpeedThresh->setAlignment(Qt::AlignRight);
    parametersLayout->addWidget(new QLabel("Maximum Speed"), ++row, 0, 1, 1, Qt::AlignRight);
    parametersLayout->addWidget(maxSpeedThresh, row, 1, 1, 1);
    parametersLayout->addWidget(new QLabel("cm/sec"), row, 2, 1, 1);

    lockoutTime = new QSpinBox;
    lockoutTime->setRange(0, 100000);
    lockoutTime->setValue(7500);
    lockoutTime->setAlignment(Qt::AlignRight);
    parametersLayout->addWidget(new QLabel("Min On/Off Length"), ++row, 0, 1, 1, Qt::AlignRight);
    parametersLayout->addWidget(lockoutTime, row, 1);
    parametersLayout->addWidget(new QLabel("timestamp units"), row, 2, 1, 1);


    parametersLayout->setColumnStretch(0, 2);

    connect(lowerLeftX, SIGNAL(valueChanged(int)), this, SLOT(updateSpatialData(void)));
    connect(lowerLeftY, SIGNAL(valueChanged(int)), this, SLOT(updateSpatialData(void)));
    connect(upperRightX, SIGNAL(valueChanged(int)), this, SLOT(updateSpatialData(void)));
    connect(upperRightY, SIGNAL(valueChanged(int)), this, SLOT(updateSpatialData(void)));
    connect(cmPerPix, SIGNAL(valueChanged(double)), this, SLOT(updateSpatialData(void)));
    connect(minSpeedThresh, SIGNAL(textChanged(const QString &)), this, SLOT(updateSpatialData(void)));
    connect(maxSpeedThresh, SIGNAL(textChanged(const QString &)), this, SLOT(updateSpatialData(void)));
    connect(lockoutTime, SIGNAL(valueChanged(int)), this, SLOT(updateSpatialData(void)));

    algorithmParametersGroupBox->setLayout(parametersLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(algorithmParametersGroupBox, Qt::AlignCenter);
    setLayout(layout);
}

void SpatialFilter::setFilterEnabled(bool value)
{
    enabled = value;
    updateSpatialData();
}

void SpatialFilter::updateSpatialData(void)
{
    SpatialFilterParameters data;

    data.lowerLeftX = lowerLeftX->value();
    data.lowerLeftY = lowerLeftY->value();
    data.upperRightX = upperRightX->value();
    data.upperRightY = upperRightY->value();
    data.minSpeed = minSpeedThresh->text().toDouble();
    data.maxSpeed = maxSpeedThresh->text().toDouble();
    data.lockoutTime = lockoutTime->value();
    data.enabled = enabled;
    data.cmPerPix = cmPerPix->value();

    emit SendFSDataMessage(FS_SET_SPATIAL_STIM_PARAMS, (char*)&data, sizeof(SpatialFilterParameters));
}

void SpatialFilter::saveToXML(QDomDocument &doc, QDomElement &rtConf)
{
    QDomElement nt = doc.createElement("spatialConfiguration");

    nt.setAttribute("lowerLeftX", lowerLeftX->value());
    nt.setAttribute("lowerLeftY", lowerLeftY->value());
    nt.setAttribute("upperRightX", upperRightX->value());
    nt.setAttribute("upperRightY", upperRightY->value());

    nt.setAttribute("cmPerPix", cmPerPix->value());
    nt.setAttribute("minSpeed", minSpeedThresh->text().toDouble());
    nt.setAttribute("maxSpeed", maxSpeedThresh->text().toDouble());
    nt.setAttribute("lockoutTime", lockoutTime->value());

    rtConf.appendChild(nt);

}

int SpatialFilter::loadFromXML(QDomElement &nt)
{
    lowerLeftX->setValue(nt.attribute("lowerLeftX").toInt());
    lowerLeftY->setValue(nt.attribute("lowerLeftY").toInt());
    upperRightX->setValue(nt.attribute("upperRightX").toInt());
    upperRightY->setValue(nt.attribute("upperRightY").toInt());
    cmPerPix->setValue(nt.attribute("cmPerPix").toDouble());

    minSpeedThresh->setText(nt.attribute("minSpeed"));
    maxSpeedThresh->setText(nt.attribute("maxSpeed"));
    lockoutTime->setValue(nt.attribute("lockoutTime").toDouble());

    return 1;
}

PhaseFilter::PhaseFilter(QWidget *parent) // TODO finish gui elems
    : QDialog(parent)
{
    QGroupBox *filterParametersGroupBox = new QGroupBox("Filter Parameters");
    QGridLayout *fParamLayout = new QGridLayout;
    filterParametersGroupBox->setStyleSheet("QGroupBox { font-weight: bold; } ");

    QGroupBox *smoothingParametersGroupBox = new QGroupBox("Filter Smoothing Parameters");
    QGridLayout *sParamLayout = new QGridLayout;
    smoothingParametersGroupBox->setStyleSheet("QGroupBox { font-weight: bold; }");

    QGroupBox *targetParametersGroupBox = new QGroupBox("Targeting Parameters");
    QGridLayout *tParamLayout = new QGridLayout;
    targetParametersGroupBox->setStyleSheet("QGroupBox { font-weight: bold; }");

    QGroupBox *enableDisableGroupBox = new QGroupBox("Enable / Disable");
    enableDisableGroupBox->setStyleSheet("QGroupBox { font-weight: bold; }");
    QGridLayout *eParamLayout = new QGridLayout;

    /* ----------------------------------- */

    int row = 0;

    // Filter combo box
    filterType = new QComboBox;
    filterType->addItem("Butterworth");
    fParamLayout->addWidget(new QLabel("Filter Type"),row,0,1,1,Qt::AlignLeft);
    fParamLayout->addWidget(filterType,row,1,1,2,Qt::AlignLeft);

    // Filter order spin box
    filterOrder = new QSpinBox;
    filterOrder->setRange(1,8); // this can be easily expanded in phasefilter.cpp
    filterOrder->setValue(4);
    fParamLayout->addWidget(new QLabel("Filter Order"),++row,0,1,1,Qt::AlignLeft);
    fParamLayout->addWidget(filterOrder,row,1,1,1,Qt::AlignLeft);

    // Frequency Selectors
    freqLow = new QLineEdit;
    freqLow->setValidator(new QDoubleValidator(0.0,150.0,2,this));
    freqLow->setText("6");
    fParamLayout->addWidget(new QLabel("Low Bandpass"),++row,0,1,1,Qt::AlignLeft);
    fParamLayout->addWidget(freqLow, row, 1, 1, 1, Qt::AlignLeft);

    freqHigh = new QLineEdit;
    freqHigh->setValidator(new QDoubleValidator(0.0,150.0,2,this));
    freqHigh->setText("12");
    fParamLayout->addWidget(new QLabel("High Bandpass"),++row,0,1,1,Qt::AlignLeft);
    fParamLayout->addWidget(freqHigh, row, 1, 1, 1, Qt::AlignLeft);

    QFrame *line = new QFrame(this);
    line->setObjectName(QString::fromUtf8("line"));
    line->setGeometry(QRect(320, 150, 118, 3));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    fParamLayout->addWidget(line,++row,0,2,3,Qt::AlignLeft);

    filterParametersGroupBox->setLayout(fParamLayout);

    /* ----------------------------------- */

    row = 0;
    smoothingSize = new QSpinBox;
    smoothingSize->setRange(1,30000);
    smoothingSize->setValue(1024);
    sParamLayout->addWidget(new QLabel("Samples to Smooth"),row,0,Qt::AlignLeft);
    sParamLayout->addWidget(smoothingSize,row,1,Qt::AlignLeft);

    smoothingEnabled = new QCheckBox;
    sParamLayout->addWidget(new QLabel("Enabled Smoothing"),++row,0,Qt::AlignLeft);
    sParamLayout->addWidget(smoothingEnabled,row,1,Qt::AlignLeft);

    smoothingParametersGroupBox->setLayout(sParamLayout);

    /* -------------------------------------*/

    row = 0;

    targetPhase = new QComboBox;
    targetPhase->addItem("None");
    targetPhase->addItem("Peak");
    targetPhase->addItem("Falling Zero");
    targetPhase->addItem("Trough");
    targetPhase->addItem("Rising Zero");
    tParamLayout->addWidget(new QLabel("Target Phase"), row, 0, 1,1, Qt::AlignLeft);
    tParamLayout->addWidget(targetPhase, row, 1,1, Qt::AlignRight);

    targetPower = new QLineEdit;
    targetPower->setValidator(new QDoubleValidator(0.0, 100.0, 2, this));
    targetPower->setText("0");
    tParamLayout->addWidget(new QLabel("Target Power"),++row,0,1,1,Qt::AlignLeft);
    tParamLayout->addWidget(targetPower,row,1,1,1,Qt::AlignRight);

    delay = new QSpinBox;
    delay->setRange(0,30000);
    tParamLayout->addWidget(new QLabel("Delay\n(not supported yet)"), ++row, 0, 1,1,Qt::AlignLeft);
    tParamLayout->addWidget(delay,row,1,1,1,Qt::AlignLeft);

    bufferSize = new QSpinBox;
    bufferSize->setRange(1,30000);
    bufferSize->setValue(125);
    bufferSizeSuggest = new QLabel("1/2-longest\nwavelength: 125");bufferSizeSuggest->setStyleSheet("*{ color:red; font-size: 9px;}");
    tParamLayout->addWidget(new QLabel("Buffer Size\n(For Averagin Amp)"),++row,0,1,1,Qt::AlignLeft);
    tParamLayout->addWidget(bufferSize,row,1,1,1,Qt::AlignLeft);
    tParamLayout->addWidget(bufferSizeSuggest,row,2,1,1,Qt::AlignLeft);

    lockoutTime = new QSpinBox;
    lockoutTime->setRange(0,100000);
    lockoutTime->setValue(10000);
    tParamLayout->addWidget(new QLabel("Lockout Time"), ++row, 0, 1,1,Qt::AlignLeft);
    tParamLayout->addWidget(lockoutTime, row, 1, 1,1,Qt::AlignLeft);

    targetParametersGroupBox->setLayout(tParamLayout);

    /* ----------------------------------- */

    row = 0;

    nAtRequirement = new QSpinBox;
    eParamLayout->addWidget(new QLabel("NTrodes At \nRequirement"),row,0,Qt::AlignLeft);
    eParamLayout->addWidget(nAtRequirement,row,1,Qt::AlignLeft);
    nAtRequirement->setValue(1);

    enablePhase = new QCheckBox;
    eParamLayout->addWidget(new QLabel("Enable Phase"), ++row, 0, Qt::AlignLeft);
    eParamLayout->addWidget(enablePhase,row,1,Qt::AlignLeft);
    enablePhase->setChecked(true);

    enablePower = new QCheckBox;
    eParamLayout->addWidget(new QLabel("Enable Power"),++row,0,Qt::AlignLeft);
    eParamLayout->addWidget(enablePower);
    enablePower->setChecked(true);

    enableDisableGroupBox->setLayout(eParamLayout);

    //fParamLayout->setColumnStretch(0, 2);

    connect(filterType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateData(void)));
    connect(filterOrder, SIGNAL(valueChanged(int)), this, SLOT(updateData(void)));
    connect(freqLow, SIGNAL(textChanged(QString)), this, SLOT(updateData(void)));
    connect(freqHigh, SIGNAL(textChanged(QString)), this, SLOT(updateData(void)));
    connect(bufferSize, SIGNAL(valueChanged(int)), this, SLOT(updateData(void)));
    connect(smoothingEnabled, SIGNAL(stateChanged(int)), this, SLOT(updateData(void)));
    connect(smoothingSize,SIGNAL(valueChanged(int)),this,SLOT(updateData(void)));
    connect(targetPhase, SIGNAL(currentIndexChanged(int)), this, SLOT(updateData(void)));
    connect(targetPower, SIGNAL(textChanged(QString)), this, SLOT(updateData(void)));
    connect(delay,SIGNAL(valueChanged(int)),this,SLOT(updateData(void)));
    connect(nAtRequirement,SIGNAL(valueChanged(int)), this, SLOT(updateData()));
    connect(enablePhase, SIGNAL(stateChanged(int)), this, SLOT(updateData(void)));
    connect(enablePower, SIGNAL(stateChanged(int)), this, SLOT(updateData(void)));


    filterParametersGroupBox->setLayout(fParamLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(filterParametersGroupBox, Qt::AlignCenter);
    layout->addWidget(smoothingParametersGroupBox,Qt::AlignCenter);
    layout->addWidget(targetParametersGroupBox, Qt::AlignCenter);
    layout->addWidget(enableDisableGroupBox, Qt::AlignCenter);
    setLayout(layout);
}

void PhaseFilter::setFilterEnabled(bool enable)
{
    enabled = enable;
    updateData();
}

void PhaseFilter::updateData(void)
{
    QString filename="FSGUI_Debug.txt";
    QFile file( filename );QTextStream stream( &file );
    file.open(QIODevice::WriteOnly|QIODevice::Append);

    PhaseFiltParameters data;
    stream << "FSGui: Update phase filter param";

    data.filterType = filterType->currentIndex();
    data.order = filterOrder->value();
    data.bufferSize = bufferSize->value();

    data.freqLow = freqLow->text().toDouble();
    data.freqHigh = freqHigh->text().toDouble();

    data.smooth = smoothingEnabled->checkState();
    data.smoothWidth = smoothingSize->value();

    data.desiredStartPhase = (target_t) targetPhase->currentIndex();
    data.desiredPower = targetPower->text().toDouble();
    data.desiredDelay = delay->value();
    data.lockoutTime = lockoutTime->value();

    data.nRequired = nAtRequirement->value();
    data.enablePhase = enablePhase->checkState();
    data.enablePower = enablePower->checkState();
    data.enabled = enabled;

    // Variables not currently set by FSGui or handled in FSData
    data.filterChannels = 1;
    data.desiredStopPhase = NONE;


    stream << "Enabled status: " << data.enabled;

    // update qlabel suggesting buffer size to be about half the wavelength
    qint16 suggestedBuffer= (int) 1500/(2*(data.freqLow));
    bufferSizeSuggest->setText( QString("1/2-longest\nwavelength: ") + QString::number(suggestedBuffer) );


    emit SendFSDataMessage(FS_SET_PHASE_PARAMS, (char*) &data, sizeof(PhaseFiltParameters));
}
