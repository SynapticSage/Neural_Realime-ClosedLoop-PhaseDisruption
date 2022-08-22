#ifndef __SPIKE_DEFINES_H__
#define __SPIKE_DEFINES_H__

#define VERSION "2.1.0"

// uncomment to debug without DSPs
// in other words, #ifdef NO_DSP_DEBUG means to not really use DSPs and to build
// a dummy/demo version of nspike -- so comment this out for a final build
//
#define NO_DSP_DEBUG

// uncomment to take data from a non-NSpike DSP source. 
//#define DATA_NO_DSP

// uncomment to debug without video
// in other words, #ifdef NO_VIDEO_DEBUG means to not really use the video
// capture card and to build a dummy/demo version of nspike -- 
// so comment this out for a final build
//
#define NO_VIDEO_DEBUG

// uncomment to produce output of each digitial IO packet 
#define DIO_DEBUG

// indicate where the digital IO boards are.  For the separate DIO DSP, comment
// out the following
//#define DIO_ON_MASTER_DSP

/* some math defines */
#define sqr(x) ((x) * (x))
#define absv(x) ((x) > 0) ? (x) : (-1.0 * (x))
#define RoundUp(x) (((x) > (long) (x)) ? (long) (x) + 1: (long) (x))
#define Fix(x) (((x) > 0) ? ((long) floor(x)) : ((long) ceil(x)))
#define round(x) (((x) > ((int) (x) + 0.5)) ? (int) (x) + 1: (int) (x))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define PI	3.14159265358979
#define TWOPI	6.28318530717959



/* system types */
#define MASTER	 	0
#define SLAVE		1

/* the different types of data/data output, set up so that they can be or'ed 
 * together to support multiple types of acquisition on a single machine. */
#define POSITION        ((unsigned char) 1)
#define SPIKE		((unsigned char) (1<<1))
#define CONTINUOUS	((unsigned char) (1<<2))
#define DIGITALIO	((unsigned char) (1<<3))
#define FSDATA	((unsigned char) (1<<4))

/* the maximum number of DSPS.  Note that if you change this you will have to
 * remove or add definitions to keep the number of defined DSPS equal to
 * MAX_DSPS */
#define MAX_DSPS		13
#define MAX_CHAN_PER_DSP	16
#define MAX_ELECT_PER_DSP	(MAX_CHAN_PER_DSP / NCHAN_PER_ELECTRODE)
#define DSP_BASE_SAMP_RATE	((int) 30000)
/* the desired number of continuous buffers from spike_daq per second */
#define NCONT_BUF_PER_SEC	 50
#define MAX_CONT_BUF_SIZE	((DSP_BASE_SAMP_RATE / NCONT_BUF_PER_SEC) *( MAX_CHAN_PER_DSP + 1) )
#define SAMP_TO_TIMESTAMP	3   //divide by 3 to turn 30KHz timestamps into 100 usec timestamps

/* Local digitial IO Variables */
#define MAX_DIO_PORTS		4

/* defines for the types of programs / boxes for internal and external 
 * messaging. To add DSPS, add them by number and increase the index for the 
 * programs to prevent numerical overlaps. */
/* note that acqusition is started on the highest number programs first, so the
 * programs that do actual data collection should have the lowest numbers */

/* NOTE: the first DSP must be number 0 */
#define DSP0			0
#define DSP1			1
#define DSP2			2
#define DSP3			3
#define DSP4			4
#define DSP5			5
#define DSP6			6
#define DSP7			7
#define DSP8			8
#define DSP9			9

#ifdef DIO_ON_MASTER_DSP
#define DSPDIO			DSP0 // the DSP that handles digital and analog IO
#else
#define DSPDIO			10 // the DSP that handles digital and analog IO
#endif

#define DSP0ECHO		11 // the echo port for DSP0
#define DSPDIOECHO		12 // the echo port for DSPDIO
#define SPIKE_POSDAQ		13
#define	SPIKE_DAQ		14
#define	SPIKE_PROCESS_POSDATA	15
#define	SPIKE_SAVE_DATA		16
#define	SPIKE_FS_DATA		17
#define	SPIKE_MAIN		18
#define MAX_MODULE_ID		18

#define NAUDIO_OUTPUTS		2  // there are currently 2 analog outputs on the DSPs

