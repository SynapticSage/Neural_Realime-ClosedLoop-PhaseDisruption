
#include "fsGUI.h"



StimConfigTab::StimConfigTab (QWidget *parent)
    : QWidget(parent)
{
    QString s;


    stimConfigA = new StimConfigureWidget();
    stimConfigA->setID(0);
    connect(stimConfigA, SIGNAL(newStateScript(QString*)), this, SIGNAL(newStateScript(QString*)));
    connect(stimConfigA, SIGNAL(stimulatorReadyChanged()), this, SLOT(updateStimulatorStatus()));
    connect(stimConfigA, SIGNAL(sig_setAutoSettle(bool,int)), this, SIGNAL(sig_setAutoSettle(bool,int)));

    stimConfigB = new StimConfigureWidget();
    stimConfigB->setID(1);
    connect(stimConfigB, SIGNAL(newStateScript(QString*)), this, SIGNAL(newStateScript(QString*)));
    connect(stimConfigB, SIGNAL(stimulatorReadyChanged()), this, SLOT(updateStimulatorStatus()));
    connect(stimConfigB, SIGNAL(sig_setAutoSettle(bool,int)), this, SIGNAL(sig_setAutoSettle(bool,int)));

    QLabel *explanation = new QLabel(
        "Configuration parameters for indvidual pulses or " \
        "pulse sequences. <br>These parameters are used for "
        "<b>either</b> <i>Output-only Experiments</i> or " \
        "<i>Real-time Feedback Experiments.</i>");
    //explanation->setAlignment(Qt::AlignHCenter);
    explanation->setWordWrap(true);
    explanation->setIndent(30);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(explanation, 0, 0, 1, 2);
//    layout->addWidget(widget, row, column, rowSpan, columnSpan, Alignment);
    stimulatorAButton = new QPushButton("A");
    layout->addWidget(stimulatorAButton, 1, 0, Qt::AlignRight);
    stimulatorAButton->setCheckable(true);
    stimulatorAButton->setChecked(false);
    stimulatorAButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    stimulatorAButton->setStyleSheet("QPushButton::checked{color: green;}");
    layout->addWidget(stimConfigA, 1, 1, Qt::AlignLeft);
    activeStimulator = 0;
    stimConfigA->setEnabled(false);


    stimulatorBButton = new QPushButton("B");
    layout->addWidget(stimulatorBButton, 2, 0, Qt::AlignRight);
    stimulatorBButton->setCheckable(true);
    stimulatorBButton->setChecked(false);
    stimulatorBButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    stimulatorBButton->setStyleSheet("QPushButton::checked{color: green;}");
    layout->addWidget(stimConfigB, 2, 1, Qt::AlignLeft);
    stimConfigB->setEnabled(false);

    connect(stimulatorAButton, SIGNAL(toggled(bool)), this, SLOT(updateStimulatorStatus()));
    connect(stimulatorBButton, SIGNAL(toggled(bool)), this, SLOT(updateStimulatorStatus()));

    QFont font;
    font.setPointSize(32);
    font.setBold(true);
    stimulatorAButton->setFont(font);
    stimulatorBButton->setFont(font);

    layout->setColumnStretch(1, 1);

    setLayout(layout);
}

void StimConfigTab::updateStimulatorStatus(void)
{
    bool ready = true;

    stimConfigA->setEnabled(stimulatorAButton->isChecked());
    stimConfigB->setEnabled(stimulatorBButton->isChecked());

    // send out an indication of whether at least one stimulator is selected and ready
    if (!stimulatorAButton->isChecked() && !stimulatorBButton->isChecked()) {
        ready = false;
    }
    else {
        if (stimulatorAButton->isChecked()) {
            ready = stimConfigA->isHardwareReady();
            // also send the function number to FSData and update the information for the Latency Filter
            emit stateScriptFunctionValid(stimConfigA->getScriptFunctionNum(), stimConfigA->isHardwareReady());
        }
        if (stimulatorBButton->isChecked()) {
            ready = ready && stimConfigB->isHardwareReady();
            emit stateScriptFunctionValid(stimConfigB->getScriptFunctionNum(), stimConfigB->isHardwareReady());

        }
    }

    emit stimulatorsReady(ready);
    return;
}


