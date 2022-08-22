/*
 * fsdata.cpp: Program for getting data hostName trodes  for
 * realtime processing
 *
 *
 *
 * Copyright 2014 Loren M. Frank
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

#include <fsDefines.h>
#include <stimControlGlobalVar.h>
#include <stdio.h>
//#include <QDebug>

// DEBUG LIBRARIES
#include <QFile>
#include <QTextStream>

/* function definitions that allow us to use spike_dsp_shared.h */
void DisplayErrorMessage(char *);
void DisplayStatusMessage(char *);



char tmpstring[200];





SocketInfo 		clientMessage[MAX_SOCKETS]; // the structure for messaging to FSGui
SocketInfo 		clientData[MAX_SOCKETS]; // the structure for getting data
SocketInfo      ECUMessage; // direct shortcut messages to ECU
SocketInfo      MCUMessage; // direct shortcut messages to MCU

int             nDataSockets = 0;

FILE *latencyTestFile = fopen("/tmp/latencyTestData.csv", "w");  // File to write out latency data
FILE *rippleFilterStateFile = fopen("/tmp/rippleFilterState.csv", "w");
FILE *phaseParametersFile = fopen("phaseParameters.csv","w");
FILE *phaseProcessFile = fopen("phaseProcess.csv","w");
FILE *phaseBufferFile = fopen("phaseBuffer.csv","w");

void fsdataexit(int status);
void Usage(void);


