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


#include "fsgui.h"


extern DigIOInfo digioinfo;
extern SocketInfo *server_message;

ThetaTab::ThetaTab (QWidget *parent)
  : QWidget(parent)
{
    QString s;

    Q3GridLayout *grid = new Q3GridLayout(this, 10, 6, 20, -1, "grid 1");

/*    grid->addMultiCellWidget(new QLabel("Tetrode / channel", this), 0, 0, 0, 4);
    StimChan = new QComboBox( FALSE, this, "Channel Combo Box" );
    StimChan->insertStringList(*(daq_io_widget->ChannelStrings));
    grid->addMultiCellWidget(StimChan, 0, 0, 3, 3);
    connect(StimChan, SIGNAL(activated(int)), this, SLOT(updateThetaData(void)));
    connect(daq_io_widget, SIGNAL(updateChanDisplay(int)), this, SLOT(changeStimChanDisplay(int))); */

    grid->addMultiCellWidget(new QLabel("Pulse Length (100 us units)", this), 1, 1, 0, 2);
    pulse_len = new QSpinBox (0, 10000, 1, this, "Pulse Length");
    pulse_len->setValue(DIO_RT_DEFAULT_PULSE_LEN);
    grid->addMultiCellWidget(pulse_len, 1, 1, 3, 3);
    connect(pulse_len, SIGNAL(valueChanged(int)), this, SLOT(updateThetaData(void)));

    grid->addMultiCellWidget(new QLabel("Velocity Threshold (pixels/sec)", this), 2, 2, 0, 2);
    vel_thresh = new QSpinBox (0, 1000, 1, this, "Velocity Threshold");
    vel_thresh->setValue(DIO_RT_DEFAULT_THETA_VEL);
    grid->addMultiCellWidget(vel_thresh, 2, 2, 3, 3);
    connect(vel_thresh, SIGNAL(valueChanged(int)), this, SLOT(updateThetaData(void)));

    grid->addMultiCellWidget(new QLabel("Filter Delay (msec)", this), 3, 3, 0, 2);
    filt_delay = new QSpinBox (0, 1000, 1, this, "Filter Delay");
    filt_delay->setValue(DIO_RT_DEFAULT_THETA_FILTER_DELAY);
    grid->addMultiCellWidget(filt_delay, 3, 3, 3, 3);
    connect(filt_delay, SIGNAL(valueChanged(int)), this, SLOT(updateThetaData(void)));

    grid->addMultiCellWidget(new QLabel("Desired phase of stimulation (deg)", this), 4, 4, 0, 2);
    theta_phase = new QSpinBox (0, 1000, 1, this, "Desired Phase");
    grid->addMultiCellWidget(theta_phase, 4, 4, 3, 3);
    connect(theta_phase, SIGNAL(valueChanged(int)), this, SLOT(updateThetaData(void)));

    triggeredStart = new QPushButton("Start", this, "start");
    triggeredStart->setToggleButton(TRUE);
    triggeredStart->setEnabled(FALSE);
    grid->addMultiCellWidget(triggeredStart, 9, 9, 1, 1);
    connect(triggeredStart, SIGNAL(toggled(bool)), this, 
	    SLOT(startThetaStim(bool)));

    triggeredStop = new QPushButton("Stop", this, "stop");
    triggeredStop->setToggleButton(TRUE);
    grid->addMultiCellWidget(triggeredStop, 9, 9, 4, 4); 
    connect(triggeredStop, SIGNAL(toggled(bool)), this, 
	    SLOT(stopThetaStim(bool)));
}

void ThetaTab::updateThetaData(void)
{
  ThetaStimParameters data;

  data.pulse_length = pulse_len->value();
  data.vel_thresh = vel_thresh->value();
  data.filt_delay = filt_delay->value();
  data.stim_phase = theta_phase->value();

  /* This needs to be updated for the new spike_fsdata */
//  SendDAQFSMessage(DIO_SET_RT_THETA_PARAMS, (char *) &data, sizeof(ThetaStimParameters));
//
  triggeredStart->setEnabled(TRUE);
}

void ThetaTab::startThetaStim(bool on)
{
  if (on) {
    updateThetaData();
    SendMessage(server_message[SPIKE_FS_DATA].fd, DIO_THETA_STIM_START, NULL, 0);
    triggeredStart->setOn(false);
    triggeredStart->setPaletteForegroundColor("green");
    triggeredStart->setText("Running");
    triggeredStop->setPaletteForegroundColor("black");
    triggeredStop->setText("Stop");
    triggeredStop->setOn(false);
  }
}

void ThetaTab::stopThetaStim(bool on)
{
  if (on) {
    SendMessage(server_message[SPIKE_FS_DATA].fd, DIO_THETA_STIM_STOP, NULL, 0);
    triggeredStart->setPaletteForegroundColor("black");
    triggeredStart->setText("Start");
    triggeredStop->setPaletteForegroundColor("red");
    triggeredStop->setText("Stopped");
    triggeredStart->setOn(false);
  }
}