void StimConfigTab::saveToXML(QDomDocument &doc, QDomElement &rootNode)
{
    QDomElement sConf = doc.createElement("stimulatorConfiguration");

    rootNode.appendChild(sConf);

    sConf.setAttribute("stimAChecked", stimulatorAButton->isChecked());
    sConf.setAttribute("stimBChecked", stimulatorBButton->isChecked());

    stimConfigA->saveToXML(doc, sConf);
    stimConfigB->saveToXML(doc, sConf);
}

int StimConfigTab::loadFromXML(QDomNode &stimNode)
{
    QDomElement nt;


    stimulatorAButton->setChecked((bool)stimNode.toElement().attribute("stimAChecked").toInt());
    stimulatorBButton->setChecked((bool)stimNode.toElement().attribute("stimBChecked").toInt());

    QDomNodeList nList = stimNode.childNodes();
    nt = nList.at(0).toElement();
    stimConfigA->loadFromXML(nt);
    nt = nList.at(1).toElement();
    stimConfigB->loadFromXML(nt);

    if (stimulatorAButton->isChecked()) {
        stimConfigA->sendScript();
    }
    if (stimulatorBButton->isChecked()) {
        stimConfigB->sendScript();
    }
    return 1;
}





StimConfigureWidget::StimConfigureWidget(QWidget *parent)
    : QWidget(parent)
{

    this->setID(-1); //initialize object ID to -1 (ID not set code)
    qApp->setStyleSheet("QGroupBox {  border: 1px solid gray; border-radius: 3px; margin-top: 1px;}");
    qApp->setStyleSheet("QGroupBox::title {font-weight: bold;}");

    QGroupBox *groupBox = new QGroupBox();

    pulseLengthSpinBox = new QDoubleSpinBox();
    pulseLengthSpinBox->setSuffix(" ms");
    pulseLengthSpinBox->setAlignment(Qt::AlignRight);
    pulseLengthSpinBox->setDecimals(0);
    pulseLengthSpinBox->setRange(1, 500);
    pulseLengthSpinBox->setToolTip("Length in milliseconds of each pulse in pulse sequence.");
    connect(pulseLengthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateStimParameters(void)));

    nPulsesSpinBox = new QSpinBox();
    // TODO Multiple pulses disabled until Statescript is fixed
    nPulsesSpinBox->setRange(0, 10000);
    nPulsesSpinBox->setValue(1);
    //nPulsesSpinBox->setRange(1, 1);
    nPulsesSpinBox->setAlignment(Qt::AlignRight);
    nPulsesSpinBox->setToolTip("Number of pulses in pulse sequence.");
    nPulsesSpinBox->setEnabled(true);    \
    nPulsesSpinBox->setSpecialValueText(tr("Inf"));
    connect(nPulsesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateStimParameters(void)));

    preDelaySpinBox = new QSpinBox();
    preDelaySpinBox->setRange(0,1000);
    preDelaySpinBox->setValue(0);
    preDelaySpinBox->setSuffix(" ms");
    preDelaySpinBox->setAlignment(Qt::AlignRight);
    preDelaySpinBox->setToolTip("Time before first pulse");

    sequencePeriodSpinBox = new QDoubleSpinBox();
    sequencePeriodSpinBox->setAlignment(Qt::AlignRight);
    sequencePeriodSpinBox->setSuffix(" ms");
    sequencePeriodSpinBox->setDecimals(0);
    sequencePeriodSpinBox->setRange(1, 5000);
    sequencePeriodSpinBox->setValue(100);
    sequencePeriodSpinBox->setToolTip("Period of pulses in pulse sequence.");
    connect(sequencePeriodSpinBox, SIGNAL(valueChanged(double)), this, SLOT(periodChanged(void)));
    sequenceFrequencySpinBox = new QDoubleSpinBox();
    sequenceFrequencySpinBox->setAlignment(Qt::AlignRight);
    sequenceFrequencySpinBox->setRange(0.2, 1000);
    sequenceFrequencySpinBox->setValue(10);
    sequenceFrequencySpinBox->setDecimals(1);
    sequenceFrequencySpinBox->setSuffix(" Hz");
    sequenceFrequencySpinBox->setToolTip("Frequency of pulses in pulse sequence.");
    sequenceFrequencySpinBox->setEnabled(true); //always disable, period has to be 1ms resolution

    connect(sequenceFrequencySpinBox, SIGNAL(valueChanged(double)), this, SLOT(frequencyChanged(void)));

    QLabel *pulseLengthGraphic = new QLabel;
    pulseLengthGraphic->setPixmap(QPixmap(":/images/pulselength.png"));
    pulseLengthGraphic->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

