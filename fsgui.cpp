/*
 * fsGUI.cpp: the qt object for feedback stimulation (fs) program interfaces
 *
 * Copyright 2014 Loren M. Frank
 *
 * This program is part of the nspike data acquisition package.
 * this is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * this is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nspike; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "fsGUI.h"


int PythonConfig::loadFromXML(QDomNode &pythonNode) {

    pythonFSData = bool(pythonNode.toElement().attribute("pythonFSData").toInt());

    return 1;
}

void PythonConfig::saveToXML(QDomDocument &doc, QDomElement &rootNode)
{
    QDomElement pythonConf = doc.createElement("pythonConfiguration");

    rootNode.appendChild(pythonConf);
    pythonConf.setAttribute("pythonFSData", pythonFSData);
}



int HPCConfig::loadFromXML(QDomNode &hpcConfNode) {

}

void HPCConfig::saveToXML(QDomDocument &doc, QDomElement &rootNode){
}

FSGui::FSGui(QStringList arguments, QWidget* parent) : QMainWindow(parent)
{

    this->setWindowTitle("Stimulation Control");

    //QRect r(0, 0, 600, 400);
    //this->setGeometry(r);

    int optionInd = 1;
    trodes = false;
    trodesClientStarted = false;
    dataStreaming = false;
    moduleNet = new TrodesModuleNetwork();
    configFileName = "";

    fsDataPath = "";
    while (optionInd < arguments.length()) {
        if ((arguments.at(optionInd).compare("-trodesConfig", Qt::CaseInsensitive) == 0) && (arguments.length() > optionInd + 1)) {
            nsParseTrodesConfig(arguments.at(++optionInd));
            qDebug() << "FSGui parsing trodesConfig file " << arguments.at(optionInd);
            trodes = 1;
        }
        else if ((arguments.at(optionInd).compare("-serverAddress", Qt::CaseInsensitive) == 0) && (arguments.length() > optionInd + 1)) {
            moduleNet->trodesServerHost = arguments.at(++optionInd);
            trodes = 1;
        }
        else if ((arguments.at(optionInd).compare("-serverPort", Qt::CaseInsensitive) == 0) && (arguments.length() > optionInd + 1)) {
            moduleNet->trodesServerPort = (quint16)(arguments.at(++optionInd).toInt());
        }
        else if ((arguments.at(optionInd).compare("-FSData", Qt::CaseInsensitive) == 0) && (arguments.length() > optionInd + 1)) {
            fsDataPath = arguments.at(++optionInd);
        }
        else if ((arguments.at(optionInd).compare("-config", Qt::CaseInsensitive) == 0) && (arguments.length() > optionInd + 1)) {
            configFileName = arguments.at(++optionInd);
        }
        optionInd++;
    }
    if ((moduleNet->trodesServerPort == 0) && (trodes == 1)) {
        // try setting this from the network config file
        moduleNet->trodesServerHost = networkConf->trodesHost;
        moduleNet->trodesServerPort = networkConf->trodesPort;
    }
    else if (trodes == 0) {
        qDebug() << "Error: FSGui must be run with trodes";
        exit(1);
    }

    /* Create a menu bar */
    fileMenu = menuBar()->addMenu(tr("&File"));

    /* create the actions */
    loadFileAction = new QAction(tr("&Open Config File"), this);
    loadFileAction->setShortcuts(QKeySequence::Open);
    loadFileAction->setStatusTip(tr("Open a FSGui configuration file"));
    connect(loadFileAction, SIGNAL(triggered()), this, SLOT(loadSettings()));

    saveFileAction = new QAction(tr("&Save Config File"), this);
    saveFileAction->setShortcuts(QKeySequence::Save);
    saveFileAction->setStatusTip(tr("Save a FSGui configuration file"));
    connect(saveFileAction, SIGNAL(triggered()), this, SLOT(saveSettings()));

    openLatencyFileAction = new QAction(tr("Open Latency File"), this);
    openLatencyFileAction->setStatusTip(tr("Open a file to save latency test data"));
    connect(openLatencyFileAction, SIGNAL(triggered()), this, SLOT(openLatencyDataFile()));

    closeLatencyFileAction = new QAction(tr("Close Latency File"), this);
    closeLatencyFileAction->setStatusTip(tr("Close the latency test data file"));
    connect(closeLatencyFileAction, SIGNAL(triggered()), this, SLOT(closeLatencyDataFile()));

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Exit the GUI"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    fileMenu->addAction(loadFileAction);
    fileMenu->addAction(saveFileAction);
    fileMenu->addAction(openLatencyFileAction);
    fileMenu->addAction(closeLatencyFileAction);
    fileMenu->addAction(exitAction);

    closeLatencyFileAction->setEnabled(false);

    /* Data menu */
    dataMenu = menuBar()->addMenu(tr("&Data"));

    /* Help menu */
    menuHelp = new QMenu;
    menuHelp->setTitle("Help");
    actionAbout = new QAction(this);
    actionAbout->setText("About");
    actionAbout->setMenuRole(QAction::AboutRole);
    menuHelp->addAction(actionAbout);
    menuBar()->addAction(menuHelp->menuAction());
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));

    // Create the NTrode Selection Dialogs.  These will not be shown until the appropriate menu item is selected
    contDataDialog = new NTrodeSelectDialog("Continuous Data NTrodes", true, this);
    connect(contDataDialog, SIGNAL(nTrodeSelected(qint16, bool)), this, SLOT(updateContNTrodeSelection(qint16, bool)));
    connect(moduleNet, SIGNAL(moduleDataChanUpdated(int, int)), contDataDialog, SLOT(updateNTrodeList(int, int)));
    spikeDataDialog = new NTrodeSelectDialog("Spike Data NTrodes", false, this);
    connect(spikeDataDialog, SIGNAL(nTrodeSelected(qint16, bool)), this, SLOT(updateSpikeNTrodeSelection(qint16, bool)));

    selectContinuousDataAction = new QAction(tr("Select Continuous Data"), this);
    selectContinuousDataAction->setStatusTip(tr("Select the nTrodes whose continuous data will be send to FSData"));
    connect(selectContinuousDataAction, SIGNAL(triggered()), contDataDialog, SLOT(show()));

    selectSpikeDataAction = new QAction(tr("Select Spike Data"), this);
    selectSpikeDataAction->setStatusTip(tr("Select the nTrodes whose spike data will be send to FSData"));
    connect(selectSpikeDataAction, SIGNAL(triggered()), spikeDataDialog, SLOT(show()));
    //selectSpikeDataAction->setEnabled(false); // CHANGE When needed

    restartFSDataAction = new QAction(tr("Restart FSData"), this);
    connect(restartFSDataAction, SIGNAL(triggered()), this, SLOT(startFSData()));

    // connect the fsDataRestarted signal to the data dialogs and the filters so that if we restart fsData, we send the configuration
    // data to the new fsData