/* the name of the status file. This can be set to stderr to output to the
 * terminal or sysinfo.statusfile to create a local file for each program*/
//#define STATUSFILE	sysinfo.statusfile
#define STATUSFILE	stderr

/* defines for the three types of sockets */
#define UNIX		0
#define TCPIP		1
#define UDP		2

/* we'll assume that we'll need no more than 100 ports */
#define MAX_NETWORK_PORTS	100

#define DEFAULT_STRING_LENGTH	100

#define BIT0    0x0001
#define BIT1    0x0002
#define BIT2    0x0004
#define BIT3    0x0008
#define BIT4    0x0010
#define BIT5    0x0020
#define BIT6    0x0040
#define BIT7    0x0080
#define BIT8    0x0100
#define BIT9    0x0200
#define BIT10   0x0400
#define BIT11   0x0800
#define BIT12   0x1000
#define BIT13   0x2000
#define BIT14   0x4000
#define BIT15   0x8000

#define DEFAULT_CONFIG_FILE		"nspikeconfig"
#define DEFAULT_NETCONFIG_FILE      "nspikenetworkconfig"
#define DEFAULT_COLOR_FILE      "/usr/local/nspike/nspike_rgbcolor"

#define  DATASOCKET_WRITE_BUFFER_SIZE       8388600 // the size for data socket write buffers
#define  MAX_SOCKET_WRITE_SIZE		65536 // the largest single write size
#define	 NCHAN_PER_AMP			8 	// each neuralynx amp has 8 channels
#define  MAX_BUFFER_SIZE		1000000
#define  DEFAULT_THRESH			10000  
#define  DEFAULT_GAIN			40000 
#define  DEFAULT_LOW_FILTER		300  
#define  DEFAULT_HIGH_FILTER		600 
#define	 MIN_THRESH			0
#define	 MAX_THRESH			SHRT_MAX 
#define	 MAX_DATAVAL			SHRT_MAX
#define  MAX_DISP_VAL			USHRT_MAX
#define	 MIN_DATAVAL			SHRT_MIN
#define  MAX_PACKET_SIZE		500  // the maximum size of a data buffer from the DSPs in terms of the number of short integers in the packet
#define  MAX_PACKET_BYTES		1000  // the maximum size of a data buffer from the DSPs in terms of the number of bytes in the packet
#define MAX_SPIKES_PER_BUF		200  // at most 200 spikes at a time from spike_daq 

/* the size of the buffer used by spike_save_data. This must be larger than the
 * largest buffer of any data type, so we allocate 1 MB */
#define SAVE_BUF_SIZE			1000000 

#define EVENT_DESCRIPT_SIZE		200

/* to define the maximum number of connections we mulitply the number of
 * modules by the maximum number of slaves */
#define MAX_SLAVES		10
#define MAX_MACHINES		(MAX_SLAVES+1)
#define MAX_SOCKETS 		(MAX_MODULE_ID + 1)
#define MAX_CONNECTIONS		(MAX_SLAVES * MAX_SOCKETS + 1)     // allocate space for all possible connections 


#define SOCKET_CLIENT_TIMEOUT   5  // 5 second timeout seems to work



/* defines for master / slave messaging */
#define TCPIP_SOCKET_CLIENT_TIMEOUT   	120  	// wait up to two minutes
						// for the master to start
						// it's network configuration 

/* Message definitions. Make sure to update MESSAGES_START and MESSAGES_END
when adding new messages */