//    labelAutoSettle = new QLabel(tr("Auto Settle"));
//    labelAutoSettle->setToolTip("Turn on/off auto settling the stream display on stimulation to offset the stimulation distortion.");
    setAutoSettle = new QCheckBox(tr("Auto Settle"));
    setAutoSettle->setToolTip("Turn on/off auto settling the stream display on stimulation to offset the stimulation distortion.");
    setAutoSettle->setCheckable(true);
    setAutoSettle->setChecked(true);
    connect(setAutoSettle, SIGNAL(clicked(bool)), this, SLOT(autoSettleToggled(bool)));

    QGridLayout *parametersLayout = new QGridLayout;

    //QLabel *nPulsesGraphic = new QLabel;
    //nPulsesGraphic->setPixmap(QPixmap(":/images/npulses.png"));
    //nPulsesGraphic->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    parametersLayout->addWidget(new QLabel("Pulse Length"), 0, 0,
                                Qt::AlignRight | Qt::AlignVCenter);
    parametersLayout->addWidget(pulseLengthSpinBox, 0, 1, Qt::AlignVCenter);
    //parametersLayout->addWidget(pulseLengthGraphic, 0, 2);
    parametersLayout->setRowMinimumHeight(1, 5);

    parametersLayout->addWidget(new QLabel("# of Pulses"), 2, 0,
                                Qt::AlignRight | Qt::AlignVCenter);
    parametersLayout->addWidget(nPulsesSpinBox, 2, 1, Qt::AlignVCenter);

    parametersLayout->addWidget(new QLabel("Pre Delay"), 3, 0,
                                Qt::AlignRight | Qt::AlignVCenter);

    parametersLayout->addWidget(preDelaySpinBox, 3, 1, Qt::AlignVCenter);

    /* For multi-pulse sequences, control frequency or period of
     * pulses in stimulation train. */
    multiPulseGroup = new QGroupBox("Period and Frequency");
    QGridLayout *multiPulseLayout = new QGridLayout;
    multiPulseLayout->addWidget(new QLabel("Period"), 0, 0,
                                Qt::AlignRight | Qt::AlignVCenter);
    multiPulseLayout->addWidget(sequencePeriodSpinBox, 0, 1);

    multiPulseLayout->addWidget(new QLabel("Frequency"), 1, 0,
                                Qt::AlignRight | Qt::AlignVCenter);
    multiPulseLayout->addWidget(sequenceFrequencySpinBox, 1, 1);
    multiPulseGroup->setLayout(multiPulseLayout);
    multiPulseGroup->setEnabled(true);

    parametersLayout->addWidget(multiPulseGroup, 4, 0, 2, 2, Qt::AlignRight);

    //QLabel *pulsePeriodGraphic = new QLabel;
    //pulsePeriodGraphic->setPixmap(QPixmap(":/images/period.png"));
    //multiPulseLayout->addWidget(pulsePeriodGraphic,0,2,2,1, Qt::AlignCenter);

    pulseTrainGroup = new QGroupBox("Pulse Trains", this);
    pulseTrainGroup->setEnabled(true);
    QGridLayout *pulseTrainLayout = new QGridLayout;
    pulseTrainLayout->addWidget(new QLabel("Number of output trains"), 0, 0, Qt::AlignRight | Qt::AlignTop);
    nTrainsSpinBox = new QSpinBox();
    nTrainsSpinBox->setAlignment(Qt::AlignRight);
    nTrainsSpinBox->setRange(0, 200);
    nTrainsSpinBox->setValue(1);
    nTrainsSpinBox->setSpecialValueText("Inf.");
    nTrainsSpinBox->setToolTip("Number of output sequences to trigger before returning; Set to 0 for continuous trains");
    connect(nTrainsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateStimParameters(void)));
    pulseTrainLayout->addWidget(nTrainsSpinBox, 0, 1, Qt::AlignRight | Qt::AlignVCenter);


    pulseTrainLayout->addWidget(new QLabel("Inter-train Interval"), 1, 0, Qt::AlignRight | Qt::AlignVCenter);
    trainIntervalSpinBox = new QSpinBox();
    trainIntervalSpinBox->setAlignment(Qt::AlignRight);
    trainIntervalSpinBox->setSuffix(" ms");
    trainIntervalSpinBox->setRange(100, 60000);
    trainIntervalSpinBox->setValue(1000);
    trainIntervalSpinBox->setToolTip("Time in milliseconds from the onset of one \npulse/pulse sequence to the onset of the next.");
    connect(nTrainsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateStimParameters(void)));
    pulseTrainLayout->addWidget(trainIntervalSpinBox, 1, 1, Qt::AlignRight | Qt::AlignVCenter);

    pulseTrainGroup->setLayout(pulseTrainLayout);
    parametersLayout->addWidget(pulseTrainGroup, 4, 2, 2, 3, Qt::AlignCenter);


    /* For each stimulator, we need one and potentially two (if biphasic) pin masks. */
    primaryStimPinSpinBox = new QSpinBox();
    primaryStimPinSpinBox->setAlignment(Qt::AlignRight);
    primaryStimPinSpinBox->setRange(1, 64);
    primaryStimPinSpinBox->setToolTip("Output pin (range 1 - 64) to stimulate.\nFor biphasic triggering, this is the first pin triggered.");
    connect(primaryStimPinSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateStimParameters(void)));
    QLabel *primaryStimPinLabel = new QLabel("Primary");

    biphasicCheckBox = new QCheckBox("Enable Biphasic Stim");

    //TODO biphasic option disabled until statescript is fixed
    biphasicCheckBox->setEnabled(false);
    //connect(biphasicCheckBox, SIGNAL(stateChanged(int)), this, SLOT(ableBiphasicStimulation(int)));

    secondaryStimPinSpinBox = new QSpinBox();
    secondaryStimPinSpinBox->setAlignment(Qt::AlignRight);
    secondaryStimPinSpinBox->setRange(1, 64);
    secondaryStimPinSpinBox->setToolTip("For biphasic triggering, this is the second pin triggered.\nThe value must lie in the same bank of 16 as the primary pin.");
    connect(secondaryStimPinSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateStimParameters(void)));
    secondaryStimPinLabel = new QLabel("Secondary");

    QGridLayout *stimPinControlsLayout = new QGridLayout;
    stimPinControlsLayout->addWidget(primaryStimPinLabel, 0, 0,
                                     Qt::AlignRight | Qt::AlignVCenter);
    stimPinControlsLayout->addWidget(primaryStimPinSpinBox, 0, 1);
    stimPinControlsLayout->addWidget(biphasicCheckBox, 1, 0, 1, 2);
    stimPinControlsLayout->addWidget(secondaryStimPinLabel, 2, 0,
                                     Qt::AlignRight | Qt::AlignVCenter);
    stimPinControlsLayout->addWidget(secondaryStimPinSpinBox, 2, 1);

    QGroupBox *stimPinControlsGroup = new QGroupBox("Stimulation Pins");
    stimPinControlsGroup->setLayout(stimPinControlsLayout);
 //   stimPinControlsGroup->setStyleSheet("QGroupBox::enabled{border: 2px solid navy;border-radius: 5px; margin-top: 1ex;}" \
 //                                       "QGroupBox::title{subcontrol-origin: margin; subcontrol-position:top center; padding: 0 3px;}");
    parametersLayout->addWidget(stimPinControlsGroup, 0, 4, 4, 1,
                                Qt::AlignHCenter | Qt::AlignTop);

    sendScriptButton = new QPushButton("Send script", this);
    connect(sendScriptButton, SIGNAL(pressed()), this, SLOT(sendScript(void)));
    parametersLayout->addWidget(sendScriptButton, 3, 6, 1, 1,  Qt::AlignHCenter | Qt::AlignCenter);

    scriptStatus = new QLabel("Script NOT sent", this);

    parametersLayout->addWidget(scriptStatus, 4, 6, 1, 1,  Qt::AlignHCenter | Qt::AlignCenter);

    parametersLayout->addWidget(setAutoSettle, 5, 3, 6, 1, Qt::AlignCenter);