int main(int argc, char **argv) 
{
    sleep(5);

  // the port and hostname for the fsGUI program connections

  int		maxfds;
  fd_set	readfds; 

  int       message;	// a temporary message variable
  char 		messageData[MAX_BUFFER_SIZE];
  char      *msgptr;
  // message data can contain a sysinfo or channelinfo structureMAX_NTRODES
  u32		messageDataLen; // the length of the data in the message
  int 		i, id,  nxtarg;
  bool      socketError = 0;

  struct timeval	lasttval;
  struct timezone	tzone;

  QString filename="DataDebug.txt";
  QFile file( filename );QTextStream stream( &file );
  if ( file.open(QIODevice::WriteOnly|QIODevice::Append) )
  {
      stream << "FSdata starts." << endl;
  }

  fprintf(stderr, "fsData: starting\n");

  clientMessage[0].port = 0;
  clientMessage[0].hostName[0] = '\0';
  // Parse the command line
  if (argc > 1) {
      nxtarg = 1;
      while (nxtarg < argc) {
          if (strcmp(argv[nxtarg], "-port") == 0) {
              clientMessage[0].port = (unsigned short) atoi(argv[++nxtarg]);
          }
          else if (strcmp(argv[nxtarg], "-hostName") == 0) {
              strcpy(clientMessage[0].hostName, argv[++nxtarg]);
          }
          nxtarg++;
      }
  }
  if ((clientMessage[0].port == 0) || (strlen(clientMessage[0].hostName) == 0)) {
      fprintf(stderr, "Usage: fsData -port fsGUIServerPort -hostName fsGUIServerHost\n");
      exit(-1);
  }

  fprintf(stderr, "About to get TCPIP socket connection to FSGui server %s, port %d\n", clientMessage[0].hostName, clientMessage[0].port);

  // we first need to set up a client message socket to connect to  FSGui.
  if ((clientMessage[0].fd = GetTCPIPClientSocket(clientMessage[0].hostName, clientMessage[0].port)) == -1) {
      fprintf(stderr, "Error connecting to fsGUI program on host %s port %d\n", clientMessage[0].hostName, clientMessage[0].port);
      exit(-1);
  }

  fprintf(stderr, "Got TCPIP socket connection to FSGui server %s, port %d\n", clientMessage[0].hostName, clientMessage[0].port);
  // zero out the clientData array
  for (i = 0; i < MAX_SOCKETS; i++) {
      clientData[i].fd = -1;
      clientData[i].enabled = false;
  }

  /* get the current time */
  gettimeofday(&lasttval, &tzone);

  // create the ripple filter object
  stream << "Ripple filter object constructor about to be invoked." << endl;
  RTFilter rtf;

  // Setup shortcut messaging to ECU
  // TODO currently hardcoded details.  should be configurable

  
  while (1) { 
     /* set up the initial list of file descriptors to watch
        TODO: replace the select with a poll call*/

    SetupFDList(&readfds, &maxfds, clientMessage, clientData);
    select(maxfds, &readfds, NULL, NULL, NULL);

    id = 0;
    // check for data
    while (clientData[id].fd != -1) {

        //fprintf(stderr, "checking data fd %d, dataType %d\n", clientData[id].fd, clientData[id].dataType);
        if (FD_ISSET(clientData[id].fd, &readfds)) {
//            if( clientData[id].dataType == TRODESDATATYPE_POSITION) {
//                fprintf(stderr, "fsDataMain: camera position data available\n");
//            }
            if (clientData[id].socketType == TRODESSOCKETTYPE_TCPIP) {
                 message = GetMessage(clientData[id].fd,(char *) messageData, &messageDataLen, 0);
            }
            else if (clientData[id].socketType == TRODESSOCKETTYPE_UDP) {
                message = GetUDPMessage(clientData[id].fd,(char *) messageData, &messageDataLen, 0);
            }
            ProcessData(clientData[id].dataType, messageData, messageDataLen, clientData[id].nTrodeIndex, &rtf);
            //fprintf(stderr, "got data fd %d, dataType %d\n", clientData[id].fd, clientData[id].dataType);

        }
        id++;
    }
    // now check for messages
    if (FD_ISSET(clientMessage[0].fd, &readfds)) {
        message = GetMessage(clientMessage[0].fd,(char *) messageData, &messageDataLen, 1);
        //fprintf(stderr, "FSGui got message %d\n", message);
        msgptr = messageData;
        switch(message) {
        case TRODESMESSAGE_ECUHARDWAREINFO: {
            stream << "Trodes ECU HARDWARE MESSAGE." << endl;
            // set the hardware address and open a UDP socket to it
            HardwareNetworkInfo *hNI;
            hNI = (HardwareNetworkInfo *) messageData;
            if ((ECUMessage.fd = GetUDPHardwareClientSocket(hNI->address, hNI->port, &ECUMessage.socketAddress)) == -1) {
                fprintf(stderr, "Error connecting to port for direct ECU control (broadcast) at %s port %d\n",
                        hNI->address, hNI->port);
            }
            break;
        }
        case TRODESMESSAGE_MCUHARDWAREINFO: {
            stream << "Trodes MCU HARDWARE MESSAGE." << endl;
            // set the hardware address and open a UDP socket to it
            HardwareNetworkInfo *directHNI;
            directHNI = (HardwareNetworkInfo *) messageData;
            if ((MCUMessage.fd = GetUDPHardwareClientSocket(directHNI->address, directHNI->port, &MCUMessage.socketAddress)) == -1) {
                fprintf(stderr, "Error connecting to port for direct MCU control (broadcast) at %s port %d\n",
                        directHNI->address, directHNI->port);
            }
            break;
        }

        case TRODESMESSAGE_NUMNTRODES: {
            //stream << "Trodes NumContNTrodes MESSAGE." << endl;
            // allocate space for the given number of nTrodes in the ripple filter structure
            rtf.numNTrodes = * ((int *) messageData);
            rtf.contNTrodeEnabled = (bool *) calloc(rtf.numNTrodes, sizeof(bool));
            rtf.rippleFilter.chanRippleFilter = new SingleChanRippleFilter*[rtf.numNTrodes];
            for (i = 0; i < rtf.numNTrodes; i++ ) {
                rtf.rippleFilter.chanRippleFilter[i] = new SingleChanRippleFilter;
            }
            rtf.rippleFilter.customRippleBaselineMean = (double *) calloc(rtf.numNTrodes, sizeof(double));
            rtf.rippleFilter.customRippleBaselineStd = (double *) calloc(rtf.numNTrodes, sizeof(double));

            // Set phasefilter trode count and allocate space for singlechan objects
            //rtf.phaseFilter.numNTrodes = rtf.numNTrodes;
            rtf.phaseFilter.singlePhaseFilt = new SingleChanPhase[rtf.numNTrodes];

            break;
        }
        case TRODESMESSAGE_STARTDATACLIENT:
            stream << "Trodes START  DATA CLIENT MESSAGE." << endl;
            // the message is a DataClientInfo structure
            DataClientInfo *dc;
            dc = (DataClientInfo *) messageData;
            //fprintf(stderr, "FSData starting socket to %s port %d, dataType %d, socketType %d, nTrodeIndex %d\n",
            //       dc->hostName, dc->port, dc->dataType, dc->socketType, dc->nTrodeIndex);
            if (dc->socketType == TRODESSOCKETTYPE_TCPIP) {
                if ((clientData[nDataSockets].fd = GetTCPIPClientSocket(dc->hostName, dc->port)) == -1) {
                    fprintf(stderr, "Error connecting to trodes data socket on host %s port %d\n",
                            clientData[nDataSockets].hostName, clientData[nDataSockets].port);
                    socketError = true;
                }                              
            }
            else if (dc->socketType == TRODESSOCKETTYPE_UDP) {
                if ((clientData[nDataSockets].fd = GetUDPDataClientSocket(dc->hostName, dc->port, &clientData[nDataSockets].socketAddress)) == -1) {
                    fprintf(stderr, "Error connecting to trodes data socket on host %s port %d\n",
                            clientData[nDataSockets].hostName, clientData[nDataSockets].port);
                    socketError = true;
                }
            }
            if (!socketError) {
                // if this is a TCPIP socket, set this to be dedicated to the appropriate dataType.
                // To do so we send a three character message with the datatype and the nTrode number (if this is a spike or continuous data socket)
                // UDP sockets are created in the host program and are already dedicated.
                char messageData[3];

                messageData[0] = (char) dc->dataType;
                if ((dc->dataType == TRODESDATATYPE_CONTINUOUS) | (dc->dataType == TRODESDATATYPE_SPIKES)) {
                    memcpy(messageData+1, (char *) &dc->nTrodeIndex, sizeof(uint16_t));
                    clientData[nDataSockets].enabled = false;  // disable by default.  This will be enabled by a message from FSGui
                }
                else {
                    // the last two elements are currently unused
                    messageData[1] = messageData[2] = 0;
                    clientData[nDataSockets].enabled = true;  // enabled by default
                }
                if (dc->socketType == TRODESSOCKETTYPE_TCPIP) {
                    SendMessage(clientData[nDataSockets].fd, TRODESMESSAGE_SETDATATYPE, messageData, sizeof(char)*3);
                }
                clientData[nDataSockets].socketType = dc->socketType;
                // send the decimation factor
                //uint16_t decimationSwapped = ByteSwap(dc->decimation);
                memcpy(messageData, &dc->decimation, sizeof(uint16_t));
                SendMessage(clientData[nDataSockets].fd, TRODESMESSAGE_SETDECIMATION, (char *) &dc->decimation, sizeof(uint16_t));
                //fprintf(stderr, "FSData sent decimation on fd %d\n", clientData[nDataSockets].fd);
                clientData[nDataSockets].dataType = dc->dataType;
                clientData[nDataSockets].nTrodeId = dc->nTrodeId;
                clientData[nDataSockets].nTrodeIndex = dc->nTrodeIndex;
                clientData[nDataSockets].port = dc->port;
                clientData[nDataSockets].decimation = dc->decimation;
                strcpy(clientData[nDataSockets].hostName,dc->hostName);
                fprintf(stderr, "FSData connected to trodes data socket %d on host %s port %d datatype %d nTrodeId %d, enabled %d\n", nDataSockets,
                        clientData[nDataSockets].hostName, clientData[nDataSockets].port, dc->dataType, dc->nTrodeId, clientData[nDataSockets].enabled);
                nDataSockets++;
            }
            break;
        case TRODESMESSAGE_ENABLECONTDATASOCKET: {
            stream << "Trodes ENABLE CONT DATA SOCKET." << endl;
            int nTrodeIndex = *((uint16_t *) messageData);
            bool enable = (bool) messageData[2];
            fprintf(stderr, "Got ENABLECONTDATASOCKET message, size = %d, ntrode index = %d,  enable = %d\n", messageDataLen, nTrodeIndex,enable);
            // set the data socket and the filter entry to enabled;
            // find the associated clientData Structure
            for (int i = 0; i < nDataSockets; i++) {
                if ((clientData[i].dataType == TRODESDATATYPE_CONTINUOUS) &&
                      (clientData[i].nTrodeIndex == nTrodeIndex)) {
                    clientData[i].enabled = enable;
                    // this is a convenient time to set the nTrodeId used for status messages
                    rtf.rippleFilter.chanRippleFilter[nTrodeIndex]->nTrodeId = clientData[i].nTrodeId;
                    rtf.phaseFilter.singlePhaseFilt[nTrodeIndex].nTrodeId = clientData[i].nTrodeId;
                    break;
                }
            }
            rtf.contNTrodeEnabled[nTrodeIndex] = enable;
            rtf.rippleFilter.chanRippleFilter[nTrodeIndex]->enabled = enable;
            //enable each phasefilt chan
            rtf.phaseFilter.singlePhaseFilt[nTrodeIndex].enabled = enable;
            break;
        }

        case TRODESMESSAGE_ENABLESPIKEDATASOCKET: {
            stream << "Trodes ENABLED SPIKE DATA SOCKET." << endl;
            int nTrodeIndex = *((uint16_t *) messageData);
            bool enable = messageData[2];
            //fprintf(stderr, "Got ENABLESPIKEDATASOCKET message, size = %d, ntrode index = %d, enable = %d\n", messageDataLen, nTrodeIndex, enable);

            // set the data socket and the filter entry to enable;
            for (int i = 0; i < nDataSockets; i++) {
                if ((clientData[i].dataType == TRODESDATATYPE_SPIKES) &&
                        (clientData[i].nTrodeIndex == nTrodeIndex)) {
                    clientData[i].enabled = enable;
                    break;
                }
            }
            //fprintf(stderr, "done with ENABLESPIKEDATASOCKET message, size = %d, ntrode index = %d, enable = %d\n", messageDataLen, nTrodeIndex, enable);

            break;
        }
        case TRODESMESSAGE_SETSCRIPTFUNCTIONVALID: {
            stream << "Trodes SET SCRIPT FUNCTION VALID." << endl;
            // the message contains a function number and whether it should be triggered
            bool valid, found = false;
            int lastFound = -1;
            uint16_t fNum = *(uint16_t *) messageData;
            //fprintf(stderr, "FSData: func set %d\n", fNum);
            valid = (bool) messageData[2];
            for (int fid = 1; i <= FS_MAX_STATE_SCRIPT_FUNCTIONS; i++) {
                if (rtf.functionNum[fid] == fNum) {
                    rtf.functionValid[fid] = valid;
                    found = true;
                    break;
                }
                else if (rtf.functionNum[fid] != -1) {
                    lastFound = fid;
                }
            }
            if (!found) {
                // add this function number to the end of the list
                rtf.functionNum[lastFound+1] = fNum;
                rtf.functionValid[lastFound+1] = valid;
            }
            break;
        }
        case TRODESMESSAGE_NDIGITALPORTS: {
            stream << "Trodes N DIGITAL PORTS." << endl;
            int *iptr = (int *) messageData;
            rtf.nDigInPorts = iptr[0];
            rtf.nDigOutPorts = iptr[1];
            // allocate space for the port state variables
            rtf.digInState = new bool[rtf.nDigInPorts];
            memset(rtf.digInState, 0, rtf.nDigInPorts * sizeof(bool));
            rtf.digOutState = new bool[rtf.nDigInPorts];
            memset(rtf.digOutState, 0, rtf.nDigOutPorts * sizeof(bool));
            break;
        }
        case TRODESMESSAGE_TURNONDATASTREAM:
            stream << "Trodes Turn On Data Stream." << endl;
            // go through all of the clientData sockets and send a start message to all that are enabled
            fprintf(stderr, "FSData: got start datastream message\n");

            for (i = 0; i < nDataSockets; i++) {
                if (clientData[i].enabled) {
                    fprintf(stderr, "FSData: sending start datastream message on client %d, fd %d\n", i, clientData[i].fd);

                    SendMessage(clientData[i].fd, TRODESMESSAGE_TURNONDATASTREAM, NULL, 0);
                    fprintf(stderr, "FSData: sent start datastream message on client %d, fd %d\n", i, clientData[i].fd);

                }
            }
            break;
       case TRODESMESSAGE_TURNOFFDATASTREAM:
            stream << "Trodes Off Data Stream." << endl;
            // go through all of the clientData sockets and send a stop message to all that are enabled
            for (i = 0; i < nDataSockets; i++) {
                if (clientData[i].enabled) {
                    fprintf(stderr, "FSData: sending stop datastream message on client %d, fd %d\n", i, clientData[i].fd);
                    SendMessage(clientData[i].fd, TRODESMESSAGE_TURNOFFDATASTREAM, NULL, 0);
                }
            }
            break;
        case TRODESMESSAGE_SETAUTOSETTLE: {
//            qDebug() << "Got Auto Settle message";
            bool setAutoSettle = *(bool*)messageData;
//            qDebug() << "Setting auto settle to " << setAutoSettle;
            rtf.autoSettle = setAutoSettle;
            break;
        }
        case TRODESMESSAGE_QUIT:
             fprintf(stderr, "fsData Exiting.\n");
             fsdataexit(0);
             break;
        default:
             ProcessMessage(message, messageData, messageDataLen, &rtf);
             break;
        }
    }
  }

  file.close();

    }

