#ifndef PHASEFILTER_H
#define PHASEFILTER_H

#ifndef DSPFILTERS_DSP_H
    #include <Dsp.h>
#endif

class FilterParameters
{
    // DATA
    int filterType;
    int order;
    int filterChannels;

    float freqHigh;
    float freqLow;

    bool smooth;
    int smoothWidth;

    //METHOD
    Dsp::Filter* getFilter();
};

enum target_t
{
    NONE,PEAK,FALLING_ZERO, TROUGH, RISING_ZERO
};
enum phase_t
{
    NO_PHASE, RISING_POS, FALLING_POS, FALLING_NEG, RISING_NEG
};

struct PhaseParameters
{
    target_t desiredStartPhase;
    target_t desiredStopPhase;
    int     desiredDelay;
};

class SingleChanPhase
{

    // METHODS

    ~SingleChanPhase();
    void Initialize(int nBuffer);
    void ResetData();

    void refreshAmplitudeEst();
    void ProcessData(double d);

    // DATA

    Dsp::Filter *chanFilter; //can contain any number of different filter types

    double *channelBuffer;
    int bufferSize;
    int currInd;
    int prevInd;

    // Filtered Value Storage
    double currFiltVal;
    double prevFiltVal;

    // Phase Amplitude Storage
    phase_t currphase;
    double currAmplitude;

    //Targets
    target_t on;
    target_t off;
    int delay; // samples to delay

    // Stimulation Vote
    bool stimOn;

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

    void ProcessData(int nTrodeIndex, double d, bool stimEnabled);
//    void ProcessData(int nTrodeIndex, double d, bool stimEnabled, u32 currentTime,
//                     struct timespec *sendTime, struct timespec *receiveTime);

    /*---------------------------------------------------------*/
    /*                      DATA
    /*---------------------------------------------------------*/
    // FILTER PARAMETERS    -- control of bandpass, filter type, and order
    FilterParameters parmF;
    // PHASE PARAMETERS     -- control of which phase to target and delays to stimulation
    PhaseParameters parmP;

    // TRACKERS
    // Permanent Stimulation Trackers
    bool stimOn;
    bool inLockout;

    // Permanent Time Trackers
    uint_fast32_t lastStimTime;
    uint_fast32_t lastPhaseStartTime;

    // Initialization Period Trackers
    int samplesUntilEnable;

    // CHANNEL DATA
    //    double **masterChannelProcess;      //what's actually sent to DSP filter
    //    double **masterChannelBuffer;       //what's kept online
    // The commented out buffers were going to be used for a separate mode where, instead of each channel at a time, channels were going to be processed in parallel
    int     numNTrodes;                 // number of channels
    int     bufferSize;                 //how many values to keep alive in the buffer .. more useful for "manual" coefficent values method;

    SingleChanPhase *singlePhaseFilt; // don't think it has to be double pointered, hence why I'm removing that
};

#endif // PHASEFILTER_H
