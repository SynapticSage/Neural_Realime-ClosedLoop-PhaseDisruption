#ifndef STIMCONTROLDEFINES_H
#define STIMCONTROLDEFINES_H


#define NFILT 19
//#define NSPEED_FILT_POINTS 15
#define NSPEED_FILT_POINTS 30

#define DEFAULT_MODE -1
#define OUTPUT_ONLY_MODE 0
#define REALTIME_FEEDBACK_MODE 1

#define FS_STOP_RT_FEEDBACK 200
#define FS_START_RT_FEEDBACK 201
#define FS_RESET_RT_FEEDBACK 202

#define FS_RT_ENABLE 203
#define FS_RT_DISABLE 204


#define FS_START_LATENCY_TEST 205
#define FS_STOP_LATENCY_TEST 206


#define FS_SET_RT_STIM_PARAMS 210
#define FS_SET_RT_STIM_PULSE_PARAMS 211
#define FS_SET_RIPPLE_STIM_PARAMS 212
#define FS_SET_RT_FEEDBACK_PARAMS 213
#define FS_QUERY_RT_FEEDBACK_STATUS 214
#define FS_SET_SPATIAL_STIM_PARAMS 215
#define FS_SET_LATENCY_TEST_PARAMS 216
#define FS_SET_CUSTOM_RIPPLE_BASELINE_MEAN 217
#define FS_SET_CUSTOM_RIPPLE_BASELINE_STD 218
#define FS_RT_STATUS 219

#define FS_PULSE_SEQ_START	220
#define FS_PULSE_SEQ_STOP	221

#define FS_LATENCY_DATA     222
#define FS_DIG_OUT_GATE_STATUS  223

// >>>>>>> 
#define FS_SET_PHASE_PARAMS 127 // previously = 230
#define FS_CONFIG_FILE 229
#define FS_CREATE_SAVE_FILE 230
#define FS_START_RECORDING 231
#define FS_STOP_RECORDING 232
#define FS_CLOSE_SAVE_FILE 233

#define MAX_PULSE_SEQS 		100

#define FS_STIM_STATUS      'm'
#define FS_SPATIAL_STATUS   's'
#define FS_RIPPLE_STATUS    'r'
#define FS_THETA_STATUS     't'
#define FS_LATENCY_STATUS   'l'
#define FS_PHASE_STATUS     'p'

#define FS_RT_DEFAULT_PULSE_LEN 10

#define FS_MAX_STATE_SCRIPT_FUNCTIONS 4 // two digital, two analog stimulator functions

#define FS_RT_DEFAULT_THETA_VEL 2
#define FS_RT_DEFAULT_THETA_FILTER_DELAY 730

#define FS_RT_DEFAULT_SAMP_DIVISOR  10000
#define FS_RT_DEFAULT_RIPPLE_COEFF1 1.2
#define FS_RT_DEFAULT_RIPPLE_COEFF2 0.2
#define FS_RT_DEFAULT_MUA_COEFF1 1.2
#define FS_RT_DEFAULT_MUA_COEFF2 0.2
#define FS_RT_DEFAULT_RIPPLE_TIME_DELAY 0
#define FS_RT_DEFAULT_RIPPLE_JITTER 0
#define FS_RT_DEFAULT_RIPPLE_THRESHOLD 5.0
#define FS_RT_DEFAULT_RIPPLE_N_ABOVE_THRESH 1
#define FS_RT_DEFAULT_MUA_THRESHOLD 3.0
#define FS_RT_DEFAULT_RIPPLE_LOCKOUT 7500
#define FS_RT_DEFAULT_RIPPLE_SPEED_THRESH 1000.0

#define FS_RT_DEFAULT_LATENCY_TEST_THRESHOLD 2000

#define FS_DEFAULT_CM_PER_PIX 0.87

#define FS_AO_MODE_CONTINUOUS 	1
#define FS_AO_MODE_PULSE	2
#define FS_AO_MODE_WAVE	3

#define FS_ARB_MAX_WAVE_LEN 65535

#define STATE_SINGLE_PULSE_COMMANDS 100
#define STATE_PULSE_FILE 200
#define STATE_THETA_STIM 300
#define STATE_RIPPLE_STIM 400
#define STATE_LATENCY_TEST 500

#define PULSE_IMMEDIATELY 1

#define NLAST_VALS 20

#define DECIMATED_SAMPLING_RATE  1500  // decimate the data to 1.5 KHz sampling rate

#include "DSPFilters/Dsp.h"

typedef unsigned int u32;

typedef struct _ArbInfo {
    int aout;
    bool continuous;
    unsigned short trigger_pin;
    unsigned short trigger;
    unsigned short wavefm[FS_ARB_MAX_WAVE_LEN];
    unsigned short len;
} ArbInfo;