//<<<<<<< HEAD
//    connect(this, SIGNAL(sig_fsDataRestarted()), contDataDialog, SLOT(updateNTrodeList()));
//    connect(this, SIGNAL(sig_fsDataRestarted()), spikeDataDialog, SLOT(updateNTrodeList()));
//=======
    //connect(this, SIGNAL(sig_fsDataRestarted()), feedbackTab, SLOT(updateAllFilters()));
    //connect(this, SIGNAL(sig_fsDataRestarted()), contDataDialog, SLOT(updateNTrodeList()));
    //connect(this, SIGNAL(sig_fsDataRestarted()), spikeDataDialog, SLOT(updateNTrodeList()));
//>>>>>>> origin

    startDataAction = new QAction(tr("Start data to FSData"), this);
    connect(startDataAction, SIGNAL(triggered()), this, SLOT(startData()));
    stopDataAction = new QAction(tr("Stop data to FSData"), this);
    connect(stopDataAction, SIGNAL(triggered()), this, SLOT(stopData()));
    stopDataAction->setEnabled(false);


    dataMenu->addAction(selectContinuousDataAction);
    dataMenu->addAction(selectSpikeDataAction);
    dataMenu->addSeparator();
    dataMenu->addAction(startDataAction);
    dataMenu->addAction(stopDataAction);
    dataMenu->addSeparator();
    dataMenu->addAction(restartFSDataAction);

    qtab = new QTabWidget(this);
//    qtab->setUsesScrollButtons(false);

    stimConfigTab = new StimConfigTab(this);
    qtab->addTab(stimConfigTab, "Digital Out");
    connect(stimConfigTab, SIGNAL(newStateScript(QString*)), moduleNet, SLOT(sendStateScript(QString*)));
    connect(stimConfigTab, SIGNAL(sig_setAutoSettle(bool,int)), this, SLOT(updateAutoSettleSetting(bool,int)));


    aOutConfigTab = new AOutConfigTab(this);
    qtab->addTab(aOutConfigTab, "Analog Out");

    /* Create the FeedbackTab */
    feedbackTab = new FeedbackTab(this);
    qtab->addTab(feedbackTab, "Feedback / Stimulation");
    connect(this, SIGNAL(sig_fsDataRestarted()), feedbackTab, SLOT(updateAllFilters()));

    /* Create Latency Test Tab */
    latencyTab = new LatencyTab(this);
    qtab -> addTab(latencyTab, "Latency Test");
    connect(latencyTab, SIGNAL(newStateScript(QString*)), moduleNet, SLOT(sendStateScript(QString*)));
    connect(latencyTab, SIGNAL(SendFSDataMessage(int,char*,int)), this, SLOT(SendFSDataMessage(int,char*,int)));
    connect(latencyTab, SIGNAL(startFSDataStream()), this, SLOT(startData()));
    connect(latencyTab, SIGNAL(stopFSDataStream()), this, SLOT(stopData()));


    connect(stimConfigTab, SIGNAL(stimulatorsReady(bool)), feedbackTab, SLOT(updateStimulatorStatus(bool)));
    connect(stimConfigTab, SIGNAL(stateScriptFunctionValid(uint16_t,bool)), this, SLOT(setStateScriptFunctionValid(uint16_t,bool)));
    // TO DO: add the same update for the Analog out tab.

    connect(feedbackTab, SIGNAL(SendFSDataMessage(int, char*, int)), this, SLOT(SendFSDataMessage(int, char*, int)));
    connect(feedbackTab->resetFeedbackButton, SIGNAL(clicked()), this, SLOT(resetRealtimeStim()));
    connect(feedbackTab->startFeedbackButton, SIGNAL(clicked()), this, SLOT(startRealtimeStim()));
    connect(feedbackTab->stopFeedbackButton, SIGNAL(clicked()), this, SLOT(stopRealtimeStim()));
    connect(contDataDialog, SIGNAL(nTrodeSelected(qint16, bool)), feedbackTab->rippleFilt, SLOT(setBaselineValuesSelect(qint16,bool)));

    mainLayout = new QGridLayout();
    mainLayout->addWidget(qtab);

    QWidget *window = new QWidget();
    window->setLayout(mainLayout);
    setCentralWidget(window);

    // start a new server for fsData and start fsData
    fsDataServer = new TrodesServer();
    fsDataServer->startLocalServer("fsGUI main");
    // when FSData connects, we start the main Trodes client and setup the local message handler
    // for messages from FSData; note that in these functions we check to see if FSData was restarted
    connect(fsDataServer, SIGNAL(clientConnected()), this, SLOT(startTrodesClient()));
    connect(fsDataServer, SIGNAL(clientConnected()), this, SLOT(setupFSDataMessageHandler()));

    show();

    // start FSData
    fsDataProcess = NULL;
    fsDataRestarted = false;
    startFSData();

}

