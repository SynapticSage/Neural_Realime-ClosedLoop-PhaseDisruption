/****************************************************************************
** Form interface generated from reading ui file 'stimform.ui'
**
** Created: Tue Apr 7 15:28:31 2009
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef STIMFORM_H
#define STIMFORM_H

#include <QtGui>
#include <QtWidgets>
#include <QtXml>


class StimForm : public QWidget
{
    Q_OBJECT

public:
    StimForm( QWidget* parent = 0);
    ~StimForm();

private:
    QSpinBox *pulseLengthSpinBox;
    QSpinBox *nPulsesSpinBox;
    QSpinBox *preDelaySpinBox;
    QDoubleSpinBox *frequencySpinBox;
    QSpinBox *periodSpinBox;
    QSpinBox *nTrainsSpinBox;
    QSpinBox *interPulseIntervalSpinBox;

    QPushButton *continuousButton;
    QPushButton *pulseStart;
    QPushButton *pulseStop;

   int generateSingleStim(char *stimCommand);
   void stopPulseSeq(void);

protected:

protected slots:
   void updateStimData(void);
   void ablePulseTrain(void);
   void periodChanged(void);
   void frequencyChanged(void);
   void ableMultiplePulses(void);
   void startPulseSeq(bool start);
   void stopPulseSeq(bool stop);

};

#endif // STIMFORM_H
