/****************************************************************************
** Form implementation generated from reading ui file 'stimform.ui'
**
** Created: Tue Apr 7 22:58:19 2009
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/


#include <fsGUI.h>



/*
 *  Constructs a StimForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
StimForm::StimForm( QWidget* parent)
    : QWidget( parent)
{
    QGridLayout *grid = new QGridLayout(this);

    grid->addWidget(new QLabel("Pulse Length (100 us)",this), 1, 1, 1, 2);
    pulseLengthSpinBox = new QSpinBox(this);
    pulseLengthSpinBox->setRange(1, 20000);
    grid->addWidget(pulseLengthSpinBox, 1, 1, 3, 3);
    connect(pulseLengthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateStimData(void)));


    grid->addWidget(new QLabel("# Pulses in Train",this), 2, 2, 1, 1);
    nPulsesSpinBox = new QSpinBox(this);
    nPulsesSpinBox->setRange(1, 1000);
    grid->addWidget(nPulsesSpinBox, 2, 2, 2, 2);
    connect(nPulsesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateStimData(void)));
    connect(nPulsesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(ablePulseTrain(void)));

    grid->addWidget(new QLabel("pre Delay",this), 3, 3, 1, 1);
    preDelaySpinBox = new QSpinBox(this);
    preDelaySpinBox->setRange(1, 1000);
    grid->addWidget(preDelaySpinBox, 3, 3, 2, 2);
    connect(preDelaySpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateStimData(void)));
    connect(preDelaySpinBox, SIGNAL(valueChanged(int)), this, SLOT(ablePulseTrain(void)));

    grid->addWidget(new QLabel("Frequency (Hz)",this), 4, 4, 2, 2);
    frequencySpinBox = new QDoubleSpinBox(this);
    frequencySpinBox->setRange(0.1, 200);
    grid->addWidget(frequencySpinBox, 4, 4, 3, 3);
    frequencySpinBox->setEnabled(false);
    connect(frequencySpinBox, SIGNAL(valueChanged(double)), this, SLOT(frequencyChanged(void)));

    grid->addWidget(new QLabel("Period (100 us)",this), 4, 4, 4, 4);
    periodSpinBox = new QSpinBox(this);
    periodSpinBox->setRange(50, 10000);
    grid->addWidget(periodSpinBox, 4, 4, 5, 5);
    periodSpinBox->setEnabled(false);
    connect(periodSpinBox, SIGNAL(valueChanged(int)), this, SLOT(periodChanged(void)));


    grid->addWidget(new QLabel("# Pulse Trains",this), 6, 6, 1, 1);
    nTrainsSpinBox = new QSpinBox(this);
    nTrainsSpinBox->setRange(1, 50);
    grid->addWidget(nTrainsSpinBox, 6, 6, 2, 2);
    connect(nTrainsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateStimData(void)));
    connect(nTrainsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(ableMultiplePulses(void)));

    continuousButton = new QPushButton("Continuous", this );
    continuousButton->setCheckable(true);
    grid->addWidget(continuousButton, 6, 6, 4, 4);
    connect(continuousButton, SIGNAL(isDown(bool)), this, SLOT(ableMultiplePulses(void)));
    connect(continuousButton, SIGNAL(isDown(bool)), nTrainsSpinBox, SLOT(setDisabled(bool)));
    connect(continuousButton, SIGNAL(isDown(bool)), this, SLOT(updateStimData(void)));

    grid->addWidget(new QLabel("Inter-Pulse Interval (100 us)",this), 7, 7, 2, 3);
    interPulseIntervalSpinBox = new QSpinBox(this);
    interPulseIntervalSpinBox->setRange(1, 600000);
    interPulseIntervalSpinBox->setValue(50000);
    grid->addWidget(interPulseIntervalSpinBox, 7, 7, 4, 4);
    interPulseIntervalSpinBox->setEnabled(false);
    connect(interPulseIntervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateStimData(void)));

    pulseStart = new QPushButton( "Start", this );
    pulseStart->setCheckable(true);
    grid->addWidget(pulseStart, 12, 12, 1, 2);
    connect(pulseStart, SIGNAL(isDown(bool)), this, SLOT(startPulseSeq(bool)));

    pulseStop = new QPushButton( "Stop", this );
    pulseStop->setCheckable(true);
    grid->addWidget(pulseStop, 12, 12, 4, 5);
    connect(pulseStop, SIGNAL(isDown(bool)), this, SLOT(stopPulseSeq(bool)));


}

/*
 *  Destroys the object and frees any allocated resources
 */
StimForm::~StimForm()
{
    // no need to delete child widgets, Qt does it all for us
}