FSGui::~FSGui()
{

//    qDebug() << "Check to see if fsDataProcess is running and not exited:";
//    qDebug() << "   Q_PID() = " << fsDataProcess->pid();
//    qDebug() << "   State = " << fsDataProcess->state();
//    qDebug() << "   atEnd() = " << fsDataProcess->atEnd();
    qDebug("Exiting.");


    SendFSDataMessage(TRODESMESSAGE_QUIT, NULL, 0);
    if (fsDataProcess->waitForFinished(3000) == false && fsDataProcess->state() != QProcess::NotRunning) {
        QMessageBox messageBox;
        messageBox.critical(0, "Error", QString(fsDataPath + " could not be stopped."));
    }
    if (latencyFile.isOpen()) latencyFile.close();
}

void FSGui::startFSData()
{
    // FSData is not a true Trodes module (different args, e.g.), so we won't
    // use trodesSocket::startSingleModule to launch it.

    QStringList argList;
    argList << "-hostName" << fsDataServer->getCurrentAddress() << "-port" << QString("%1").arg(fsDataServer->serverPort());

    if (fsDataPath.length() > 0) {
        // check to see if the fsDataProcess has been started or not
        if (fsDataProcess == NULL) {
            fsDataProcess = new QProcess(this);
            // make sure that outputs to to stdout and stderr
            fsDataProcess->setProcessChannelMode(QProcess::ForwardedChannels);
        }
        else if (fsDataServer->messageHandlers.length() > 0) {
            // fsData is running, so we send it an exit command
            SendFSDataMessage(TRODESMESSAGE_QUIT, NULL, 0);
            // now we wait for it to end
            if (fsDataProcess->waitForFinished(3000) == false) {
                QMessageBox messageBox;
                messageBox.critical(0, "Error", QString(fsDataPath + " could not be stopped. Fix and restart"));
            }

        }

        // Now start FSData
        qDebug() << "Launching fsData " << fsDataPath << " : " << ", arglist" << argList;

        QFileInfo fsdataFileInfo(fsDataPath);
        QString fsDataAbsPath;

        //Get the directory containing the calling executable (FSGui, in this case)
        QString callingAppDir = QCoreApplication::applicationDirPath();

    #ifdef __APPLE__
        //If calling app (FSGui) is an app bundle, use bundle location as the base directory to find FSData
        if (callingAppDir.endsWith(QString(".app/Contents/MacOS")))
            callingAppDir = QDir::cleanPath(callingAppDir + "/../../../");
    #endif

        // Convert relative path to absolute path (relative to calling executable)
        if (fsdataFileInfo.isRelative()){
            fsDataAbsPath = callingAppDir + "/" + fsDataPath;
            qDebug() << "'fsData' string (" << fsDataPath << ") converted to absolute path: " << fsDataAbsPath;
        }else{
            fsDataAbsPath = fsDataPath;
            if (!globalConf->suppressModuleAbsPathWarning){
                QMessageBox::warning(
                            0, "error", QString(
                                "fsData module file location is specified with an absolute path: \n\n<Argument flag=\"-FSData\" value=\""
                                + fsDataPath + "\"/>\""
                                "\" \n\nThis is strongly discouraged since it can lead to "
                                "running old versions of modules. Instead, use a "
                                "relative path for both FSGui and FSData. (Paths are relative "
                                "to directory containing the launching application, in this case: '"
                                + callingAppDir  + QDir::separator() + "'). \n\n "
                                "When in doubt, use bare moduleNames in your config file, e.g.: \n\n"
                                "   <SingleModuleConfiguration moduleName=\"FSGui\" sendTrodesConfig=\"1\" sendNetworkInfo=\"1\">\n"
                                "   <Argument flag=\"-FSData\" value=\"FSData\"/>\n\n"
                                "(Suppress this warning with suppressModuleAbsPathWarning=\"1\" "
                                "in GlobalConfiguration)."));
            }
	    qDebug() << "FSData module path specified as an absolute path (not recommended): " << fsDataPath;
        }
        //update FileInfo
        fsdataFileInfo.setFile(fsDataAbsPath);

        if (!fsdataFileInfo.exists()) {
                QMessageBox messageBox;
                messageBox.critical(0, "Error", QString("Module \"FSData\" could not be started at path: \"%1\".\n\nFile does not exist.").arg(fsDataAbsPath));
                qDebug() << "Error: Can't load FSData module. Config string: " << fsDataPath\
                         << ". Launching application directory: " << callingAppDir << ". Module search location (file doesn't exist): " << fsDataAbsPath;
        }

        fsDataProcess->start(fsDataAbsPath, argList);
        // check that the launch worked
        if (fsDataProcess->waitForStarted(10000) == false) {
            QMessageBox messageBox;
            messageBox.critical(0, "Error", QString(fsDataAbsPath + " could not be started. Fix and restart"));
        }
        // note that if FSData was restarted, the fsDataRestarted flag will be set in startTrodesClient
    }
    else {
        QMessageBox messageBox;
        messageBox.critical(0, "Error", QString("Run FSData manually with arguments " + argList.join(" ")));
    }

}