//     parametersLayout->addWidget(setAutoSettle, 5, 0, Qt::AlignCenter);
    parametersLayout->setRowMinimumHeight(5, 5);

    groupBox->setLayout(parametersLayout);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(groupBox, 0, 0);
    setLayout(layout);

    ableBiphasicStimulation(false);

    // Load defaults
    // Update state (number of pulses->period, biphasic->secondary stim)
}


void StimConfigureWidget::frequencyChanged(void)
{
    sequencePeriodSpinBox->blockSignals(true);
    sequencePeriodSpinBox->setValue(1000 / sequenceFrequencySpinBox->value());
    sequenceFrequencySpinBox->setValue(1000 / sequencePeriodSpinBox->value());
    sequencePeriodSpinBox->blockSignals(false);
    updateStimParameters();
    return;
}

void StimConfigureWidget::periodChanged(void)
{
    sequenceFrequencySpinBox->blockSignals(true);
    sequenceFrequencySpinBox->setValue(1000 / sequencePeriodSpinBox->value());
    sequenceFrequencySpinBox->blockSignals(false);
    updateStimParameters();
}

void StimConfigureWidget::ablePulseSequence(void)
{
    if (nPulsesSpinBox->value() > 1) {
        multiPulseGroup->setEnabled(true);
    }
    else {
        multiPulseGroup->setEnabled(false);
    }
    updateStimParameters();
}