typedef struct _PulseCommand {
    u32 start_samp_timestamp;
    bool digital_only;  // true if we should do only digital IO; false for analog IO
    int statemachine;

    /* digital defines */
    unsigned short pin1, pin2;
    int is_biphasic;

    /* analog defines */
    int aout;  // the number of the analog output to use
    int aout_mode; // the mode for the analog out
    float minv, maxv; // the minimum and maximum voltages

    int cont_percent; // the continuous mode level (1-100% of max)
    int pulse_percent; // the pulse mode level (1-100% of max)
    int wave_percent; // the wave mode peak level (1-100% of max)

    int pre_delay; // in ticks;
    int pulse_width; // in ticks (10 kHz)
    int n_pulses;
    int inter_pulse_delay; // in ticks
    int n_repeats; // note that this is decremented to zero (not preserved) by code
                   // except for -1 which is the special case of continuous
    int inter_frame_delay; // in ticks;

    ArbInfo arbinfo;


} PulseCommand;

// special codes - embedded in the "pulse_width" field
#define FS_PULSE_COMMAND_END  -1    //  end of command sequence
#define FS_PULSE_COMMAND_REPEAT -10 //  repeat command sequence
          // jumping to command in line field, repeating n_repeats


typedef struct _ThetaFilterParameters {
    int vel_thresh; // in ms
    int filt_delay;
    int stim_phase;
    bool enabled;
} ThetaFilterParameters;

typedef struct _RippleFilterParameters {
    double ripCoeff1;
    double ripCoeff2;
    double ripple_threshold;
    int sampDivisor;
    int n_above_thresh;
    int lockoutTime;
    int detectNoRippleTime;
    int dioGatePort;
    bool detectNoRipples;
    bool dioGate;
    bool enabled;
    bool useCustomBaseline;     //Use whatever custom baseline values are set in double* customRippleBaseline...
    bool updateCustomBaseline;  //Set custom baseline values from SingleChanRippleFilter estimates and send to FSGUI

} RippleFilterParameters;

enum target_t
{
    NONE,PEAK,FALLING_ZERO, TROUGH, RISING_ZERO
};
enum phase_t
{
    NO_PHASE, RISING_POS, FALLING_POS, FALLING_NEG, RISING_NEG
};
typedef class _PhaseFiltParameters {
public:

    //Methods
    Dsp::Filter* getFilter(void);
    void print(void);
    void printCSV(u32 timestamp);

    // Higher-order Filter Params
    int filterType;
    int order;
    int filterChannels=1;

    // Frequency Filter Params
    float freqHigh;
    float freqLow;

    // Smoothing Params
    bool smooth;
    int smoothWidth;

    //Channel Buffer Size
    int bufferSize;

    // Phase Paramss
    bool enablePhase;
    target_t desiredStartPhase;
    target_t desiredStopPhase=NONE;
    int desiredDelay;
    int nRequired=1;
    bool detectAbsence=false;
    int absenceTime=0;

    // Power Params
    bool enablePower;
    double desiredPower;

    // Lockout
    int lockoutTime=0;

    //State
    bool enabled;

} PhaseFiltParameters;

typedef struct _RippleStatusMsg {
  double ripMean;
  double ripStd;
  int sincelast;
  int isRunning;
  double ratSpeed;
} RippleStatusMsg;


typedef struct {
    int lowerLeftX;
    int lowerLeftY;
    int upperRightX;
    int upperRightY;
    double minSpeed;
    double maxSpeed;
    double cmPerPix;
    int   lockoutTime;
    bool enabled;
} SpatialFilterParameters;

/*
typedef struct {
    bool enabled;
    int  portA;  // the digital port for stimulator A
    int  portB;  // the digital port for stimulator B

    int stateScriptTriggerFuncNum; // the statescript function number to raise latency dig out
    // Latency test no longer uses thresholding
    //int threshold;
} LatencyTestParameters;
*/

typedef struct _InternalLatencyParameters {
    bool enabled;
    unsigned short stateScriptFnNum;
    int outputDIOPort;
    int testInterval;  // the time between tests
} InternalLatencyParameters;

typedef struct _HPCLatencyParameters {
    // STUB
    bool enabled;
} HPCLatencyParameters;


/* Contains all parameters for latency test, along with
 * which tests are selected.
 */
typedef struct _LatencyParameters {
    InternalLatencyParameters internal;
    HPCLatencyParameters hpc;


} LatencyParameters;

class PulseInfo {
public:
    //PulseArray(void);

    PulseCommand *nextPulseCmd;
    PulseCommand pulseList[MAX_PULSE_SEQS+1];

private:
    void InitPulseArray(void);
};







#endif // STIMCONTROLDEFINES_H