void FSGui::startData()
{
    // send a message to trodes that we've enabled data so trodes can disable changes in the module Data channels

    if ((contDataDialog->numberSelected() == 0) && (spikeDataDialog->numberSelected() == 0)) {
        QMessageBox::warning(this, "No nTrodes Selected", "At least one nTrode must be selected to stream data.\nNote that latency testing requires continuous data");
        return;
    }
    restartFSDataAction->setEnabled(false);
    if (~dataStreaming){
        moduleNet->trodesClient->sendMessage(TRODESMESSAGE_TURNONDATASTREAM);
        // send a message to FSData to tell it to start the data streams
        qDebug() << "FSGui: start data stream message to FSData";
        SendFSDataMessage(TRODESMESSAGE_TURNONDATASTREAM, NULL, 0);
        selectContinuousDataAction->setEnabled(false);
        selectSpikeDataAction->setEnabled(false);
        stopDataAction->setEnabled(true);
        startDataAction->setEnabled(false);

        loadFileAction->setEnabled(false);
        dataStreaming = true;
    }
}

void FSGui::stopData()
{
    restartFSDataAction->setEnabled(true);
    if (dataStreaming) {
        moduleNet->trodesClient->sendMessage(TRODESMESSAGE_TURNOFFDATASTREAM);
        // send a message to FSData to tell it to stop the data streams
        SendFSDataMessage(TRODESMESSAGE_TURNOFFDATASTREAM, NULL, 0);
        selectContinuousDataAction->setEnabled(true);
        selectSpikeDataAction->setEnabled(true);
        stopDataAction->setEnabled(false);
        startDataAction->setEnabled(true);
        loadFileAction->setEnabled(true);

        dataStreaming = false;
    }
}

void FSGui::startTrodesClient()
{
    // this is called either when FSGui is first starting and FSData connects or when FSData restarts
    dataTypesConnected = 0;
    numContNTrodesConnected = 0;
    numSpikeNTrodesConnected = 0;

    if (!trodesClientStarted) {
        moduleNet->dataNeeded = TRODESDATATYPE_CONTINUOUS | TRODESDATATYPE_SPIKES | TRODESDATATYPE_DIGITALIO;
        //moduleNet->dataNeeded = TRODESDATATYPE_CONTINUOUS;

        if (hpcConfig.hpcEnabled) {
            moduleNet->dataNeeded |= hpcConfig.dataType;
        }
        //moduleNet->dataNeeded = TRODESDATATYPE_CONTINUOUS;
        // check to see if one of the modules is a cameraModule, in which case we add position data
        if (moduleConf->modulePresent("cameraModule")) {
            qDebug() << "FSGui adding position data type";
            moduleNet->dataNeeded |= TRODESDATATYPE_POSITION;
        }

        // we need to set up our own sockets for data
        moduleNet->useQTSocketsForData = false;
        moduleNet->moduleName = "FSGui";
        connect(moduleNet, SIGNAL(quitReceived()), this, SLOT(close()));
        connect(moduleNet, SIGNAL(stateScriptFunctionRange(int, int)), this, SLOT(setStateScriptFunctionRange(int,int)));

        connect(moduleNet, SIGNAL(startDataClient(DataTypeSpec*, int)), this, SLOT(startDataSockets(DataTypeSpec*, int)));
        qDebug() << "FSGUI trying to connect to trodes server on" << moduleNet->trodesServerHost.toLatin1() << moduleNet->trodesServerPort;
        moduleNet->trodesClientConnect(moduleNet->trodesServerHost, moduleNet->trodesServerPort, false);

        connect(moduleNet->trodesClient, SIGNAL(moduleDataChanUpdated(int, int)), contDataDialog, SLOT(updateNTrodeList(int, int)));

        // Setup moduleNet message handlers
        connect(moduleNet->trodesClient, SIGNAL(openFileEventReceived(QString)), this, SLOT(setupFile(QString)));
        connect(moduleNet->trodesClient, SIGNAL(startAquisitionEventReceived()), this, SLOT(startRecording()));
        connect(moduleNet->trodesClient, SIGNAL(stopAquisitionEventReceived()), this, SLOT(stopRecording()));
        connect(moduleNet->trodesClient, &TrodesClient::closeFileEventReceived, this, &FSGui::closeFile);


        trodesClientStarted = true;
    }
    else {

        fsDataRestarted = true;
        // this is also called when fsData is restarted, so we need to tell trodes to send the data available again
        //moduleNet->trodesClient->sendMessage(TRODESMESSAGE_DATATYPEAVAILABLE);
        moduleNet->trodesClient->sendMessage(TRODESMESSAGE_SENDDATATYPEAVAILABLE);
    }
    // disable the Data menu items for continuous or spike data if we do not need those data types
    if (!(moduleNet->dataNeeded | TRODESDATATYPE_CONTINUOUS)) {
        selectContinuousDataAction->setEnabled(false);
    }
    if (!(moduleNet->dataNeeded | TRODESDATATYPE_SPIKES)) {
        selectSpikeDataAction->setEnabled(false);
    }



    // Whether or not we use nTrode data, we tell FSData how many nTrodes there are
    int numNTrodes = spikeConf->ntrodes.length();
    SendFSDataMessage(TRODESMESSAGE_NUMNTRODES, (char*)&numNTrodes, sizeof(int));
    int pointsInWaveform = POINTSINWAVEFORM;
    // also send the number of points in each spike waveform and, for each nTrode, the number of channels for that nTrode
    SendFSDataMessage(TRODESMESSAGE_NUMPOINTSPERSPIKE, (char *) &pointsInWaveform, sizeof(int));
    int nTrodeInfo[numNTrodes];
    for (int i; i < numNTrodes; i++) {
        nTrodeInfo[i] = spikeConf->ntrodes[i]->hw_chan.length(); // number of channels
    }
    SendFSDataMessage(TRODESMESSAGE_NUMCHANPERNTRODE, (char *) nTrodeInfo, 2 * numNTrodes * sizeof(int));
}

