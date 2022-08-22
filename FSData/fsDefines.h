#ifndef FSDEFINES_H
#define FSDEFINES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <limits.h>
#include <algorithm>
#include <time.h>

#include <../fsSharedStimControlDefines.h>
#include <fsSocketDefines.h>
#include <trodesSocketDefines.h>


#define ENDIAN_SWAP_UINT16(X) (X >> 8 | X << 8)

extern double fNumerator[NFILT];
extern double fDenominator[NFILT];
extern double speedFilterValues[NSPEED_FILT_POINTS];

extern FILE *rippleFilterStateFile;
extern FILE *latencyTestFile;

extern FILE* phaseParametersFile;
extern FILE* phaseProcessFile;
extern FILE* phaseBufferFile;

#define MAX(x, y) ((x) > (y) ? (x) : (y))
/* structure for storing internal sockets for messaging*/
typedef struct _SocketInfo {
    char name[80];        // file name for UNIX_SOCKETS
    char hostName[80];          //Destination
    char type;            //MESSAGE or DATA
    int socketType;        // UNIX, TCPIP or UDP
    unsigned short port;  // port number (for ethernet sockets)
    uint8_t dataType;      // the type of data this socket handles
    int decimation;     // the decimation factor for continuous data
    int nTrodeId;        // the nTrode number (if relevant)
    int nTrodeIndex;   // the index of this nTrode for referencing corresponding filter structures
    bool enabled;
    int fd;
    struct sockaddr_in socketAddress; // Socket address details to send UDP
} __attribute__((__may_alias__)) SocketInfo;



class SingleChanRippleFilter {
public:
    SingleChanRippleFilter();
    double rippleMean;
    double rippleSd;
    double rippleMax;
    double fX[NFILT];
    double fY[NFILT];
    int	   filtind;
    double lastVal[NLAST_VALS];
    int	   lvind;
    double currentVal;
    double currentThresh;
    double posGain;
    int    nTrodeId; // the user assigned number of this nTrode
    bool   enabled;  // true if this Ntrode is enabled

    void resetData(void); // reset everything except the enabled flag.
};

class RippleFilter {
public:
    RippleFilterParameters parm;
    SingleChanRippleFilter **chanRippleFilter;

    double *customRippleBaselineMean;
    double *customRippleBaselineStd;

    double filtNum[NFILT]; // the numerator for the ripple filter
    double filtDen[NFILT];

    bool stimOn;
    int counter;
    int numNTrodes;
    bool inLockout;
    u32 lastStimTime;
    u32 lastRipTime;
    void ResetRippleData(void);
    void ResetRippleCounters(void);
    void updateCustomRippleBaseline();  //Update the baseline from SingleChanRippleFilter

    void Init(int numNTrodes);

    bool ProcessRippleData(int nTrodeIndex, double d, bool stimEnabled, u32 currentTime,
                           struct timespec *sendTime, struct timespec *receiveTime);
    double FilterChannel(SingleChanRippleFilter *rptr, double d);
    double updateLastVal(SingleChanRippleFilter *rptr, double d);
    int nAboveRippleThresh(void);

    void sendStatusUpdate (u32 timeSinceLast);
    void sendCustomRippleBaseline();
    int nAboveRippleThresh(RippleFilter *rptr);
};


class SpatialFilter {
public:
    SpatialFilterParameters parm;

    bool stimOn;
    bool stimChanged;
    int  lockoutTime;
    u32	 lastChange;

    double speed[NSPEED_FILT_POINTS];
    double speedFilt[NSPEED_FILT_POINTS];
    short xpos, ypos;  // location of animal
    double lastx; // x location converted to cm
    double lasty; // y location converted to cm
    int	   ind;
    bool   inLockout;



    void Init();
    double filterPosSpeed(uint16_t x, uint16_t y);
    void ResetSpeedData(void);
    bool ProcessSpatialData(int16_t xposition, int16_t yposition, u32 timestamp);

    void sendStatusUpdate(void);
};

class ThetaFilter {
public:
    ThetaFilterParameters parm;
    bool ProcessThetaData(double d, u32 t);

    bool stimOn;
    bool inLockout;

    //void sendStatusUpdate (void);

};

class LatencyFilter {
public:
    int counter;
    bool triggered;
    bool lastDIOstate;
    bool firstPulseDetected;

    u32 lastLatency;
    u32 lastTriggerTime;
    u32 firstPulseTime;
    u32 maxLatency;
    u32 minLatency;

    u32 latencySum;
    u32 nMeasurements;

    LatencyParameters parm;

    void Init(void);
    bool processContinuousData(u32 timestamp);
    bool processDigitalIOData(bool *digOutState, u32 timestamp, int port, bool input, bool state);
    void sendLatencyData(u32 time);
    void sendStatusUpdate (void);

};

class SingleChanPhase
{

    // METHODS
public:
    SingleChanPhase();
    ~SingleChanPhase();
    void ResetData(PhaseFiltParameters parentParm);

    // Data Processing Methods
    void refreshAmplitudeEst();
    void ProcessData(double d);

    // Print Methods
    char* statusMessage(void);
    void  print(void);
    void printBuffer(void);
    void printCSV(double datum, u32 timestamp, bool masterStimOn,
                  bool inLockout, bool printParams=false);

    // Filter Object
    Dsp::Filter *chanFilter; //can contain any number of different filter types

    // Param copy
    PhaseFiltParameters parm;

    //  Updated data
    double *channelBuffer;
    double ** analyzeHere;
    int bufferSize;
    int currInd;
    int prevInd;