void ProcessMessage(int message, char *messagedata, int messagedatalen, RTFilter *rtf)
{
    QString filename="DataDebug.txt";
    QFile file( filename );QTextStream stream( &file );
    file.open(QIODevice::WriteOnly|QIODevice::Append);

    ushort ecuMsg;
    ushort ecuMsg2;

    switch(message) {
        case FS_SET_RIPPLE_STIM_PARAMS:
            stream << "Setting ripple status" << endl;
            memcpy((char *)&(rtf->rippleFilter.parm), messagedata, sizeof(RippleFilterParameters));
            fprintf(stderr, "Updating ripple filter parameters, enabled = %d\n", rtf->rippleFilter.parm.enabled);
            rtf->resetRealtimeProcessing();
            break;
        case FS_SET_SPATIAL_STIM_PARAMS:
            memcpy((char *)&(rtf->spatialFilter.parm), messagedata, sizeof(SpatialFilterParameters));
            rtf->resetRealtimeProcessing();
            //fprintf(stderr, "Updating spatial filter parameters\n");
            break;
        case FS_SET_PHASE_PARAMS:
             stream << "Setting phase status" << endl;
             memcpy((char *)&(rtf->phaseFilter.parm), messagedata, sizeof(PhaseFiltParameters));
             rtf->resetRealtimeProcessing();
             rtf->phaseFilter.parm.print();
             //rtf->phaseFilter.parm.printCSV(rtf->timestamp);
            break;
        case FS_DIG_OUT_GATE_STATUS:
            rtf->digOutGatePort = *((int *) messagedata);
            break;
        case FS_SET_LATENCY_TEST_PARAMS:
            memcpy((char *)&(rtf->latencyFilter.parm), messagedata, sizeof(LatencyParameters));
            rtf->resetRealtimeProcessing();
            fprintf(stderr, "Updating latency test parameters, Internal (%d, %d, %d, %d); HPC (%d)\n",
                    rtf->latencyFilter.parm.internal.enabled,
                    rtf->latencyFilter.parm.internal.stateScriptFnNum,
                    rtf->latencyFilter.parm.internal.testInterval,
                    rtf->latencyFilter.parm.internal.outputDIOPort,
                    rtf->latencyFilter.parm.hpc.enabled);

            break;
        case FS_QUERY_RT_FEEDBACK_STATUS:
            //fprintf(stderr, "fsdata query status\n");
            rtf->sendStatusUpdate();
            rtf->rippleFilter.sendStatusUpdate(rtf->timestamp - rtf->lastStimTime);
            rtf->spatialFilter.sendStatusUpdate();
            //rtf->thetaFilter.sendStatusUpdate();
            rtf->latencyFilter.sendStatusUpdate();
            rtf->phaseFilter.sendStatusUpdate(rtf->timestamp - rtf->lastStimTime);
            if(rtf->rippleFilter.parm.updateCustomBaseline) {
                //fprintf(stderr, "fsdata update custom baseline\n");
                rtf->rippleFilter.updateCustomRippleBaseline();
                rtf->rippleFilter.sendCustomRippleBaseline();
            }

            break;
        case FS_SET_CUSTOM_RIPPLE_BASELINE_MEAN:
            fprintf(stderr, "FSData updated custom baseline mean\n");
            memcpy((char *)(rtf->rippleFilter.customRippleBaselineMean), messagedata, sizeof(double)*rtf->rippleFilter.numNTrodes);
            break;
        case FS_SET_CUSTOM_RIPPLE_BASELINE_STD:
            fprintf(stderr, "FSData updated custom baseline std\n");
            memcpy((char *)(rtf->rippleFilter.customRippleBaselineStd), messagedata, sizeof(double)*rtf->rippleFilter.numNTrodes);
            break;

        case FS_RESET_RT_FEEDBACK:
            stream << "Reset realtime processing." << endl;
            rtf->resetRealtimeProcessing();
            fprintf(stderr,"FSData: Received Realtime RESET command ......\n");
            break;
        case FS_START_RT_FEEDBACK:
            stream << "Starting feedback" << endl;
            rtf->stimEnabled = true;
            fprintf(stderr,"FSData: Received Realtime START command ......\n");
            break;
        case FS_STOP_RT_FEEDBACK:
            /* stop any ongoing output */
            rtf->stopStimulation(ECUMessage.fd, ECUMessage.socketAddress);
            rtf->stimEnabled = false;
            fprintf(stderr,"FSData: Received Realtime STOP command......\n");
            break;
        case FS_START_LATENCY_TEST:
            rtf->latencyTestEnabled = true;
            fprintf(stderr,"FSData: Received Latency Test START command ......\n");
            break;
        case FS_STOP_LATENCY_TEST:
            /* stop any ongoing output */
            rtf->latencyTestEnabled = false;
            fprintf(stderr,"FSData: Received Latency Test STOP command......\n");
            break;
         case FS_PULSE_SEQ_START:
            //rtf->startStimulation();
            fprintf(stderr,"FSData: Sorry pulse stimulation disabled......\n");
            break;
        case FS_PULSE_SEQ_STOP:
            //rtf->stopStimulation();

 /*           fprintf(stderr,"FSData: Received STOP command, aout_mode = %d\n", nextPulseCmd->aout_mode);
            SendMessage(client_data[SPIKE_MAIN].fd, FS_PULSE_SEQ_EXECUTED, (char *)&messageCode, sizeof(int)); */
            break;
        default:
            break;
    }
    file.close();
}

void fsdataexit(int status)
{
   sleep(1);
   fflush(latencyTestFile);
   fflush(rippleFilterStateFile);
   fclose(latencyTestFile);
   fclose(rippleFilterStateFile);
   fclose(phaseParametersFile);
   fclose(phaseProcessFile);
   fclose(phaseBufferFile);
   CloseSockets(clientMessage);
   CloseSockets(clientData);
   exit(status);
}

int NTrodeSocketIndex(int nTrodeId, uint8_t dataType, const int nDataSockets) {
    // returns the index of the socket carrying continuous data from the specified NTrode
    // find the socket for this nTrode
    for (int i = 0; i < nDataSockets; i++) {
        if ((clientData[i].nTrodeId == nTrodeId) && (clientData[i].dataType == dataType)) {
            return i;
        }
    }
    return -1;
}

int SendFSGUIMessage(uint8_t message, const char *data, uint32_t datalen) {
    return SendMessage(clientMessage[0].fd, message, data, datalen);
}

void DisplayErrorMessage(char *message)
{
  fprintf(stderr, "spike_fsdata error: %s\n", message);
}

void DisplayStatusMessage(char *message)
{
  fprintf(stderr, "spike_fsdata status: %s\n", message);
}