void FSGui::setupFSDataMessageHandler() {
    // a message from FSData will first go to the main message handler, but since the message type is not specified there, it will
    // result in a dataReceived signal that we can process
    connect(fsDataServer->messageHandlers.last(), SIGNAL(messageReceived(quint8,QByteArray)), this, SLOT(processFSDataMessage(quint8,QByteArray)));
}


void FSGui::processFSDataMessage(quint8 messageType, QByteArray message) {

    // process a message from FSData
    switch(messageType) {
    //qDebug() << "message from FSData, type" << messageType << 'size' << message.length();
    case FS_RT_STATUS: {
        QString status(message);
        //qDebug() << "got FS_RT_STATUS, string =" << status;

        // put the message up in the appropriate status box
        switch (message[0]) {
        case FS_STIM_STATUS:
            feedbackTab->statusGroupBox->setTitle(status.remove(0,1));
            break;
        case FS_RIPPLE_STATUS:
            feedbackTab->updateRippleStatus(status.remove(0,1));
            break;
        case FS_SPATIAL_STATUS:
            feedbackTab->updateSpatialStatus(status.remove(0,1));
            break;
        case FS_THETA_STATUS:
            feedbackTab->updateThetaStatus(status.remove(0,1));
            break;
        case FS_PHASE_STATUS:
            feedbackTab->updatePhaseStatus(status.remove(0,1));
            break;
        case FS_LATENCY_STATUS:
            latencyTab->updateLatencyStatus(status.remove(0,1));
           break;
        }
        break;
    }
    case FS_LATENCY_DATA:
        // this is a u32 timestamp that needs to be written to the latency file if it's open
        if (latencyFile.isOpen()) {
            latencyStream << message.toUInt() << endl;
        }
        break;
    case FS_SET_CUSTOM_RIPPLE_BASELINE_MEAN:
        //qDebug() << "FSGui: Received FS_SET_CUSTOM_RIPPLE_BASELINE_MEAN" << message.length() << "bytes";
        feedbackTab->rippleFilt->updateBaselineValuesListMean((double *)message.data());
        break;
    case FS_SET_CUSTOM_RIPPLE_BASELINE_STD:
        //qDebug() << "FSGui: Received FS_SET_CUSTOM_RIPPLE_BASELINE_STD";
        feedbackTab->rippleFilt->updateBaselineValuesListStd((double *)message.data());
        break;
    }

}

void FSGui::updateContNTrodeSelection(qint16 nTrodeIndex, bool selected)
{
    // send a message to FSData to enable or disable the nTrode data socket
    char messageData[3];

    //qDebug() << "FSGui sending message to FSData to enable nTrode with index " << nTrodeIndex;
    memcpy(messageData, &nTrodeIndex, sizeof(qint16));
    messageData[2] = (char)selected;
    SendFSDataMessage(TRODESMESSAGE_ENABLECONTDATASOCKET, messageData, 3 * sizeof(char));
}

void FSGui::updateSpikeNTrodeSelection(qint16 nTrodeIndex, bool selected)
{
    // send a message to FSData to enable or disable the nTrode data socket
    char messageData[3];

    memcpy(messageData, &nTrodeIndex, sizeof(qint16));
    messageData[2] = (char)selected;

    SendFSDataMessage(TRODESMESSAGE_ENABLESPIKEDATASOCKET, messageData, 3 * sizeof(char));
}

void FSGui::SendFSDataMessage(int message, char *data, int size)
{
    if (fsDataServer->messageHandlers.length() > 0) {
        fsDataServer->messageHandlers[0]->sendMessage(message, data, size);
    }
    return;
}

void FSGui::startDataSocket(DataClientInfo *dc)
{
    if ((hpcConfig.hpcEnabled) && (hpcConfig.dataType & dc->dataType)) {
        // TO DO: add coce to send the start data client message to the HPC framework
    }
    else {
        SendFSDataMessage(TRODESMESSAGE_STARTDATACLIENT, (char*)dc, (uint32_t)sizeof(DataClientInfo));
    }
}

