
#include "fsGUI.h"

StimPulseTrainTab::StimPulseTrainTab (QWidget *parent)
  : QWidget(parent)
{
  QGridLayout *layout = new QGridLayout;

  stimulatorSelectComboBox = new QComboBox;
  stimulatorSelectComboBox->addItem("None");
  stimulatorSelectComboBox->addItem("A");
  stimulatorSelectComboBox->addItem("B");
  stimulatorSelectComboBox->addItem("Both (Alternating)");

  aOutSelectComboBox = new QComboBox;
  aOutSelectComboBox->addItem("None");
  aOutSelectComboBox->addItem("AOut 1");
  aOutSelectComboBox->addItem("AOut 2");
  aOut1Mode = aOut2Mode = 0;

  // Signals/Slots for stimulatorSelectComboBox are connected in
  //  main GUI code to StimConfigTab.
  layout->addWidget(new QLabel("Active Stimulator"),0,0, 
      Qt::AlignRight | Qt::AlignVCenter);
  layout->addWidget(stimulatorSelectComboBox,0,1);
  
  layout->addWidget(new QLabel("Active Analog Out"),0,2, 
      Qt::AlignRight | Qt::AlignVCenter);
  layout->addWidget(aOutSelectComboBox,0,3);

  layout->addWidget(new QLabel("Number of output trains"), 1, 0,
      1,1, Qt::AlignRight | Qt::AlignTop);

  QVBoxLayout *nTrainsLayout = new QVBoxLayout;

  nTrainsSpinBox = new QSpinBox();
  nTrainsSpinBox->setAlignment(Qt::AlignRight);
  nTrainsSpinBox->setRange(1,200);
  nTrainsSpinBox->setToolTip("Number of output sequences to trigger before returning.");
  nTrainsLayout->addWidget(nTrainsSpinBox);
  continuousButton = new QPushButton("Continuous Pulses");
  continuousButton->setCheckable(true);
  continuousButton->setToolTip("Execute digital or analog pulse sequence until Abort button is pressed. Not for use with continuous analog waveforms");
  connect(continuousButton, SIGNAL(toggled(bool)), this, SLOT(toggleContinuousMode(bool)));

  nTrainsLayout->addWidget(continuousButton);

  layout->addLayout(nTrainsLayout,1,1);

  layout->addWidget(new QLabel("Inter-train Interval"), 3, 0,
      Qt::AlignRight | Qt::AlignVCenter);
  trainIntervalSpinBox = new QSpinBox();
  trainIntervalSpinBox->setAlignment(Qt::AlignRight);
  trainIntervalSpinBox->setSuffix(" ms");
  trainIntervalSpinBox->setRange(500, 60000);
  trainIntervalSpinBox->setToolTip("Time in milliseconds from the onset of one \npulse/pulse sequence to the onset of the next.");
  layout->addWidget(trainIntervalSpinBox, 3, 1);
  layout->addWidget(new QLabel("[pulse onset to pulse onset]"), 3, 2,
      Qt::AlignRight | Qt::AlignVCenter);

  stimSingleButton = new QPushButton("Trigger Single Output");
  layout->addWidget(stimSingleButton,5,0);
  stimSingleButton->setToolTip("Trigger a single output.\n (ignore value set of 'Number of outputs'/Continuous).");

  startStimButton = new QPushButton("Start Output");
  layout->addWidget(startStimButton,5,1);
  startStimButton->setToolTip("Start digital or analog output");

  abortStimButton = new QPushButton("Abort Output Sequence");
  layout->addWidget(abortStimButton,5,2);
  abortStimButton->setToolTip("End stimulation program.");
  abortStimButton->setEnabled(false);

  /* Signals from the trigger, start, and abort buttons are
   * connected to slots in  spikeFSGUI */

  trainCounter = 0;
  nTrains = 0;

  setLayout(layout);

}