#define         MAX_MESSAGE_SIZE        100000 // the maximum size of a message. This needs to be big enough to handle any structure (SysInfo is the largest)
#define         MESSAGES_START          0
#define         MESSAGE                 0
#define         DATA                    1
#define         CONNECTION_ESTABLISHED  2 
#define         START_MESSAGE_SERVER    3 
#define         START_MESSAGE_CLIENT    4 
#define         START_DATA_SERVER       5 
#define         START_DATA_CLIENT       6 
#define         START_NETWORK_SERVER    7 
#define         START_NETWORK_CLIENT    8 
#define         START_ACQUISITION       9
#define         ACQUISITION_STARTED     10
#define         STOP_ACQUISITION        11
#define         ACQUISITION_STOPPED     12
#define         OPEN_FILE               13
#define         FILE_OPENED             14
#define         START_SAVE              15
#define         SAVE_STARTED            16
#define         STOP_SAVE               17
#define         SAVE_STOPPED            18
#define         FILE_SIZE               19 // the current size of the data file
#define         CLOSE_FILE              20
#define         FILE_CLOSED             21
#define         SYSTEM_READY            22  // system is ready 
#define         SPIKE_DATA              23 // the message contains spike data
#define         CONTINUOUS_DATA         24 // the message contains eeg data
#define         POS_DATA                25 // the message contains positiondata
#define         DATA_READY              26 // a data buffer is ready to be read
#define         SYSTEM_INFO             27 // the message contains the sysinfo structure
#define         DIGITALIO_INFO          28 // the message contains the digitalinfo structure
#define         CHANNEL_INFO            29 // the message contains the ChannelInfo structure
#define         NETWORK_INFO            30 // the message contains information for establishing a network connection
#define         DSP_INFO                31 // the message contains a dspinfo structure
#define         DSP_WRITE               32 // the message contains data to be written to the dsp
#define         DSP_READ                33 // the message contains information for a read command to be sent to a DSP
#define         DSP_IO_DONE             34 // the message contains information for a read command to be sent to a DSP
#define         DSP_TOGGLE_ACQ          35 // toggle acquisition on the local DSPs. Current a hack to fix a DSP bug

#define         POSITION_INFO           36 // the message contains position related information
#define         STOP_PROCESS            37 // start sending messages to the processing program
#define         START_PROCESS           38 // stop sending messages to the processing program
#define         STOP_DISPLAY            39 // stop sending messages to the display program
#define         START_DISPLAY           40 // start sending messages to the display program
#define         DATA_TYPE               41 // the type of data to process 
#define         RESET_CLOCK             42 // reset the clocks 
#define         CLOCK_RESET             43 // the clock was reset
#define         START_TIME              44 // the computer time at which the DSPs were reset */
#define         SYNC_TIME               45 // the current timestamp from a dsp
#define         CLEAR_SCREEN            46 // clear the screen
#define         SCREEN_CLEARED          47 // the screen has been cleared
#define         SET_DEPTH               48 // set the depth for the currently selected tetrode / channel
#define         PROGRAM_DSPS            49 // program the local DSPs
#define         DSPS_PROGRAMMED         50 // program the local DSPs
#define         REPROGRAM_DSPS          51 // reprogram the local DSPS (e.g. write to their EEPROM
#define         GET_DSP_CODE_REV        52 // return the DSP code revision number
#define         DSP_CODE_REV            53 // message data contasin the DSP code revision numbers for the local dsps
#define         SET_AUDIO               54 // set the audio channels 
#define         EVENT                   55 // the message contains an event to be saved
#define         TIME_CHECK              56 // the message contains an computer vs. master DSP time check
#define         DIGITALIO_EVENT         57 // the message contains a digital IO event 
#define         TRIGGER_OUTPUT          58 // the message contains a the number of a digital output to trigger 
#define         CHANGE_OUTPUT           59 // the message contains a the number of a digital output to trigger 
#define         FS_DATA_INFO             60 // the message contains information for the spike_user process 
#define         FS_DATA_START       61 // the message contains information for the spike_user process 
#define         FS_DATA_STOP        62 // the message contains information for the spike_user process 
#define         FS_DATA_STARTED     63 // the message contains information for the spike_user process 
#define         FS_DATA_STOPPED     64 // the message contains information for the spike_user process 
#define         OPEN_DAQ_TO_FS        65
#define         CLOSE_DAQ_TO_FS       66
#define         SETUP_DAQ_TO_FS       67
#define         SAVE_ERROR              68 // an error occurred while saving data to disk
#define         DATA_ERROR              69 // an error occurred while reading in data from the acquisition card
#define         ERROR_MESSAGE           70 // an error occured in a critical routine
#define         STATUS_MESSAGE          71 // a non critical error or a status message 
#define         SLAVE_ERROR             72 // an error occured on a slave machine
#define         DIGIO_INFO              73 // the message contains digital IO info
#define         EXIT                    74      
#define         EXITING                 75      
#define         MESSAGES_END            76


/* Events
 * Note that, for simplicity, these numbers do not overlap with the message numbers */