void FSGui::startDataSockets(DataTypeSpec *dataTypeSpec, int dataType)
{
    DataClientInfo dc;

    //qDebug() <<  "FSGui in startDataSockets, dataType" << dataTypeSpec->dataType << "socketType" << dataTypeSpec->socketType << "module" << dataTypeSpec->moduleID << "port" << dataTypeSpec->hostPort << "nTrodes" << dataTypeSpec->contNTrodeIndexList;

    dc.nTrodeId = -1;
    dc.socketType = dataTypeSpec->socketType;
    dc.port = dataTypeSpec->hostPort;
    strcpy(dc.hostName, dataTypeSpec->hostName.toLatin1().data());
    // send a message to fsData telling it to start each client.
    if (dataType & TRODESDATATYPE_ANALOGIO) {
        dc.dataType = TRODESDATATYPE_ANALOGIO;
        if (dataTypeSpec->socketType == TRODESSOCKETTYPE_UDP) {
            dc.port = dataTypeSpec->analogIOUDPPort;
        }
        startDataSocket(&dc);
        dataTypesConnected |= TRODESDATATYPE_ANALOGIO;
    }
    if (dataType & TRODESDATATYPE_DIGITALIO) {
        dc.dataType = TRODESDATATYPE_DIGITALIO;
        if (dataTypeSpec->socketType == TRODESSOCKETTYPE_UDP) {
            dc.port = dataTypeSpec->digitalIOUDPPort;
        }
        startDataSocket(&dc);
        dataTypesConnected |= TRODESDATATYPE_DIGITALIO;

        // We also need to tell FSData the maximum digital port numbers for input and output
        int message[2];
        message[0] = headerConf->maxDigitalPort(true);
        message[1] = headerConf->maxDigitalPort(false);
        SendFSDataMessage(TRODESMESSAGE_NDIGITALPORTS, (char *) message, 2*sizeof(int));
    }
    if (dataType & TRODESDATATYPE_POSITION) {
        dc.dataType = TRODESDATATYPE_POSITION;
        startDataSocket(&dc);
        dataTypesConnected |= TRODESDATATYPE_POSITION;
    }
    if (dataType & TRODESDATATYPE_CONTINUOUS) {
        // calculate the desired decimation factor
        uint16_t decimation = hardwareConf->sourceSamplingRate / DECIMATED_SAMPLING_RATE;
        // check to see that this works, and given an error if not
        if (decimation * DECIMATED_SAMPLING_RATE != hardwareConf->sourceSamplingRate) {
            QString Message = QString("Sampling rate of %1 does not permit decimation to %2. You must change the sampling rate in the configuration file to be an integer multiple of %3").arg(hardwareConf->sourceSamplingRate).arg(DECIMATED_SAMPLING_RATE).arg(DECIMATED_SAMPLING_RATE);
            QMessageBox::StandardButton reply;
            reply = QMessageBox::critical(this, tr("Error in sampling rate"), Message, QMessageBox::Abort);
            if (reply == QMessageBox::Abort) {
                close();
            }
        }
        for (int i = 0; i < dataTypeSpec->contNTrodeIndexList.length(); i++) {
            dc.dataType = TRODESDATATYPE_CONTINUOUS;
            dc.nTrodeIndex = dataTypeSpec->contNTrodeIndexList[i];
            dc.nTrodeId = spikeConf->ntrodes[dc.nTrodeIndex]->nTrodeId;
            dc.decimation = decimation;
            if (dataTypeSpec->socketType == TRODESSOCKETTYPE_UDP) {
                dc.port = dataTypeSpec->contNTrodeUDPPortList[i];
            }
            startDataSocket(&dc);
            //qDebug() <<  "FSGUi starting data socket for nTrode Index" << dc.nTrodeIndex <<"port" << dc.port;
        }
        qDebug() <<  "FSGUI cont ntrode list len" << dataTypeSpec->contNTrodeIndexList.length() << spikeConf->ntrodes.length();

        numContNTrodesConnected += dataTypeSpec->contNTrodeIndexList.length();
        if (numContNTrodesConnected == spikeConf->ntrodes.length()) {
            // all continuous NTrodes have been connected
            dataTypesConnected |= TRODESDATATYPE_CONTINUOUS;
        }
    }
    if (dataType & TRODESDATATYPE_SPIKES) {
        // create one client for data from each nTrode
        for (int i = 0; i < dataTypeSpec->spikeNTrodeIndexList.length(); i++) {
            dc.dataType = TRODESDATATYPE_SPIKES;
            dc.nTrodeIndex = dataTypeSpec->spikeNTrodeIndexList.at(i);
            dc.nTrodeId = spikeConf->ntrodes[dc.nTrodeIndex]->nTrodeId;
            if (dataTypeSpec->socketType == TRODESSOCKETTYPE_UDP) {
                dc.port = dataTypeSpec->spikeNTrodeUDPPortList[i];
            }
            startDataSocket(&dc);
        }
        numSpikeNTrodesConnected += dataTypeSpec->spikeNTrodeIndexList.length();
        if (numSpikeNTrodesConnected == spikeConf->ntrodes.length()) {
            // all spike NTrodes have been connected
            dataTypesConnected |= TRODESDATATYPE_SPIKES;
        }
    }
    //qDebug() <<  "FSGui in startDataSockets, datatypesConnected" << dataTypesConnected << moduleNet->dataNeeded;

    // Check to see if all data needed has been provided, and if so, read in the configuration file if it was specified
    if (dataTypesConnected == moduleNet->dataNeeded) {

        // we also need to send FSData the host information for the ECU hardware
        HardwareNetworkInfo hNI;
        strcpy(hNI.address, networkConf->hardwareAddress.toLatin1().data());
        hNI.port = TRODESHARDWARE_ECUDIRECTPORT;
        SendFSDataMessage(TRODESMESSAGE_ECUHARDWAREINFO, (char*)&hNI, sizeof(HardwareNetworkInfo));

        // send FSData host info about the MCU hardware.  Same address but different port.
        HardwareNetworkInfo directHNI;
        strcpy(directHNI.address, networkConf->hardwareAddress.toLatin1().data());
        hNI.port = TRODESHARDWARE_CONTROLPORT;
        SendFSDataMessage(TRODESMESSAGE_MCUHARDWAREINFO, (char*)&hNI, sizeof(HardwareNetworkInfo));


        if (!fsDataRestarted) {
            qDebug() << configFileName;
            // this is the initial startup, so we read in the configuration file
            if (configFileName.length() > 0) {
                readConfigFile(configFileName);
            }
        }
        else {
            // we need to reset our configuration, so we first disable the filters
            stopDataAction->trigger();
            feedbackTab->spatialFilterEnabled->setChecked(false);
            feedbackTab->rippleFilterEnabled->setChecked(false);
            //latencyTab->latencyTestEnabled->setChecked(false);
            latencyTab->stopLatencyTestButton->click();
            // the easiest way to reset our configuration properly for the new FSData is to save the current configuration
            // and the load it, so thats what we do.
            QTemporaryFile file;
            if (file.open()) {
                file.close();
                writeConfigFile(file.fileName());
                readConfigFile(file.fileName());
            }
        }

        //contDataDialog->updateSelected();
    }
}