void StimConfigureWidget::ableBiphasicStimulation(int state)
{
    if (state) {
        secondaryStimPinSpinBox->setEnabled(true);
        secondaryStimPinLabel->setEnabled(true);
    }
    else {
        secondaryStimPinSpinBox->setEnabled(false);
        secondaryStimPinLabel->setEnabled(false);
    }
    updateStimParameters();
}

void StimConfigureWidget::updateStimParameters(void)
{
    qint32 minInterval;

    // set the minimum train interval based on the number of pulses and their length
    if (nPulsesSpinBox->value() > 1) {
        minInterval = qCeil(nPulsesSpinBox->value() * sequencePeriodSpinBox->value() + pulseLengthSpinBox->value());
    }
    else {
        minInterval = qCeil(pulseLengthSpinBox->value());
    }

    if (nPulsesSpinBox->value() == 0) { // continuous pulses, so set pulse train number to 1
        nTrainsSpinBox->setValue(1);
        nTrainsSpinBox->setEnabled(false);
        trainIntervalSpinBox->setValue(1);
        trainIntervalSpinBox->setEnabled(false);
    }
    else {
        nTrainsSpinBox->setEnabled(true);
        trainIntervalSpinBox->setEnabled(true);
        if (minInterval < 100) minInterval = 100;
        trainIntervalSpinBox->setRange(minInterval, 60000);
    }

    scriptStatus->setText("Script NOT Sent");

    hardwareReady = 0;
    emit stimulatorReadyChanged();
    emit stimulatorPortSelected(primaryStimPinSpinBox->value());

    qDebug("Updating stim parameters.");
}