#define	        EVENT_ADJUST_CHAN_THRESH	100	
#define	        EVENT_ADJUST_ELECT_THRESH	101    
#define	        EVENT_ADJUST_POS_THRESH		102   
#define	        EVENT_ADJUST_CHAN_GAIN		103	
#define	        EVENT_DAQ_ERROR			104
#define	        EVENT_ACQUISITION_STARTED	105
#define	        EVENT_ACQUISITION_STOPPED	106
#define	        EVENT_SAVE_STARTED		107
#define	        EVENT_SAVE_STOPPED		108
#define 	EVENT_PACKET_LOSS		109

/* Digital IO messages are definied in spike_digio.h */

/* Datatypes */
#define		SPIKE_DATA_TYPE		0
#define		CONTINUOUS_DATA_TYPE	1
#define		POSITION_DATA_TYPE	2
#define		DIGITALIO_DATA_TYPE	3
#define		EVENT_DATA_TYPE		4
#define		TIME_CHECK_DATA_TYPE	5

/* Position Defines */
#define	MIN_POS_THRESH		0
#define	MAX_POS_THRESH		255
#define	MAX_POS_VAL		255
#define MAX_PIXELS 		(PIXELS_PER_LINE * LINES_PER_FIELD)
#define MAX_MPEG_SLICES		15
#define BYTES_PER_PIXEL 2   // for YUV image format
#define MAX_IMAGE_BYTES  (MAX_PIXELS * BYTES_PER_PIXEL)
// #define MAX_POS_TIMESTAMP_DIFF   340   // !!! uncomment for the 30Hz camera
#define MAX_POS_TIMESTAMP_DIFF   710      // !!! comment out for the 30Hz camera
#define VIDEO_DEV_NAME   "/dev/video"
#define MPEG1_CODEC     1
#define MPEG2_CODEC     2
#define MJPEG_CODEC     3
#define MAX_CODEC       3
#define MAX_GOPSIZE     300    // one I frame in 10 seconds

/* Maximum size for a single data record in the output file (used in
 * spike_extract */
#define MAX_RECORD_SIZE			MAX_MESSAGE_SIZE


/* Definitions related to data structures */

#define NCHAN_PER_ELECTRODE	4 // 4 channels per tetrode
#define MAX_CHAN_PER_ELECTRODE	32 // for silicon probes ; continuous mode only
#define MAX_CHANNELS		127 // Currently a max of 127 channels per machine
#define MAX_ELECTRODES		128 // Current maximum number of electrodes (used for array sizes and electrode number checking) 
#define MIN_ELECTRODE_NUMBER	1 // Electrodes must be numbered starting with 1
#define MAX_ELECTRODE_NUMBER	(MAX_ELECTRODES+1) 

#define NPOINTS_PER_SPIKE	40 // save a 1.25 ms window around the spike
#define NTOTAL_POINTS_PER_SPIKE	(NCHAN_PER_ELECTRODE * NPOINTS_PER_SPIKE)
#define NPOST_THRESH_POINTS	32  // save 32 points after each threshold crossing
#define NPRE_THRESH_POINTS	8  // save 8 points before each threshold crossings
#define POST_THRESH_INC		24 // go 24 points 
#define SEC_TO_TSTAMP		10000 // conversion from seconds to 100 usec timestamps
#define TSTAMP_TO_USEC		100 // conversion from 100 usec timestamps to microseconds

#define DEPTH_CONVERSION	0.02645833333333333 // the conversion from 12ths of a screw turn to mm
#define MAX_DEPTH		600  	// allow depths up to 600 (~1.5 cm)


/* maximum file size in MB (2000 for systems that don't support 64 bit file
 * sizes) */
#define	MAX_FILE_SIZE		2000

typedef unsigned int u32;

/* structure for storing internal sockets for messaging*/
typedef struct _SocketInfo {
    char 	name[80]; // file name for UNIX_SOCKETS
    char	from[80]; //Source 
    int		fromid; //Source module id
    char 	to[80];   //Destination 
    int 	toid;   //Destination module id
    int		ind;	// the array index
    char	type;	  //MESSAGE or DATA
    char 	protocol; // UNIX, TCPIP or UDP
    unsigned short port;  // port number (for ethernet sockets)
    int 	fd;
} __attribute__((__may_alias__)) SocketInfo;