void FSGui::setStateScriptFunctionRange(int start, int end) {

    // Each stimulator requires two functions
    latencyTab->setStateScriptFunctionNumber(start);
    stimConfigTab->stimConfigA->setScriptFunctionNum(start+2);
    stimConfigTab->stimConfigB->setScriptFunctionNum(start+4);
    // TO DO: set functions for analog IO
}



void FSGui::setStateScriptFunctionValid(uint16_t fNum, bool valid) {
    // send a message to FSData indicating that this function number should or should not be
    // triggered when stimulation is to be delivered.  FSData is hard coded to know that each function number actually
    // corresponds to two functions. The first is the go function and the second the stop function
    //fprintf(stderr,"FSGUI: statescript valid %d %X\n", fNum, fNum);
    char message[3];
    memcpy(message, &fNum, sizeof(uint16_t));
    message[2] = valid;
    //fprintf(stderr, "FSGUI: statescript msg: %u %u %u\n",message[0],message[1],message[2]);
    SendFSDataMessage(TRODESMESSAGE_SETSCRIPTFUNCTIONVALID, message, (uint32_t)sizeof(char)*3);
}

void FSGui::setupFile(QString filename) {
    // Creating FSGui log file
    qDebug() << "FSGUI: setup file: " << filename;
    SendFSDataMessage(FS_CREATE_SAVE_FILE, filename.toLocal8Bit().data(), sizeof(char) * filename.toLocal8Bit().size());
}

void FSGui::startRecording() {
    SendFSDataMessage(FS_START_RECORDING, NULL, 0);
}

void FSGui::stopRecording() {
    SendFSDataMessage(FS_STOP_RECORDING, NULL, 0);

}

void FSGui::closeFile() {
    SendFSDataMessage(FS_CLOSE_SAVE_FILE, NULL, 0);
}

void FSGui::loadSettings(void)
{
    QString configFileName = QFileDialog::getOpenFileName(this,
                                                            QString("Open FSGUI Settings"));

    qDebug() << "FS looking for settings in" << configFileName;
    readConfigFile(configFileName);
}

void FSGui::saveSettings(void)
{
    QString configFileName = QFileDialog::getSaveFileName(this,
                                                            QString("Save Stimulator Settings"));

    qDebug() << "FS saving settings in" << configFileName;
    writeConfigFile(configFileName);
}

void FSGui::enableTabs(bool enable)
{
    if (qtab->isTabEnabled(CONFIG_STIMULATORS_TAB) != enable)
        qtab->setTabEnabled(CONFIG_STIMULATORS_TAB, enable);

    //qDebug("Enabled == %d\n", enable);
}

void FSGui::triggerSingleStim(void)
{
    PulseCommand pCmd[3]; // at most 3 pulse commands are needed


}

void FSGui::startOutputOnlyStim(void)
{

}

void FSGui::abortOutputOnlyStim(void)
{
    qDebug("abortOutputOnlyStim signal received");
    SendFSDataMessage(FS_PULSE_SEQ_STOP, NULL, 0);
}

void FSGui::resetRealtimeStim(void)
{
    qDebug("resetRealtimeStim signal received");
    SendFSDataMessage(FS_RESET_RT_FEEDBACK, NULL, 0);
}

void FSGui::startRealtimeStim(void)
{
    SendFSDataMessage(FS_START_RT_FEEDBACK, NULL, 0);
    /* Now disable the start button and enable the stop button*/
    //qDebug() << "starting realtime stim";
    // test

    // This usleep prevents crashes when using fsDataPy; this needs to be fixed.
    //QThread::usleep(1000000);

    feedbackTab->startFeedbackButton->setEnabled(false);
     //qDebug() << "starting realtime stim 2";
    feedbackTab->stopFeedbackButton->setEnabled(true);
     //qDebug() << "starting realtime stim 3";
    feedbackTab->resetFeedbackButton->setEnabled(false);
     //qDebug() << "starting realtime stim 4";
    feedbackTab->rippleFilterEnabled->setEnabled(false);
 //qDebug() << "starting realtime stim 5";
    feedbackTab->spatialFilterEnabled->setEnabled(false);
     //qDebug() << "starting realtime stim 6";

    // We also need to disable the stimulators so that the values can't be changed
    stimConfigTab->stimConfigA->setEnabled(false);
     //qDebug() << "starting realtime stim 7";
    stimConfigTab->stimConfigB->setEnabled(false);
     //qDebug() << "starting realtime stim 8";

}


void FSGui::stopRealtimeStim(void)
{
    qDebug("stopRealtimeStim signal received");
    SendFSDataMessage(FS_STOP_RT_FEEDBACK, NULL, 0);
    /* Now disable the stop button and enable the start button*/
    feedbackTab->startFeedbackButton->setEnabled(true);
    feedbackTab->stopFeedbackButton->setEnabled(false);

    feedbackTab->resetFeedbackButton->setEnabled(true);
    feedbackTab->rippleFilterEnabled->setEnabled(true);
    feedbackTab->spatialFilterEnabled->setEnabled(true);

    if (stimConfigTab->stimulatorAButton->isChecked()) {
        stimConfigTab->stimConfigA->setEnabled(true);
    }
    if (stimConfigTab->stimulatorBButton->isChecked()) {
        stimConfigTab->stimConfigB->setEnabled(true);
    }
}

