#ifndef __SPIKE_FS_CONFIGURE_AOUT_H__
#define __SPIKE_FS_CONFIGURE_AOUT_H__

#include <QtGui>
#include <QtWidgets>
#include <QtXml>
#include <fsSharedStimControlDefines.h>

#define DEFAULT_TMP_PULSE_COMMANDS_FILE "/tmp/tmp_pulse_commands"

class AOutContinuousMode : public QWidget {
  Q_OBJECT

public:
  AOutContinuousMode(QWidget *parent = 0);

  QSpinBox *contPercentSpinBox;
};


class AOutPulseMode : public QWidget {
  Q_OBJECT

public:
  AOutPulseMode(QWidget *parent = 0);

  QDoubleSpinBox *pulseLengthSpinBox;
  QSpinBox *nPulsesSpinBox;
  QGroupBox *multiPulseGroup;
  QDoubleSpinBox *sequenceFrequencySpinBox;
  QDoubleSpinBox *sequencePeriodSpinBox;
  QSpinBox *pulsePercentSpinBox;

  PulseCommand aOutPulseCmd;

public slots:
  void pulseChanged(void);
  void frequencyChanged(void);
  void periodChanged(void);
  void ablePulseSequence(void);

signals:
  void aOutPulseCmdChanged(void);
};


class AOutWaveMode : public QWidget {
  Q_OBJECT

public:
  AOutWaveMode(QWidget *parent = 0);
  QPushButton *rampButton;
  QPushButton *sineButton;
  QSpinBox     *maxPercentSpinBox;
  QSpinBox     *lengthSpinBox;
  QPushButton  *continuousButton;

private slots:
  void waveChanged(void);

signals:
  void aOutWaveChanged(void);
};




class AOutConfigureWidget : public QWidget
{
  Q_OBJECT

public:
  AOutConfigureWidget(QWidget *parent = 0);
  PulseCommand aOutPulseCmd;
  int loadFromXML(QDomElement &nt);
  void saveToXML(QDomDocument &doc, QDomElement &aConf);

  //bool isHardwareReady();


public slots:
  void setAOutMode(int);
  void setAOutRange(int);
  void updateAOutPulseCmd(void);

private:
  QComboBox *aOutRangeSelectBox;
  QDoubleSpinBox *aOutRangeMinSpinBox;
  QDoubleSpinBox *aOutRangeMaxSpinBox;
  QSpinBox *aOutTriggerBitSpinBox;
  QComboBox *aOutModeComboBox;
  QStackedWidget *aOutModeStack;

  AOutContinuousMode *aOutContinuousMode;
  AOutPulseMode *aOutPulseMode;
  AOutWaveMode *aOutWaveMode;
  
signals:
  void aOutModeChangedSignal(void);
};


class AOutConfigTab : public QWidget
{
  Q_OBJECT

public:
  AOutConfigTab(QWidget *parent = 0);
  int loadFromXML(QDomNode &aOutConfNode);
  void saveToXML(QDomDocument &doc, QDomElement &rootNode);

  int activeAOut;
  AOutConfigureWidget *aOut1Config;
  AOutConfigureWidget *aOut2Config;

public slots:
  void selectAOut(void);
  void setActiveAOut(int);
  void aOutModeChanged(void);

signals:
  void activeAOutChanged(int, int, int);

private:
  QPushButton *aOut1Button;
  QPushButton *aOut2Button;

protected:
};
#endif