    // Filtered Value Storage
    double currFiltVal;
    double prevFiltVal;

    // Phase Amplitude Storage
    phase_t currphase;
    double currAmplitude;

    // Stimulation Vote
    bool stimOn;

    // State and Identity
    bool enabled;//contntrodes=false;//enablecontdatasocket=true
    int nTrodeId;
};

class PhaseFilter
{
public:
    /*---------------------------------------------------------*/
    /*                      METHODS
    /*---------------------------------------------------------*/
    // "CONSTRUCTOR" METHOD AND DESCTRUCTOR
    PhaseFilter();
    ~PhaseFilter();
    void Init(int numNTrodes);

    // RESET METHODS
    void ResetData();
    void ResetCounters();

    bool ProcessData(int nTrodeIndex, double d, u32 currentTime);

    /*---------------------------------------------------------*/
    /*                      DATA
    /*---------------------------------------------------------*/
    // FILTER PARAMETERS    -- control of bandpass, filter type, and order
    // and PHASE PARAMETERS     -- control of which phase to target and delays to stimulation
    PhaseFiltParameters parm;

    // TRACKERS
    // Permanent Stimulation Trackers
    bool stimOn;//init
    bool inLockout;//init

    // Permanent Time Trackers
    u32 lastStimTime;
    u32 lastPhaseStartTime;

    // Initialization Period Trackers
    int samplesUntilEnable;

    // CHANNEL DATA
    //    double **masterChannelProcess;      //what's actually sent to DSP filter
    //    double **masterChannelBuffer;       //what's kept online
    // The commented out buffers were going to be used for a separate mode where, instead of each channel at a time, channels were going to be processed in parallel
    int     numNTrodes;//numntrodes                 // number of channels
    int     bufferSize;//init                 //how many values to keep alive in the buffer .. more useful for "manual" coefficent values method;

    SingleChanPhase *singlePhaseFilt;//numntrodes // don't think it has to be double pointered, hence why I'm removing that
    int nAtRequirement();
    void sendStatusUpdate(uint_fast32_t timeSinceLast);
};

class RTFilter {
public:
    RTFilter(void);

    int hardwareFD; // the file descriptor for the UDP connection to the hardware
    int numNTrodes;
    int nDigInPorts;
    int nDigOutPorts;
    bool *digInState;
    bool *digOutState;
    bool stimEnabled;
    bool latencyTestEnabled;
    bool *contNTrodeEnabled;
    bool *spikeNTrodeEnabled;
    bool autoSettle; //autoSettle following stimulation ripple

    bool continuousStimulation;  // this is currently set when the spatial filter uses continuous stimulation
    u32 lastStimTime;       // timestamp of packet that triggered stim

    u32 timestamp;          // timestamp of incoming packet being processed

    RippleFilter rippleFilter;
    SpatialFilter spatialFilter;
    ThetaFilter thetaFilter;
    LatencyFilter latencyFilter;
    PhaseFilter phaseFilter;

    int digOutGatePort = 0;  // the user selected port to use for gating digital outputs; 0 indicates no gate

    uint16_t functionNum[FS_MAX_STATE_SCRIPT_FUNCTIONS];
    bool functionValid[FS_MAX_STATE_SCRIPT_FUNCTIONS];

    void resetRealtimeProcessing(void);
    void sendStatusUpdate(void);

    void startStimulation(int, sockaddr_in);
    void stopStimulation(int, sockaddr_in);
    void sendSettleCommand(int, sockaddr_in);
    void sendECUShortcutMessage(uint16_t, int, sockaddr_in);

};


/* the main message and data processing routines */
void ProcessMessage(int message, char *messagedata, int messagedatalen, RTFilter *rtf);
void ProcessData(int datatype, char *data, int datalen, int nTrodeIndex, RTFilter *rtf);
void SetupFDList(fd_set *readfds, int *maxfds, SocketInfo *messageInfo,
                 SocketInfo *dataInfo);
void AddFD(int fd, SocketInfo *s, int *fdlist);
void RemoveFD(int fd, SocketInfo *s, int *fdlist);

int GetServerSocket(const char *name);
int GetServerSocket(const char *name, int timeoutsec, int timeoutusec);
int GetClientSocket(const char *name);
/* the second version of GetClientSocket is used only by spike_matlab to get,
 * if available, a socket to send data to Matlab */
int GetClientSocket(const char *name, int timeoutsec, int timeoutusec);

int GetTCPIPServerSocket(unsigned short port);
int GetTCPIPClientSocket(const char *name, unsigned short port);

int GetUDPHardwareClientSocket(const char *name, unsigned short port, struct sockaddr_in *address);
int GetUDPDataClientSocket(const char *name, unsigned short port, struct sockaddr_in *address);

int GetUDPServerSocket(unsigned short port);

int SendMessage(int fd, uint8_t message, const char *data, uint32_t datalen);
int SendFSGUIMessage(uint8_t message, const char *data, uint32_t datalen);
int SendHardwareMessage(int fd, const char *data, u32 datalen, struct sockaddr_in socketAddress);

int GetMessage(int fd, char *messagedata, u32 *datalen, int block);
int GetUDPMessage(int fd, char *messagedata, u32 *datalen, int block);
int WaitForMessage(int fd, int message, float sec);
int WaitForMessage(int fd, int message, float sec, char *data, u32 *datalen);

void CloseSockets(SocketInfo *s);
void ClearData(int fd);

int NTrodeSocketIndex(int nTrode, uint8_t dataType, const int nDataSockets);


void StartStimulation(void);

void StopStimulation(void);
void fsdataexit(int status);


#endif // FSDEFINES_H