/* TO DO: fix the master slave sections to refer to a machine[] array where
 * machine[0] is the master, machine[1] is the first slave, and so on */

typedef struct _NetworkInfo {
    char 		myname[80];	// the name of this machine
    int			nmachines;	// the total number of machines
    char		machinename[MAX_MACHINES][80]; // all of the machine names
    char		dspname[MAX_DSPS][80];  // the names of the dsps
    int 		myindex;	// the defined slave number for this machine 
    char 		mastername[80]; // the name of the master
    int 		*masterfd;	// the fd for the connections to the master
    unsigned short 	masterport;	// the fd for the master to slave connection
    int  		nslaves;	// the number of slaves
    char 		**slavename;	// the names or ip addresses of the slaves
    unsigned short 	*slaveport;	// the port numbers for the slave 
    int 		*slavefd;	// the fd for the slave to master connections
    int			rtslave;	// the index of the rt-linux slave
    int			fsdataslave;	// the index of the user slave if there is one
    
    unsigned short	port[MAX_NETWORK_PORTS];// the port numbers to use for TCP/IP and UDP connections
    int			nports;		// the number of port numbers
    int			messageinfd[MAX_CONNECTIONS]; // the file descriptors of the clients from which we get messages
    int			datainfd[MAX_CONNECTIONS]; // the file descriptors of the clients from which we get data
    int			messageoutfd[MAX_CONNECTIONS]	; // the file descriptors of the clients to whom we send messages
    int			dataoutfd[MAX_CONNECTIONS]; // the file descriptors of the clients to whom we send data
    int			nconn;		// the number of connections
    SocketInfo		conn[MAX_CONNECTIONS];	// information about the non master/slave data and message connections. This is read in by spike_main and spike_main_rt and the information is then distributed to the other modules.
} NetworkInfo;

/* electrode information */
typedef struct _ChannelInfo {
    short	index;		// the index of this channel in sysinfo.channelinfo[machinnum].  This is used to send the channel to other machines
    short 	dspnum;		// the number of the dsp that processes data 
    				//   from this channel
    short 	dspchan;	// the number of channel as far as the DSP is 
    				//   concerned (the number from the preamp chip)
    short 	dspind;		// the index of this channel on the dsp
    short 	number;		// the user selected number for this channel 
    short	electchan;	// the number of this channel within the electrode
    unsigned short 	depth;		// the user entered depth for this channel 
    unsigned short       thresh;		// the threshold for the channel
    unsigned short   maxdispval;     // the maximum amplitude of the signal that 
    				//   fits in the display window without clipping
    unsigned short 	lowfilter;	// the low filter setting for this channel 
    unsigned short 	highfilter;	// the high filter setting for this channel 
    short	refelect;	// the reference electrode for this channel
    short	refchan;	// the channel of the reference electrode
    short	dsprefchan;	// the dsp channel of the reference 
    int		machinenum;	// the machine number for this channel.  This is used to update the channel information on all machines
    int		color;		// the color of the trace for eeg display in 
    				//  terms of an index into the color list from
				//  COLOR_FILE
} ChannelInfo;


typedef struct _ElectrodeInfo {
    int		number;		// the number of this electrode (used only in the DSPInfo structure */
    int 	nchan;		// the number of channels
    short 	dspchan[MAX_CHAN_PER_ELECTRODE];	// the dsp channel for each channel of this electrode 
    short 	channelinfochan[MAX_CHAN_PER_ELECTRODE];	// the channelinfo channel for each channel of this electrode 
} ElectrodeInfo;