void StimPulseTrainTab::updateActiveAOut(int aOutIndex, int aO1Mode,
    int aO2Mode) 
{

  /* updatet the local analog out selector without sending a signal */
  aOutSelectComboBox->blockSignals(true);
  aOutSelectComboBox->setCurrentIndex(aOutIndex);
  aOutSelectComboBox->blockSignals(false);

  aOut1Mode = aO1Mode;
  aOut2Mode = aO2Mode;
  updateAOutEnable();
}

void StimPulseTrainTab::updateAOutEnable()
{
  /* if either is in continuous mode we disable the ntrains buttons and the single
   * stim button;  FIX -- separate control of the two outputs */
  if ((aOutSelectComboBox->currentIndex() != 0) && 
      ((aOut1Mode == FS_AO_MODE_CONTINUOUS) ||
      (aOut2Mode == FS_AO_MODE_CONTINUOUS))) {
    continuousButton->setEnabled(false);
    nTrainsSpinBox->setEnabled(false);
    trainIntervalSpinBox->setEnabled(false);
    stimSingleButton->setEnabled(false);
  }
  else {
    continuousButton->setEnabled(true);
    nTrainsSpinBox->setEnabled(true);
    trainIntervalSpinBox->setEnabled(true);
    stimSingleButton->setEnabled(true);
  }
}


void StimPulseTrainTab::startStimulation(int count)
{
  abortStimButton->setEnabled(true);
  stimSingleButton->setEnabled(false);
  startStimButton->setEnabled(false);

  nTrainsSpinBox->setEnabled(false);
  continuousButton->setEnabled(false);
  trainIntervalSpinBox->setEnabled(false);

  nTrains = nTrainsSpinBox->value();
  qDebug("\nnTrains is %d\n\n",nTrains);
  if (count == -1)
    nTrainsSpinBox->setSuffix(" (continuous)");
  if (count >= 0)
    nTrainsSpinBox->setSuffix(QString(" of %1").arg(count+1));
  trainCounter = 0;
  nTrainsSpinBox->setValue(trainCounter);
}

void StimPulseTrainTab::endStimulation(int flag)
{
  continuousButton->setEnabled(true);
  if (!continuousButton->isChecked()) {
    nTrainsSpinBox->setEnabled(true);
  }
  trainIntervalSpinBox->setEnabled(true);

  nTrainsSpinBox->setValue(nTrains);
  nTrainsSpinBox->setSuffix("");

  stimSingleButton->setEnabled(true);
  startStimButton->setEnabled(true);
  abortStimButton->setEnabled(false);
  updateAOutEnable();
}

void StimPulseTrainTab::stepStimulation(int count)
{
  trainCounter++;
  nTrainsSpinBox->setValue(trainCounter);
  qDebug("Got step: %d\n",count);
}

void StimPulseTrainTab::writeStateToFile(QFile *)
{
}

void StimPulseTrainTab::readStateFromFile(QFile *)
{
}

void StimPulseTrainTab::saveToXML(QDomDocument &doc, QDomElement &rootNode) {
    QDomElement sConf = doc.createElement("pulseTrainConfiguration");
    rootNode.appendChild(sConf);

    sConf.setAttribute("stimulatorSelected", stimulatorSelectComboBox->currentIndex());
    sConf.setAttribute("aOutSelected", aOutSelectComboBox->currentIndex());
    sConf.setAttribute("trainInterval",trainIntervalSpinBox->value());
    sConf.setAttribute("nTrains",nTrainsSpinBox->value());
    sConf.setAttribute("continuous",continuousButton->isChecked());
}

int StimPulseTrainTab::loadFromXML(QDomNode &stimNode) {

    int error;
    QDomElement sConf = stimNode.toElement();

    stimulatorSelectComboBox->setCurrentIndex(sConf.attribute("stimulatorSelected").toInt());
    aOutSelectComboBox->setCurrentIndex(sConf.attribute("aOutSelected").toInt());
    trainIntervalSpinBox->setValue(sConf.attribute("trainInterval").toInt());
    nTrainsSpinBox->setValue(sConf.attribute("nTrains").toInt());
    continuousButton->setChecked((bool)sConf.attribute("continuous").toInt());
    return 1;
}