int StimForm::generateSingleStim(char *stimCommand)
{
  int len;

  if (nPulsesSpinBox->value() == 1) {
    len = sprintf(stimCommand, "p %d\n", pulseLengthSpinBox->value());
  }
  else {
    len = sprintf(stimCommand, "s %d %lf %d\n", pulseLengthSpinBox->value(),
    frequencySpinBox->value(), nPulsesSpinBox->value());
  }

  return len;
}

void StimForm::updateStimData(void)
{
  // create pulse array
  int commandLength = 0;
  int len;
  int i;
  char stimCommand[100];
  char pulseCommands[2000];

  memset(stimCommand,0,100);
  memset(pulseCommands,0,2000);

  len = generateSingleStim(stimCommand);

  len = sprintf(pulseCommands,"%s",stimCommand);
  commandLength += len;
  if (continuousButton->isDown()) {
      len = sprintf(pulseCommands+commandLength, "d %d\n", interPulseIntervalSpinBox->value());
      commandLength += len;
      len = sprintf(pulseCommands+commandLength, "r\n");
      commandLength += len;
  }
  else if (nTrainsSpinBox->value() > 1) {
    for (i = 0; i < nTrainsSpinBox->value()-1; i++) {
      len = sprintf(pulseCommands+commandLength, "d %d\n", interPulseIntervalSpinBox->value());
      commandLength += len;
      len = sprintf(pulseCommands+commandLength, "%s",stimCommand);
      commandLength += len;
    }
  }

  fprintf(stderr, "This is file:(%d)\n%s", commandLength, pulseCommands);

  //SendMessage(digioinfo.outputfd, FS_PULSE_SEQ, (char *) pulseCommands,
  //  commandLength * sizeof(char));
  
  return;
}

void StimForm::ableMultiplePulses(void)
{
  if ((nTrainsSpinBox->value() > 1) || continuousButton->isDown())
    interPulseIntervalSpinBox->setEnabled(true);
  else
    interPulseIntervalSpinBox->setEnabled(false);
    return;
}

void StimForm::ablePulseTrain(void)
{
  if (nPulsesSpinBox->value() > 1) {
    frequencySpinBox->setEnabled(true);
    periodSpinBox->setEnabled(true);
  }
  else {
    frequencySpinBox->setEnabled(false);
    periodSpinBox->setEnabled(false);
  }
    return;
}

void StimForm::frequencyChanged(void)
{
  periodSpinBox->blockSignals(true);
  periodSpinBox->setValue((int) (10000/frequencySpinBox->value()));
  periodSpinBox->blockSignals(false);
  updateStimData();
  return;
}

void StimForm::periodChanged(void)
{
  frequencySpinBox->blockSignals(true);
  frequencySpinBox->setValue(10000/periodSpinBox->value());
  frequencySpinBox->blockSignals(false);
  updateStimData();
    return;
}

void StimForm::startPulseSeq(bool on)
{
    if (on) {
        /*FIX */
        //if (digioinfo.outputfd) {
        if (1) {
            fprintf(stderr,"Sending file\n");
          updateStimData();
                fprintf(stderr,"Sending enable\n");
            //SendDAQFSMessage(FS_RT_ENABLE, NULL, 0);
            //SendMessage(digioinfo.outputfd, FS_PULSE_SEQ_START, NULL, 0);
            pulseStart->setStyleSheet("foreground-color: green");
            pulseStart->setText("Running");
            pulseStop->setStyleSheet("foreground-color: black");
            pulseStop->setText("Stop");
            /* Move the cursor to the beginning of the file and highlight the
             * first line */
            pulseStop->setDown(false);
        }
        else {
            QMessageBox::warning(this,"No FS Program","No user program is currently running");
            pulseStart->setDown(false);
        }
    }
}

void StimForm::stopPulseSeq(bool on)
{
    if (on) {
        if (1) {
            /* FIX */
        //if (digioinfo.outputfd) {
            //SendMessage(digioinfo.outputfd, FS_PULSE_SEQ_STOP, NULL, 0);
            pulseStart->setStyleSheet("foreground-color: black");
            pulseStart->setText("Start");
            pulseStop->setStyleSheet("");
            pulseStop->setStyleSheet("foreground-color: red");
            pulseStop->setText("Stopped");
            pulseStart->setDown(false);
            //SendDAQFSMessage(FS_RT_DISABLE, NULL, 0);
        }
        else {
            QMessageBox::warning(this, "No FS Program",
                "No user program is currently running");
            pulseStop->setDown(false);
        }
    }
}

void StimForm::stopPulseSeq(void)
{
    stopPulseSeq(true);
}