int FSGui::sendConfigFile(QString configFileName) {
    QFile file;

    if (!configFileName.isEmpty()) {
        file.setFileName(configFileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << QString("File %1 not found").arg(configFileName);
            return -1;
        }
    }
    QByteArray configContents = file.readAll();
    SendFSDataMessage(FS_CONFIG_FILE, configContents.data(), configContents.size());
}

void FSGui::about() {
    QMessageBox::about(this, tr("About FSGui"), tr(qPrintable(GlobalConfiguration::getVersionInfo())));
}

int FSGui::readConfigFile(QString configFileName)
{
    qDebug() << "FSGUI: Reading Config File " << configFileName;
    QDomDocument doc("TrodesConf");
    QFile file;

    if (!configFileName.isEmpty()) {
        file.setFileName(configFileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << QString("File %1 not found").arg(configFileName);
            return -1;
        }
        qDebug() << "Loading from configuration file " << configFileName;
        QFileInfo fi(configFileName);
        QString ext = fi.suffix();     // ext = "gz"

        //this is a normal xml config file
        if (!doc.setContent(&file)) {
            file.close();
            qDebug("XML didn't read properly.");
            return -1;
        }
    }


    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "Configuration") {
        qDebug("Configuration not root node.");
        return -1;
    }



    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    // PARSE  CONFIGURATION
    QDomNodeList pythonConfigList = root.elementsByTagName("pythonConfiguration");
    QDomNode pythonNode = pythonConfigList.item(0);
    pythonConfig.loadFromXML(pythonNode);

    QDomNodeList hpcConfigList = root.elementsByTagName("hpcConfiguration");
    QDomNode hpcNode = hpcConfigList.item(0);
    hpcConfig.loadFromXML(hpcNode);

    QDomNodeList aOutConfigList = root.elementsByTagName("aOutConfiguration");
    QDomNode aOutNode = aOutConfigList.item(0);
    aOutConfigTab->loadFromXML(aOutNode);

    QDomNodeList stimConfigList = root.elementsByTagName("stimulatorConfiguration");
    QDomNode stimNode = stimConfigList.item(0);
    stimConfigTab->loadFromXML(stimNode);

    QDomNodeList rtList = root.elementsByTagName("feedbackConfiguration");
    QDomNode rtNode = rtList.item(0);
    feedbackTab->loadFromXML(rtNode);

    QDomNodeList nTrodeList = root.elementsByTagName("contNTrodeSelectConfiguration");
    QDomNode nTrodeNode = nTrodeList.item(0);
    contDataDialog->loadFromXML(nTrodeNode);

    nTrodeList = root.elementsByTagName("spikeNTrodeSelectConfiguration");
    nTrodeNode = nTrodeList.item(0);
    spikeDataDialog->loadFromXML(nTrodeNode);


    // If we are using a python version of FSData we send it the fsgui configuration file
    if (pythonConfig.pythonFSData) {
        sendConfigFile(configFileName);
        qDebug() << "[FSGui] Sending config file to FSDataPy";
    }
    return 1;
}

bool FSGui::writeConfigFile(QString configFileName) {
    QDomDocument doc;
    QDomElement root = doc.createElement("Configuration");
    doc.appendChild(root);

    stimConfigTab->saveToXML(doc,root);
    aOutConfigTab->saveToXML(doc,root);
    feedbackTab->saveToXML(doc,root);
    contDataDialog->saveToXML(doc,root,true);
    spikeDataDialog->saveToXML(doc,root,false);


    QFile file(configFileName);

    if ( file.open(QIODevice::WriteOnly) ) {
        QTextStream TextStream(&file);
        QString xmlString = doc.toString() ;
        //doc.save(TextStream, 0);
        QString vers = "<?xml version=\"1.0\"?>";
        TextStream << vers << endl << xmlString;
        file.close();
        return true;
    }
    else {
        return false;
    }
}

void FSGui::openLatencyDataFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Open Latency Data File"),
                                                    "",
                                                    tr("All Files (*)"));

    if (!fileName.isEmpty()) {
        // open the file and create a textStream for it
        latencyFile.setFileName(fileName);
        if (latencyFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            latencyStream.setDevice(&latencyFile);
            closeLatencyFileAction->setEnabled(true);
            openLatencyFileAction->setEnabled(false);

        }
    }
}

void FSGui::closeLatencyDataFile()
{
    latencyFile.close();
    openLatencyFileAction->setEnabled(true);
}

void FSGui::updateAutoSettleSetting(bool autoSettle, int originID) {
//    qDebug() << "Updating Auto Settle.  Setting " << autoSettle << " on ID " << originID;

    char *asStr = (char *) &autoSettle;
    char *idStr = (char*) &originID;
    size_t lenAS = strlen(asStr);
    size_t lenID = strlen(idStr);

    char *message = (char*)malloc((lenAS + lenID + 1));
    if (message) {
        memcpy(message, asStr, lenAS);
        memcpy(message + lenAS, idStr, lenID);
        int totalSize = lenAS + lenID;
        message[totalSize] = '\0';
        totalSize++;
//        qDebug() << "Message to send: " << message;
        SendFSDataMessage(TRODESMESSAGE_SETAUTOSETTLE, message, totalSize);
    }
    else {
        qDebug() << "Error creating message.";
    }
}