typedef struct _DSPInfo {
    unsigned short coderev;

    int samprate; // the samping rate on this dsp
    int	machinenum; // the machine that communicates with this DSP
    int	nchan;		// the total number of channels processed by this DSP
    int nsamp;		// the number of samples per channel in each buffer from this DSP
    int ntotalsamp;	// nchan * nsamp
    int nsampout;	// the number of samples per channel in each buffer from spike_daq
    int packetshorts;   // the number of shorts in a the packet 
    int packetsize;	// dsp packetsize in bytes
    int datasize;	// dsp data size in bytes
    int nelectrodes;
    short dspchan[MAX_CHAN_PER_DSP]; // the numbers of the dsp channels handled by this DSP
    short dspcal[MAX_CHAN_PER_DSP]; // the calibration factors for each channel
    int channelinfochan[MAX_CHAN_PER_DSP]; // the channelinfo channels for each dsp channel
    ElectrodeInfo	electinfo[MAX_ELECTRODES]; // The electrodes and channels on this DSP.  
    // Note that this is a copy of the electinfo array from sysinfo on systems that do spike processing, 
    // but on LFP systems it differs, as each electrode will be represented by only one channel (generally speaking).  
    // This is somewhat redundant with the three variables above.
} DSPInfo;

/* info for running programs which interface with DAQ */
typedef struct _DaqToFSInfo {
    bool  is_enabled; // state of the DAQ_TO_FS messaging interface
    int		dsps[MAX_DSPS];		// the DSPs processing data to be sent to the user program
    int		channels[MAX_CHANNELS];		// the DSP channels to be sent to the user program
    DSPInfo		dspinfo[MAX_DSPS]; // the sampling rates etc. for the dsps
} DaqToFSInfo;


/* system information */	
typedef struct _SystemInfo {
    int 		system_type[MAX_MACHINES];   	// master or slave 
    int 		dspclock;	// 1 if we want to get times from the DSPs, 0 if we want to use the system clock
    struct timeval	computer_start_time; // the approximate time at which we last reset the clock
    int 		program_type;  	// display, process, etc.
    char		myhostname[80]; // this is a duplicate of the netinfo.myname variable. It's necessary for extracting the data correctly
    bool		rtmode;		// true if data transmission between modules should be as real time as possible
    int			machinenum;	// the index for this machine
    unsigned char	datatype[MAX_MACHINES]; // the data types for each machine
    char	        datadir[MAX_MACHINES][200]; // the data types for each machine
    unsigned char	defaultdatatype; // the default data type for this machine
    FILE		*statusfile;    // the output from this program
    char		calibrationfile[200];    // the name of the file which has the calibration factor for each channel
    char		colorfilename[200];    // the name of the file with trace colors in it.  Default is /usr/local/spike2/spike_rgbcolor
    int 		pid;
    int 		acq;		// 1 if acquisition is on everywhere
    int 		dspacq;		// 1 if the dsps are acquiring data
    int 		display;
    int 		process;
    int 		diskon;		// 1 if data is being saved
    u32	disktime;	// the last time disk status was modified/
    int 		fileopen;	// 1 or 0 depending on whether the file is open
    int 		fsdataoutput;	// 1 if we have the option of sending data to spike_user
    int 		fsdataon;	// 1 or 0 depending on whether we should be sending data to the spike_user process
    char		datafilename[200];   // the name of the data file
    char		origdatafilename[200];   // the original name of the data file (used only when a second data file is opened)
    char		digioconfigfilename[200]; // the name of the digital IO configuration file to use
    int			datafilenumber;
    float		datafilesize;	// the current size of the file
    float		diskfree;	// the current amount of free space (in MB)
    float		initialdiskfree;// the initial amount of free space (in MB)
    int			newmessage;	// 1 if there is a new status or error message
    int 		lastchslave;	// the number of the slave from which we just received a channel programming message.  Used only when we are setting references or depths that have been changed on a slave. 
    
    /* DSP related variables */
    short		ndsps;		// the total number of DSPs in use
    DSPInfo		dspinfo[MAX_DSPS]; // the sampling rates etc. for the dsps
    int 		nchannels[MAX_MACHINES];// the total number of channels of recording for this computer
    ChannelInfo 	channelinfo[MAX_MACHINES][MAX_CHANNELS]; 	
    					// thresholds, filter settings etc.
    					//channelinfo[0] is for the master
					//channelinfo[1] is the first slave,
					//etc.
    ElectrodeInfo	electinfo[MAX_ELECTRODES];  // information about the dsp channels for each electrode 
    bool		commonref;
    bool		commonthresh;
    bool		commonmdv;
    bool		commonfilt;


    float		eegtracelength; // a temporary place for the eeg trace length 
    int			nelectrodes;    // the total number of electrodes on this machine
    int			maxelectnum;    // the total number of electrodes on all machines
    u32			approxtime;    	// the APPROXIMATE current time. NOT EXACT
    u32			lastdisplayedtime;    	// the last time that was displayed
    u32			lastfilesizetime;    	// the last time that the file size was displayed
    
    /* Position related variables */
    u32		cpudsptimeoffset; // the difference between dsp and cpu times
    int		allowsyncchanchange; // 1 if the sync channel settings can be changed
    int		posinputnum;  // the number of the position input to use on the frame grabber card
    short	posimagesize[2]; //the number of pixels in the image (col x row)
    bool        trackdarkpixels; // set to 1 if tracking dark pixels, 0 if tracking bright
    bool	sendalltrackedpixels; // set to 1 if the behavioral program should be sent all the tracked pixels
    bool	posxflip; 	// displayed the image flipped about the x axis
    bool	posyflip; 	// displayed the image flipped about the y axis
    unsigned char	posthresh; // threshold value treshold for including/excluding pixel in tracked set
    /* MPEG encoding variables */
    int			mpegquality;  // the 1-99 quality factor, default 75
    int			mpegslices;  // the number of slices per frame
    int     videocodec;  // video codec index. See MPEG1_CODEC MPEG2_CODEC MJPEG_CODEC definitions
    int     videogopsize;  // number of frames between I frames. If 0 each frame is I-frame, like in MJPEG
                           // not used in MJPEG

    // used to store the GUI state of use compression and compression level
    bool        use_compression;
    unsigned    compression_level;

    bool	localref;  // true if references should be local to each machine

    DaqToFSInfo 	daq_to_user;
    char		fsgui[200]; // the name of the users gui program to run

  // default c'tor
  _SystemInfo() : use_compression(1), compression_level(6) {}
} SysInfo;


