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

#include "spikecommon.h"
#include "spikeFSGUI.h"
//#include "spike_main.h"
#include <q3buttongroup.h>
#include <qsizepolicy.h>
#include <qtabwidget.h>
#include <qmenubar.h>
#include <q3filedialog.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3TextStream>
#include <QLabel>
#include <Q3PopupMenu>
#include <string>
#include <sstream>
#include <qmessagebox.h>
#include "qinputdialog.h"

extern DigIOInfo digioinfo;


PulseFileTab::PulseFileTab (QWidget *parent)
  : QWidget(parent)
{
    /*
     * =========================================================
     * Drive laser using a file specifying pulses to use
     * =========================================================
     */
    Q3GridLayout *grid0 = new Q3GridLayout(this, 10, 6, 0, 0, "grid 1");

    /* create a menu bar  */
    menuBar = new QMenuBar(this);
    fileMenu = new Q3PopupMenu(this);
    openFileID = fileMenu->insertItem( "&Open File", this, SLOT(open()));
    saveFileID= fileMenu->insertItem( "Save File", this, SLOT(save()));
    closeFileID = fileMenu->insertItem( "&Close File", this, SLOT(close())); 
    sendFileID = fileMenu->insertItem( "&Send File to FS Program", this, SLOT(send())); 
    menuBar->insertItem( "&File", fileMenu );
    grid0->addMultiCellWidget(menuBar, 0, 1, 0, 4);

    textEdit = new Q3TextEdit(this);
    textEdit->setTextFormat( Qt::PlainText ); 
    connect(textEdit, SIGNAL(textChanged()), this, SLOT(disableStart()));
    connect(daq_io_widget, SIGNAL(pulseFileLineExecuted(int)), this, SLOT(lineExecuted(int)));
    
    grid0->addMultiCellWidget(textEdit, 1, 7, 0, 2);

    QLabel *readme = new QLabel("readme", this, 0);
    readme->setText("Text file format: <p> pulse number pulse freq pause");
    grid0->addMultiCellWidget(readme, 4, 8, 4, 5);

    pulseStart = new QPushButton("Start", this, "start");
    pulseStart->setToggleButton(TRUE);
    /* start with this disabled so that we can only start once we've send the
     * text to the user program */
    pulseStart->setEnabled(false);
    connect(pulseStart, SIGNAL(toggled(bool)), this, SLOT(startPulseSeq(bool)));
    grid0->addMultiCellWidget(pulseStart, 9, 9, 1, 1);

    pulseStop = new QPushButton("Stop", this, "stop");
    pulseStop->setToggleButton(TRUE);
    pulseStop->setOn(true);
    connect(pulseStop, SIGNAL(toggled(bool)), this, SLOT(stopPulseSeq(bool)));
    grid0->addMultiCellWidget(pulseStop, 9, 9, 4, 4); 

    fileOpen = 0;
}

void PulseFileTab::openFile(void)
{
    int answer = 1;
    if (fileOpen) {
	 answer = QMessageBox::warning( this, "Overwrite File",
			QString( "File Open.  Close and open new file?" ),
                                "&Yes", "&No", QString::null, 1, 1 );
    }
    if (answer) {
	QString fn = Q3FileDialog::getOpenFileName( QString::null, tr("All Files (*)" ), this );
	if (!fn.isEmpty()) {
	    if (!QFile::exists(fn))
		return;
	    QFile file(fn);
	    if ( !file.open(QIODevice::ReadOnly) )
		return;
	    Q3TextStream ts(&file);
	    QString txt = ts.read();
	    textEdit->setText(txt);
	    textEdit->viewport()->setFocus();
	}
    }
    fileOpen = 1;
}

void PulseFileTab::saveFile(void)
{

   QString fn = Q3FileDialog::getSaveFileName( QString::null, tr( "All Files (*)" ), this );
    QFile file(fn);
    if ( !file.open( QIODevice::WriteOnly ) )
        return;
    Q3TextStream ts( &file );
    ts << textEdit->text();
}

void PulseFileTab::closeFile(void)
{
    textEdit->clear();
    fileOpen = 0;
}

void PulseFileTab::sendFileToFS(void)
{
    if (digioinfo.outputfd) {
	QString t = textEdit->text();
	const char *toFS = t.ascii();
	SendMessage(digioinfo.outputfd, DIO_PULSE_SEQ, (char *) toFS, 
		textEdit->length() * sizeof(char));
  fprintf(stderr,"spikeFSGUI - user program length = %d\n", textEdit->length());
	pulseStart->setEnabled(true);
    }
    else {
	QMessageBox::warning(this,"No FS Program","No user program is currently running");
    }
}

void PulseFileTab::startPulseSeq(bool on)
{
    if (on) {
	if (digioinfo.outputfd) {
	    SendDAQFSMessage(DIO_RT_ENABLE, NULL, 0);
	    SendMessage(digioinfo.outputfd, DIO_PULSE_SEQ_START, NULL, 0);
	    pulseStart->setPaletteForegroundColor("green");
	    pulseStart->setText("Running");
	    pulseStop->setPaletteForegroundColor("black");
	    pulseStop->setText("Stop");
	    /* Move the cursor to the beginning of the file and highlight the
	     * first line */
	    textEdit->setCursorPosition(0, 0);
	    currentLine = 0;
	    textEdit->setSelection(currentLine, 0, currentLine+1, 0);
	    pulseStop->setOn(false);
	}
	else {
	    QMessageBox::warning(this,"No FS Program","No user program is currently running");
	    pulseStart->setOn(false);
	}
    }
}

void PulseFileTab::stopPulseSeq(bool on)
{
    if (on) {
	if (digioinfo.outputfd) {
	    SendMessage(digioinfo.outputfd, DIO_PULSE_SEQ_STOP, NULL, 0);
	    pulseStart->setPaletteForegroundColor("black");
	    pulseStart->setText("Start");
	    pulseStop->setPaletteForegroundColor("red");
	    pulseStop->setText("Stopped");
	    pulseStart->setOn(false);
      SendDAQFSMessage(DIO_RT_DISABLE, NULL, 0);
	}
	else {
	    QMessageBox::warning(this, "No FS Program", 
		    "No user program is currently running");
	    pulseStop->setOn(false);
	}
    }
}

void PulseFileTab::lineExecuted(int line)
{
    /* highlight the next line of text */
    // currentLine++;
    // textEdit->setSelection(currentLine, 0, currentLine+1, 0);
    textEdit->setSelection(line, 0, line+1,0);
}