void StimConfigureWidget::setScriptFunctionNum(uint16_t num) {
    scriptFunctionNum = num;
}

uint16_t StimConfigureWidget::getScriptFunctionNum(void) {
    return scriptFunctionNum;
}

bool StimConfigureWidget::isHardwareReady(void) {
    return hardwareReady;
}

void StimConfigureWidget::sendScript(void) {
    QString scriptString;
    generateStateScript(scriptFunctionNum, &scriptString);
    emit newStateScript(&scriptString);
    scriptStatus->setText("Script Sent");
    hardwareReady = 1;
    emit stimulatorReadyChanged();
}

void StimConfigureWidget::generateStateScript(int functionNum, QString *scriptString) {

    int well, bit, delay, maxDelay, funcNum;
    QTextStream script(scriptString, QIODevice::WriteOnly);

    // declare go variable
    script << "int f" << functionNum << "go" << endl;

    QString goCheck = QString("f%1go == 1 ").arg(functionNum);
    QString s2 = "  ";
    QString s4 = s2.repeated(2);
    QString s6 = s2.repeated(3);
    QString s8 = s2.repeated(4);
    QString s10 = s2.repeated(5);
    QString s12 = s2.repeated(6);


    // declare and init pulse & train counters and lockout
    script << "int f" << functionNum << "PulseCounter = 1" << endl;
    script << "int f" << functionNum << "TrainCounter = 1;" << endl;
    script << "int f" << functionNum << "LockOut = 0;" << endl;


    script << "function " << functionNum << endl;
    script << s2 << "if (f" << functionNum << "LockOut == 0) do" << endl;
    script << s4 << "f" << functionNum << "LockOut = 1" << endl;
    script << s4 << "f" << functionNum << "go = 1" << endl;
    script << s4 << "f" << functionNum << "PulseCounter = 1" << endl;
    script << s4 << "f" << functionNum << "TrainCounter = 1" << endl;

    if (preDelaySpinBox->value() == 0) {
        script << s4 << "if " << goCheck << "do" << endl;
    }
    else {
        script << s4 << "if " <<goCheck << "do in " << preDelaySpinBox->value() << endl;
    }

    // set the number of trains if we're not doing continuous pulses
    if (nPulsesSpinBox->value() != 0){
        if (nTrainsSpinBox->value() != 0) {
            script << s6 << "while " << goCheck << "&& f" << functionNum << "TrainCounter <= " <<
                            nTrainsSpinBox->value() << " do every " << trainIntervalSpinBox->value() << endl;
        }
        // otherwise we want continuous pulse trains, so we use 1 as the check value and skip incrementing the counter below
        // we only do this if we're not doing continuous pulses
        else {
            script << s6 << "while " << goCheck << "&& f" << functionNum << "TrainCounter <= " <<
                            1 << " do every " << trainIntervalSpinBox->value() << endl;
        }
        script << s8 << "while "<< goCheck << "&& f" << functionNum << "PulseCounter <= " << nPulsesSpinBox->value() <<
                  " do every " << sequencePeriodSpinBox->value() << endl;
    }
    else {
        script << s8 << "while "<< goCheck << "&& f" << functionNum << "PulseCounter <= 1" <<
                  " do every " << sequencePeriodSpinBox->value() << endl;
    }

    script << s10 << "portout[" << primaryStimPinSpinBox->value() << "]=1" << endl;

    script << s10 << "do in " << pulseLengthSpinBox->value() << endl;
    script << s12 << "portout[" << primaryStimPinSpinBox->value() << "]=0" << endl;
    script << s10 << "end" << endl;

    // increment the counter unless the value is 0, which corresponds to continuous (infinite) pulses
    if (nPulsesSpinBox->value() != 0) {
        script << s10 << "f" << functionNum << "PulseCounter = f" << functionNum << "PulseCounter + 1" << endl;
    }
    script << s8 << "then do" << endl;
    script << s10 << "f" << functionNum << "PulseCounter = 1" << endl;
    // increment the train counter unless the value is 0, which corresponds to continuous (infinite) pulses
    if ((nPulsesSpinBox->value() != 0) && (nTrainsSpinBox->value() != 0)) {
        script << s10 << "f" << functionNum << "TrainCounter = f" << functionNum << "TrainCounter + 1" << endl;
    }
    script << s8 << "end" << endl;

    if (nPulsesSpinBox->value() != 0) { // we need to end the TrainCounter while loop
        script << s6 << "then do" << endl;
        script << s8 << "f" << functionNum << "PulseCounter = 1" << endl;
        script << s8 << "f" << functionNum << "TrainCounter = 1" << endl;
        script << s6 << "end" << endl;
    }
    // end the second if
    script << s4 << "end" << endl;
    // reset the lock out
    script << s4 << "f" << functionNum << "LockOut = 0" << endl;
    // end the first if
    script << s2 << "end" << endl;

    script << "end;" << endl;

    // declare the stop function
    script << endl;
    script << "function " << functionNum+1 << endl;
    script << s2 << "f" << functionNum << "go = 0" << endl;
    script << "end;" << endl;


 }