typedef struct _ElectrodeData {
    u32       		timestamp;      // the timestamp in 100 usec units
    short               data[NTOTAL_POINTS_PER_SPIKE];
} ElectrodeData;


typedef struct _Point {
    float x;
    float y;
} Point;


/* Data buffers. Note that each position buffer has it's own Send and Get
 * functions, and that if you change the structure below you will need to
 * change the Send and Get functions in spike_message.cpp */
typedef struct _SpikeBuffer {
    u32 	timestamp;
    short	electnum;
    short	data[NTOTAL_POINTS_PER_SPIKE];
} SpikeBuffer;


typedef struct _ContBuffer {
    u32 	timestamp;
    short	dspnum;
    short	data[MAX_CONT_BUF_SIZE]; 
} ContBuffer;

typedef struct _PosBuffer {
    u32 		timestamp;
    u32			ntracked;
    u32			ratpos;
    int			trackedpixels[MAX_PIXELS];// the list of tracked pixels
    unsigned char 	image[MAX_IMAGE_BYTES];	// the colors for those pixel indeces
} PosBuffer;

typedef struct _PosMPEGBuffer {
    u32 		timestamp;
    u32			size;
    unsigned char 	frame[SAVE_BUF_SIZE];	// the MPEG encoded frame
} PosMPEGBuffer;

typedef struct _EventBuffer {
    u32 	timestamp;
    int		type;
    char	descript[EVENT_DESCRIPT_SIZE];
} EventBuffer;

typedef struct _DIOBuffer {
    u32		timestamp;
    unsigned short status[MAX_DIO_PORTS];
} DIOBuffer;

struct OpenFileMessage
{
  char filename[200];
  enum DataFileType { GZip, BZip2, Binary, Ascii, Default = GZip };
  DataFileType type;
  unsigned compression_level; // a number from 0-9?
};

/* the size of the static elements. This is used
 * in spike_extract only */
#define CONT_BUF_STATIC_SIZE 	(sizeof(u32) + 2 * sizeof(short))
#define DIO_BUF_STATIC_SIZE 	(sizeof(u32) + sizeof(unsigned short) * MAX_DIO_PORTS)
#define EVENT_BUF_STATIC_SIZE 	(sizeof(u32) * 3)
#define POS_BUF_STATIC_SIZE 	(4 * sizeof(u32))
#define POS_MPEG_BUF_STATIC_SIZE 	(2 * sizeof(u32))
#define TIME_CHECK_BUF_STATIC_SIZE 	(3 * sizeof(u32))

#endif
