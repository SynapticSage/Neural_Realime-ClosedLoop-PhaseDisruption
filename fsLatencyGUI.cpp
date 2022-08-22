/*
 * spikeFSGUI.cpp: the qt object for user program interfaces
 *
 * Copyright 2008 Loren M. Frank
 *
 * This program is part of the nspike data acquisition package.
 * nspike is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * nspike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nspike; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "fsGUI.h"




LatencyTest::LatencyTest (QWidget *parent)
  : QWidget(parent)
{
    QString s;

    QGridLayout *grid = new QGridLayout(this);

/*    grid->addMultiCellWidget(new QLabel("Tetrode / channel", this), 0, 0, 0, 4);
    StimChan = new QComboBox( FALSE, this, "Channel Combo Box" );
    StimChan->insertStringList(*(daq_io_widget->ChannelStrings));
    grid->addMultiCellWidget(StimChan, 0, 0, 3, 3);
    connect(StimChan, SIGNAL(activated( int )), daq_io_widget, SLOT(updateChan(int)));
    connect(daq_io_widget, SIGNAL(updateChanDisplay(int)), this, SLOT(changeStimChanDisplay(int)));
    triggeredStart = new QPushButton("Start", this, "start");
    triggeredStart->setToggleButton(TRUE);
    triggeredStart->setEnabled(FALSE);
    grid->addWidget(triggeredStart, 9, 9, 1, 1);
    connect(triggeredStart, SIGNAL(toggled(bool)), this, 
      SLOT(startTest(bool)));

    triggeredStop = new QPushButton("Stop", this, "stop");
    triggeredStop->setToggleButton(TRUE);
    grid->addMultiCellWidget(triggeredStop, 9, 9, 4, 4); 
    connect(triggeredStop, SIGNAL(toggled(bool)), this, 
      SLOT(stopTest(bool)));

    grid->addMultiCellWidget(new QLabel("Threshold", this), 7, 7, 0, 2);
    thresh = new QSpinBox (0, 65535, 1, this);
    thresh->setValue(65535);
    grid->addMultiCellWidget(thresh, 7, 7, 3, 3);
    connect(thresh, SIGNAL(valueChanged(int)), this, SLOT(updateLatencyData(void))); */

}

void LatencyTest::updateLatencyData(void)
{
  LatencyTestParameters data;

  //daq_io_widget->updateChan(StimChan->currentItem());
  data.thresh = thresh->value();
  data.pulse_length = 10;

  SendDAQFSMessage(DIO_SET_RT_LATENCY_TEST_PARAMS, (char *) &data, sizeof(LatencyTestParameters));
  triggeredStart->setEnabled(TRUE);
}

void LatencyTab::startTest(bool on)
{
  if (on) {
    updateLatencyData();
    SendDAQFSMessage(DIO_RT_ENABLE, NULL, 0);
    SendMessage(digioinfo.outputfd, DIO_LATENCY_TEST_START, NULL, 0);
    triggeredStart->setOn(false);
    triggeredStart->setPaletteForegroundColor("green");
    triggeredStart->setText("Running");
    triggeredStop->setPaletteForegroundColor("black");
    triggeredStop->setText("Stop");
    triggeredStop->setOn(false);
  }
}

void LatencyTab::stopTest(bool on)
{
  if (on) {
    SendMessage(digioinfo.outputfd, DIO_LATENCY_TEST_STOP, NULL, 0);
    SendDAQFSMessage(DIO_RT_DISABLE, NULL, 0);
    triggeredStart->setPaletteForegroundColor("black");
    triggeredStart->setText("Start");
    triggeredStop->setPaletteForegroundColor("red");
    triggeredStop->setText("Stopped");
    triggeredStart->setOn(false);
  }
}
