#ifndef __SPIKE_FSDATA_H__
#define __SPIKE_FSDATA_H__

/* below are the default sizes for the data buffer in spike_fsdata.cpp.  Note
 * that these can be overridden on the spike_fsdata command line */
#define FS_DATA_BUFFER_SIZE 	10000000  // the size of each buffer in spike_fsdata
#define FS_DATA_NUM_BUFFERS	10 // the default number of buffers in the circularly linked list

#define FS_DATA_SOCKET_NAME		"/tmp/user_data_spike_socket"

/* structure defining data types to send for export to spike_fsdata*/
typedef struct _FSDataInfo {
    bool	sendcont;  // 1 if we are supposed to send continuous data
    bool	sendspike;  // 1 if we are supposed to send spike data
    bool	sendpos;  // 1 if we are supposed to send position data
    bool	senddigio;  // 1 if we are supposed to send digital IO data
    bool	contelect[MAX_ELECTRODE_NUMBER]; // 1 for continuous electrode that should be sent to spike_fsdata  This will cause the continuous system to send whichever channel of the selected electrode out to spike_fsdata
    bool	spikeelect[MAX_ELECTRODE_NUMBER]; // 1 for spiking electrodes that should be sent out to spike_fsdata
    int 	ncont;  // the number of continuous electrodes selected
    int 	nspike;  // the number of spike electrodes selected
    int 	contnum[MAX_ELECTRODE_NUMBER];  // the numbers for the selected continuous electrodes, in order
    int 	spikenum[MAX_ELECTRODE_NUMBER];  // the numbers for the selected continuous electrodes, in order
} FSDataInfo;

typedef struct _FSDataContBuffer {
    u32         timestamp;
    short       samprate;
    short       nchan;
    short       nsamp;
    short	channum[MAX_CHAN_PER_DSP];  
    short       electnum[MAX_CHAN_PER_DSP];
    short       data[MAX_CONT_BUF_SIZE]; 
} FSDataContBuffer;

typedef SpikeBuffer FSSpikeDataBuffer;
typedef DIOBuffer FSDIODataBuffer;

typedef struct _FSDataBufferInfo {
    FSDataInfo	fsdatainfo;
    int		nspikes[MAX_ELECTRODE_NUMBER];
    int		maxspiketetnum;
    u32		conttimes[2];
    int		ncontsamp[MAX_ELECTRODE_NUMBER];
    int		maxconttetnum;
    int		nposbuf;
    int		ndigiobuf;
} FSDataBufferInfo;

#endif