void StimConfigureWidget::autoSettleToggled(bool turnOn) {
    emit sig_setAutoSettle(turnOn, originID);
}


void StimConfigureWidget::saveToXML(QDomDocument &doc, QDomElement &sConf)
{
    QDomElement nt = doc.createElement("singleStimConfiguration");

    nt.setAttribute("pulseLength", pulseLengthSpinBox->value());
    nt.setAttribute("nPulses", nPulsesSpinBox->value());
    nt.setAttribute("sequenceFrequency", sequenceFrequencySpinBox->value());
    nt.setAttribute("sequencePeriod", sequencePeriodSpinBox->value());

    nt.setAttribute("primaryBit", primaryStimPinSpinBox->value());
    nt.setAttribute("secondaryBit", secondaryStimPinSpinBox->value());
    nt.setAttribute("biphasic", biphasicCheckBox->isChecked());

    nt.setAttribute("nOutputTrains", nTrainsSpinBox->value());
    nt.setAttribute("trainInterval", trainIntervalSpinBox->value());
    sConf.appendChild(nt);
}

int StimConfigureWidget::loadFromXML(QDomElement &nt)
{
    pulseLengthSpinBox->setValue(nt.attribute("pulseLength").toDouble());
    nPulsesSpinBox->setValue(nt.attribute("nPulses").toInt());
    sequenceFrequencySpinBox->setValue(nt.attribute("sequenceFrequency").toDouble());
    sequencePeriodSpinBox->setValue(nt.attribute("sequencePeriod").toDouble());

    primaryStimPinSpinBox->setValue(nt.attribute("primaryBit").toInt());
    secondaryStimPinSpinBox->setValue(nt.attribute("secondaryBit").toInt());
    biphasicCheckBox->setChecked((bool)nt.attribute("biphasic").toInt());

    nTrainsSpinBox->setValue(nt.attribute("nOutputTrains").toInt());
    trainIntervalSpinBox->setValue(nt.attribute("trainInterval").toInt());
    // ideally we'd do more error checking above
    return 1;
}

