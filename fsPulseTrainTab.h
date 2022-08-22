#ifndef __SPIKE_FS_OUTPUT_ONLY_H__
#define __SPIKE_FS_OUTPUT_ONLY_H__

#include <QtGui>
#include <QtWidgets>
#include <QtXml>


#define DEFAULT_TMP_PULSE_COMMANDS_FILE "/tmp/tmp_pulse_commands"

class StimPulseTrainTab : public QWidget
{
  Q_OBJECT

public:
  StimPulseTrainTab (QWidget *parent);
  int loadFromXML(QDomNode &outputConfNode);
  void saveToXML(QDomDocument &doc, QDomElement &rootNode);

  int trainCounter;
  int nTrains;
  int aOut1Mode, aOut2Mode;

  QSpinBox *trainIntervalSpinBox;
  QSpinBox *nTrainsSpinBox;
  QPushButton *continuousButton;

  QComboBox *stimulatorSelectComboBox;
  QComboBox *aOutSelectComboBox;
  QPushButton *stimSingleButton;
  QPushButton *startStimButton;
  QPushButton *abortStimButton;


  void startStimulation(int count);

  void writeStateToFile(QFile *stateFile);
  void readStateFromFile(QFile *stateFile);

public slots:
  void toggleContinuousMode(bool isToggled) {nTrainsSpinBox->setEnabled(!isToggled);}
  void endStimulation(int flag);
  void stepStimulation(int count);
  void updateActiveAOut(int aOutIndex, int aOut1Mode, int aOut2Mode);

private:
  void updateAOutEnable(void);

protected:
};



#endif
