
/*
 * fsdata.cpp: Program for getting data from trodes  for
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

#include <spikecommon.h>
#include <spike_fs_defines.h>

/* function definitions that allow us to use spike_dsp_shared.h */
void DisplayErrorMessage(char *);
void DisplayStatusMessage(char *);



char tmpstring[200];





SocketInfo 		server_message[MAX_CONNECTIONS]; // the structure for the server messaging
SocketInfo 		client_message[MAX_CONNECTIONS]; // the structure for the client messaging
SocketInfo 		server_data[MAX_CONNECTIONS]; // the structure for receiving data
SocketInfo 		client_data[MAX_CONNECTIONS]; // the structure for sendin data (currently unused)

#include "spike_dsp_shared.h"
#include "../src-main/spike_dsp_shared.cpp"

void fsdataexit(int status);
void Usage(void);


int main(int argc, char **argv) 
{
  char		savebuf[SAVE_BUF_SIZE];
  int		savebufsize;
  short		datatype;
  int		nspikes;

  int		maxfds;
  fd_set	readfds; 

  char 		tmpstring[100];	// a temporary buffer 
  char 		filename[100];	
  int         	message;	// a temporary message variable
  int		messagedata[MAX_BUFFER_SIZE]; // message data can contain a sysinfo or channelinfo structure
  int		messagedatalen; // the length of the data in the message
  int 		i, id, j, nxtarg;

  struct timeval	tval, lasttval;
  struct timezone	tzone;
  int			fd;
  int 		datalen;
  int 		nwritten;
  int 		totalwritten;
  int 		writesize;

  SysInfo		*systmp;
  u32			*u32ptr;


  sysinfo.program_type = SPIKE_FS_DATA;

  fprintf(STATUSFILE, "spike_fsdata: starting\n");

  sysinfo.statusfile == NULL;
  if (STATUSFILE == NULL) {
	/* open up the status file if it is not stderr*/
	gethostname(tmpstring,80);
	sprintf(filename, "spike_fsdata_status_%s", tmpstring);
	if ((STATUSFILE = fopen(filename, "w")) == NULL) {
	  fprintf(stderr, "spike_fsdata: error opening status file\n");
	  exit(-1);
	}
  }

  /* process the arguments */
  sysinfo.acq = 0;
  sysinfo.fsdataon = 0;

  if (StartNetworkMessaging(server_message, client_message, server_data, 
		client_data) < 0) {
    fprintf(STATUSFILE, "spike_fsdata: Error starting network data messaging\n");
	fsdataexit(1);
  }

  /* get the current time */
  gettimeofday(&lasttval, &tzone);
  
  while (1) { 
    /* set up the initial list of file descriptors to watch */
    SetupFDList(&readfds, &maxfds, server_message, server_data);
    select(maxfds, &readfds, NULL, NULL, NULL);
    id = 0;
    /* check for incoming data. Note that we save all of the incoming data
     * to the data buffer, as the individual programs that send us data
     * have already filtered it */
    while ((i = netinfo.datainfd[id]) != -1) {
      if (FD_ISSET(server_data[i].fd, &readfds)) {
	message = GetMessage(server_data[i].fd, savebuf, &savebufsize, 
		1);
	if (sysinfo.fsdataon) {
	  switch(i) {
	    case SPIKE_DAQ:
	      if (message == SPIKE_DATA) {
		datatype = SPIKE_DATA_TYPE;
		/* get the number of each electrode and increment
		 * the number of spikes on them 
		nspikes = savebufsize / sizeof(SpikeBuffer);
		sptr = (FSDataSpikeBuffer *) savebuf; */
	      }
	      else if (message == CONTINUOUS_DATA) {
		datatype = CONTINUOUS_DATA_TYPE;
	      }
	      else if (message == DIGITALIO_EVENT) {
		datatype = DIGITALIO_DATA_TYPE;
	      }
	      break;
	    case SPIKE_POSDAQ:
	      datatype = POSITION_DATA_TYPE;
	      break;
	  }
	  ProcessData(datatype, savebuf, savebufsize); 
	}
      }
      id++;
    }
    id = 0;
    /* check for messages */
    while ((i = netinfo.messageinfd[id]) != -1) {
      if (FD_ISSET(server_message[i].fd, &readfds)) {
	 message = GetMessage(server_message[i].fd,
				(char *) messagedata, &messagedatalen, 0);
	switch(message) {
	  case STOP_ACQUISITION:
	     sysinfo.acq = 0;
	     SendMessage(client_message[SPIKE_MAIN].fd, ACQUISITION_STOPPED,
		 NULL, 0);
	     break;
	  case START_ACQUISITION:
	     sysinfo.acq = 1;
	     SendMessage(client_message[SPIKE_MAIN].fd, ACQUISITION_STARTED, 			     NULL, 0);
	     break;
	  case FS_DATA_INFO:
	     /* get the fsdatainfo structure */
	     memcpy((char *) &fsdatainfo, messagedata,
		    sizeof(FSDataInfo));
	     /* we also need to reset any relevant variables */
	     ResetFSStatus();
	     break;
	  case FS_DATA_START:
	     sysinfo.fsdataon = 1;
	     break;
	  case FS_DATA_STOP:
	     sysinfo.fsdataon = 0;
	     /* move on to the next buffer so that we can start
	     * fresh */
	     break;
	  case SYSTEM_INFO:
	     systmp = (SysInfo *) messagedata;
	     sysinfo.maxelectnum = systmp->maxelectnum;
	     break;
	  case DIGIO_INFO:
	     /* copy the DigIOInfo structure */
	     memcpy(&digioinfo, messagedata, sizeof(DigIOInfo));
	     break;
	  case EXIT:
	     fsdataexit(0);		    
	     break;
	  default:
	     /* Send the message on to the ProcessMessage function */
	     ProcessMessage(message, (char *) messagedata, messagedatalen);
	     break;
	}
      }
      id++;
    }
  }
  return 0;
}

void fsdataexit(int status)
{
   SendMessage(client_message[SPIKE_MAIN].fd, EXITING, NULL, 0);
   /* sleep so that all of the other programs have a chance to get the message*/
   sleep(1);
   CloseSockets(server_message);
   CloseSockets(client_message);
   CloseSockets(server_data);
   fclose(STATUSFILE);
   exit(status);
}

void DisplayErrorMessage(char *message)
{
  fprintf(stderr, "spike_fsdata error: %s\n", message);
}

void DisplayStatusMessage(char *message)
{
  fprintf(stderr, "spike_fsdata status: %s\n", message);
}
#include <stdio.h>

int main(int argc, char *argv[])
{
     
}
