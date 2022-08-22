
#include <fsDefines.h>
#include <QFile>
#include <QTextStream>


extern double fNumerator[NFILT];
extern double fDenominator[NFILT];
extern double speedFilterValues[NSPEED_FILT_POINTS];

extern FILE *rippleFilterStateFile;
extern FILE *latencyTestFile;

void SpatialFilter::Init(void) {
    stimOn = false;
    stimChanged = false;
    lastChange = 0;
    ResetSpeedData();
    xpos = ypos = 0;
    inLockout = false;
}

void SpatialFilter::ResetSpeedData()
{
    memset(speed, 0, sizeof(double) * NSPEED_FILT_POINTS);

    // read in the speed filter values
    for (int i = 0; i < NSPEED_FILT_POINTS; i++) {
       speedFilt[i] = speedFilterValues[i];
    }

    ind = NSPEED_FILT_POINTS - 1;
    lastx = 0;
    lasty = 0;
}

bool SpatialFilter::ProcessSpatialData(int16_t xposition, int16_t yposition, u32 timestamp)
{
    bool currentStim = stimOn;
    /* the current location is set when the data are read in */
//    xpos = (short) xpos;
//    ypos = (short) ypos;

    // calculate the animal's speed
    double animalSpeed = filterPosSpeed(xpos, ypos);

    // check to see if we are within the lockout time related to changes in state
    if ((timestamp - lastChange) < parm.lockoutTime) {
        // we leave the value of stim unchanged and return it
        fprintf(stderr,"timestamp %d, last change %d\n", timestamp, lastChange);
        return stimOn;
    }

    /* check that the speed is in the right window */
    if ((animalSpeed < parm.minSpeed) || (animalSpeed > parm.maxSpeed)) {
        fprintf(stderr, "speed %f outside range\n", animalSpeed);
        stimOn = false;
    }
    else {

        stimOn = ((xpos >=  parm.lowerLeftX) &&
                  (xpos <=  parm.upperRightX) &&
                  (ypos >=  parm.lowerLeftY) &&
                  (ypos <=  parm.upperRightY));
        //fprintf(stderr, "X,Y: %d, %d;  box %d, %d; %d, %d, stim: %d\n",
        //        xpos, ypos, parm.lowerLeftX, parm.lowerLeftY, parm.upperRightX, parm.upperRightY, stimOn);
    }
    if (stimOn != currentStim) {
        // we are changing the value of the stimulation state, so we record the time
        stimChanged = true;
        lastChange = timestamp;
    }
    return stimOn;
}


void SpatialFilter::sendStatusUpdate(void)
{
    char tmps[1000];

    int offset = 1;

    // put in a single character at the beginning to indicate the status type
    tmps[0] = FS_SPATIAL_STATUS;

    if (parm.enabled) {
        offset += sprintf(tmps+offset, "Spatial Filter Enabled\n\n");
    }
    else {
        offset += sprintf(tmps+offset, "Spatial Filter Disabled\n");
    }

    offset += sprintf(tmps + offset, "Rat location %d, %d\n\n",
                      xpos, ypos);

    if (parm.enabled) {
        if (stimOn) {
            sprintf(tmps + offset, "Filter triggered\n");
        }
        else {
            sprintf(tmps + offset, "Filter not triggered\n");
        }
    }
    SendFSGUIMessage(FS_RT_STATUS, tmps, 1000);
}



double SpatialFilter::filterPosSpeed(uint16_t x, uint16_t y)
{
    int i, tmpind;

    /* make sure this can't crash */
    double smoothSpd = 0.0;

    /* Calculate instantaneous speed and adjust to cm/sec */

    speed[ind] = ((x * parm.cmPerPix - lastx) * (x * parm.cmPerPix - lastx) +
                  (y * parm.cmPerPix - lasty) * (y * parm.cmPerPix - lasty));

    /* make sure this can't crash */
    if (speed[ind] != 0) {
        speed[ind] = sqrt(speed[ind]) * 30.0;
    }

    lastx = x * parm.cmPerPix; // save for next time
    lasty = y * parm.cmPerPix;


    /* apply the filter to the speed points */
    for (i = 0; i < NSPEED_FILT_POINTS; i++) {
        tmpind = (ind + i) % NSPEED_FILT_POINTS;
        smoothSpd = smoothSpd + speed[tmpind] * speedFilt[i];
    }
    ind--;
    if (ind < 0) {
        ind = NSPEED_FILT_POINTS - 1;
    }

    // if (posOutputFile != NULL)
    // fprintf(posOutputFile,"%f %f %f\n",lastx,lasty,smoothSpd);

    return smoothSpd;
}

SingleChanRippleFilter::SingleChanRippleFilter() {
    enabled = false;
    resetData();
}

void SingleChanRippleFilter::resetData() {
    rippleMean = 0;
    rippleSd = 0;
    rippleMax = 0;
    memset(fX, 0, NFILT * sizeof(double));
    memset(fY, 0, NFILT * sizeof(double));
    filtind = 0;
    memset(lastVal, 0, NLAST_VALS * sizeof(double));
    lvind = 0;
    currentVal = 0;
    currentThresh = 0;
}

void RippleFilter::Init(int num)
{
    int i;

    // copy the global filters into this structure
    for (i = 0; i < NFILT; i++) {
        filtNum[i] = fNumerator[i];
        filtDen[i] = fDenominator[i];
    }

    numNTrodes = num;
    ResetRippleData();
    ResetRippleCounters();
    stimOn = false;
    inLockout = false;
}


void RippleFilter::ResetRippleData()
{
    for (int i = 0; i < numNTrodes ; i++) {
        chanRippleFilter[i]->resetData();
    }
}

void RippleFilter::updateCustomRippleBaseline() {
    for (int ii = 0; ii < numNTrodes; ii++) {
        customRippleBaselineMean[ii] = chanRippleFilter[ii]->rippleMean;
        customRippleBaselineStd[ii] = chanRippleFilter[ii]->rippleSd;

    }
}

void RippleFilter::ResetRippleCounters(void)
{
    counter = 0;
}

double RippleFilter::updateLastVal(SingleChanRippleFilter *cptr, double d)
{
    int i;
    double mn = 0;

    for (i = 0; i < NLAST_VALS; i++) {
        mn += cptr->lastVal[i];
    }

    mn = mn / (double)NLAST_VALS;

    cptr->lastVal[cptr->lvind++] = d;

    if (cptr->lvind == NLAST_VALS)
        cptr->lvind = 0;

    return mn;
}

double RippleFilter::FilterChannel(SingleChanRippleFilter *cptr, double d)
{
    double val = 0;
    int jind;
    int i;

    cptr->fX[cptr->filtind] = d;
    cptr->fY[cptr->filtind] = 0;

    for (i = 0; i < NFILT; i++) {
        jind = (cptr->filtind + i) % NFILT;
        val = val + cptr->fX[jind] * filtNum[i] -
              cptr->fY[jind] * filtDen[i];
    }
    cptr->fY[cptr->filtind] = val;

    cptr->filtind--;

    if (cptr->filtind < 0)
        cptr->filtind += NFILT;

    return val;
}


bool RippleFilter::ProcessRippleData(int nTrodeIndex, double d, bool stimEnabled, u32 currentTime,
                                     struct timespec *sendTime, struct timespec *receiveTime)
{

//  static double v = 0.0;
//  static double posgain = 0.0;

    double gain;
    double rd;

    uint64_t sendTimeInt = sendTime->tv_sec * 1000000000L + sendTime->tv_nsec;
    uint64_t receiveTimeInt = receiveTime->tv_sec * 1000000000L + receiveTime->tv_nsec;

    //int64_t inFlightLatency = (int64_t) receiveTimeInt - (int64_t) sendTimeInt;
    //if (inFlightLatency > 10000000L) {
    //    fprintf(stderr, "Long inflight latency %ld\n",inFlightLatency/1000000L);
    //}

    SingleChanRippleFilter *cptr;

    cptr = chanRippleFilter[nTrodeIndex];

    /* --------------------------------------------------- */
    /*
       ARE WE IN LOCKOUT?
     */
    if (inLockout) {
        /* We're in lockout period following stimulation: filter linear ramp of input signal. */
        rd = FilterChannel(cptr, d * (((double)currentTime - (double)lastStimTime) / (double)parm.lockoutTime));
        cptr->currentVal = cptr->rippleMean;

        fprintf(rippleFilterStateFile, "%u,%d,%d,%d,%lf,%lf,%lf,%lu,%lu\n",
                currentTime, nTrodeIndex, stimEnabled, inLockout, rd, cptr->currentVal, d, sendTimeInt,receiveTimeInt);

        return 0;
    }
    /* --------------------------------------------------- */

    rd = FilterChannel(cptr, d); // (filter maintains state across calls)

    /* --------------------------------------------------- */
    /*
       HAVE WE JUST STARTED?
     */
    /* Wait at least 10000 samples before starting to stimulate
     * after ripple detection program is initially begun */
    if (counter < 10000) {
        counter++;
        stimOn = 0;
        return stimOn;
    }
    /* --------------------------------------------------- */

    /* --------------------------------------------------- */
    /*
       OK TO PROCESS DATA...
     */
    double y = fabs(rd); // y is absolute value of ripple filtered

    /* Only update mean and standard deviation if NOT stimulating
     * this stops the stimulation artifact from changing the values */
    if (stimEnabled == 0) {
        cptr->rippleMean = cptr->rippleMean + (y - cptr->rippleMean) / parm.sampDivisor;
        cptr->rippleSd = (fabs(y - cptr->rippleMean) - cptr->rippleSd) /
                         (double)parm.sampDivisor + cptr->rippleSd;
        // The threshold is expressed in standard deviations above mean
        cptr->currentThresh = cptr->rippleMean + cptr->rippleSd * parm.ripple_threshold;
    }


    /* --------------------------------------------------- */
    /* The goods from Jim. */
    double df = y - cptr->currentVal;

    if (df > 0) {
        /* use the mean of the last 20 values to determine the gain for this increase */
        gain = parm.ripCoeff1;
        cptr->posGain = updateLastVal(cptr, gain); // update to include this point
        cptr->currentVal = cptr->currentVal + df * cptr->posGain;
    }
    else {
        /* the gain for the decrease is fixed */
        gain = parm.ripCoeff2;
        cptr->posGain = updateLastVal(cptr, gain); // update to include this point
        cptr->currentVal = cptr->currentVal + df * gain;
    }
    if (cptr->currentVal > cptr->rippleMax) {
        cptr->rippleMax = cptr->currentVal;
    }
    fprintf(rippleFilterStateFile, "%u,%d,%d,%d,%lf,%lf,%lf,%lu,%lu\n",
            currentTime, nTrodeIndex, stimEnabled, inLockout, rd, cptr->currentVal, d, sendTimeInt,receiveTimeInt);

    //fprintf(stderr, "y = %2.2f, df = %2.2f, currentval = %2.2f, gain = %2.2f, pg = %2.2f\n", y, df, cptr->currentVal, gain, cptr->posGain);

    /* --------------------------------------------------- */



    /* Jim's algorithm outputs a magic value "v" which is roughly
     * the envelope of ripple magnitude. Check to see if v has
     * crossed threshold and stimulate if so. */  
    stimOn =  (nAboveRippleThresh() >= parm.n_above_thresh) ? 1 : 0;
    if (stimOn) {
        lastRipTime = currentTime;
        //fprintf(stderr, "last rip time =%d\n", lastRipTime);
    }

    // If we are doing normal ripple detection we return stimOn; otherwise we check to see if it has been more than the
    // specificied time since the last ripple detetion and return that

    if (!parm.detectNoRipples) {
        return stimOn;
    }
    else {
        //fprintf(stderr, "current time =%d, lastriptime = %d, returning %d\n", currentTime, lastRipTime, stimOn && ((currentTime - lastRipTime) > (u32) parm.detectNoRippleTime));
        return ((currentTime - lastRipTime) > (u32) parm.detectNoRippleTime);
    }
}

int RippleFilter::nAboveRippleThresh()
{
    int i;
    int nAbove = 0;

    for (i = 0; i < numNTrodes; i++) {
        if (chanRippleFilter[i]->enabled) {
            if (parm.useCustomBaseline) {
                // Use the custom baseline values set through FSGUI
                if (chanRippleFilter[i]->currentVal > (parm.ripple_threshold * customRippleBaselineStd[i] + customRippleBaselineMean[i])) {
                    nAbove++;
                }
            } else {
                if ((chanRippleFilter[i]->currentVal > chanRippleFilter[i]->currentThresh)) {
                    nAbove++;
                }
            }
        }
    }

    return nAbove;
}

void RippleFilter::sendCustomRippleBaseline() {
    SendFSGUIMessage(FS_SET_CUSTOM_RIPPLE_BASELINE_MEAN, (char*) customRippleBaselineMean, numNTrodes * sizeof(double));
    SendFSGUIMessage(FS_SET_CUSTOM_RIPPLE_BASELINE_STD, (char*) customRippleBaselineStd, numNTrodes * sizeof(double));
}

void RippleFilter::sendStatusUpdate(u32 timeSinceLast)
{
    char tmps[1000];
    int i, offset = 1;
    double tmpMax;

    SingleChanRippleFilter *cptr;
    // put in a single character at the beginning to indicate the status type
    tmps[0] = FS_RIPPLE_STATUS;

    if (parm.enabled) {
        offset += sprintf(tmps+offset, "Ripple Filter Enabled\n\n");

        for (i = 0; i < numNTrodes; i++) {
            cptr = chanRippleFilter[i];
            if (cptr->enabled) {
                // recalculate a maximum as the (max - mean) / std
                tmpMax = (cptr->rippleMax - cptr->rippleMean) / cptr->rippleSd;
                if(parm.useCustomBaseline) {
                    offset += sprintf(tmps + offset, "%d: *FIXED* mn(std): %2.2f (%2.2f); mx %2.1f\n",
                                      cptr->nTrodeId, customRippleBaselineMean[i], customRippleBaselineStd[i], tmpMax);
                } else {
                    offset += sprintf(tmps + offset, "%d: Rip mn(std): %2.2f (%2.2f); mx %2.1f\n",
                                      cptr->nTrodeId, cptr->rippleMean, cptr->rippleSd, tmpMax);
                }
                // reset the ripple maximum so it gets recalculated
                cptr->rippleMax = 0;
            }
        }
        sprintf(tmps + offset, "\nTimestamps since last %ld\n", timeSinceLast);
    }
    else {
        sprintf(tmps + offset, "Ripple Filter Disabled");
    }
    SendFSGUIMessage(FS_RT_STATUS, tmps, 1000);
    //fprintf(stderr, "sending status %s", tmps);
}


void InitTheta(void)
{
//    thetaFilter.parm.pulse_length = FS_RT_DEFAULT_PULSE_LEN;
//    thetaParm.vel_thresh = FS_RT_DEFAULT_THETA_VEL;
//    thetaParm.filt_delay = FS_RT_DEFAULT_THETA_FILTER_DELAY;
}

bool ThetaFilter::ProcessThetaData(double d, u32 t)
{
    return false;
}


void LatencyFilter::Init(void)
{
    counter = 0;
    triggered = false;
    firstPulseDetected = false;
    lastLatency = 0;
    latencySum = 0;
    nMeasurements = 0;
    maxLatency = 0;
    minLatency = UINT_MAX;
}

bool LatencyFilter::processContinuousData(uint32_t timestamp)
{
    if (parm.internal.enabled) {
        if((timestamp - lastTriggerTime > parm.internal.testInterval)) {
            //trigger the digital output if it has not been triggered
            if(!triggered) {
                //fprintf(stderr, "fsProcessData: ProcessData() Trigger DIO, time %d\n", timestamp);
                triggered = true;
                lastTriggerTime = timestamp;
                return true;
            }
        }
    }
    else if (parm.hpc.enabled) {
        // TODO: HPC Latency test

    }
    return false;

}

bool LatencyFilter::processDigitalIOData(bool *digOutState, u32 timestamp, int port,
                                         bool input, bool state)
{
    if(port == parm.internal.outputDIOPort) {
        //fprintf(stderr, "%d %d %d\n", maxLatency, minLatency, state);
        // NOTE: DIO messages are only sent on a state change
        if(triggered && state) {
            lastLatency = timestamp - lastTriggerTime;
            latencySum += lastLatency;
            maxLatency = std::max(maxLatency, lastLatency);
            minLatency = std::min(minLatency, lastLatency);
            nMeasurements ++;
            //fprintf(stderr, "%d\n", lastLatency);
            fprintf(latencyTestFile, "%d\n", lastLatency);
            fflush(latencyTestFile);
            triggered = false;
            return true;
        }
    }
    // Not correct port or state hasn't changed
    return false;
}

void LatencyFilter::sendLatencyData(u32 time) {
    // send the latency measurement to FSGui.  FSGui expects strings, so that's what we send...
    static char latencyString[sizeof(u32)];
    sprintf(latencyString, "%u", time);
    SendFSGUIMessage(FS_LATENCY_DATA, latencyString, sizeof(u32));
}


void LatencyFilter::sendStatusUpdate(void)
{
    char tmps[1000];
    int i, offset = 1;

    float meanLatency;

    meanLatency = (float) latencySum / (float) nMeasurements;

    // put in a single character at the beginning to indicate the status type
    tmps[0] = FS_LATENCY_STATUS;


    if (parm.internal.enabled) {
        offset += sprintf(tmps+offset,
                          "Last latency: %d samples = %f msec at 30 KHz\n",
                          lastLatency, (float) lastLatency / 30.0);
        offset += sprintf(tmps+offset,
                          "Mean latency: %0.4f samples = %f msec at 30 KHz\n",
                          meanLatency,  meanLatency / 30.0);
        offset += sprintf(tmps+offset,
                          "Max latency: %d samples = %f msec at 30 KHz\n",
                          maxLatency,  maxLatency / 30.0);
        offset += sprintf(tmps+offset,
                          "Min latency: %d samples = %f msec at 30 KHz\n",
                          minLatency,  minLatency / 30.0);
    }
    else {
        sprintf(tmps + offset, "Latency Test Disabled");
    }
    SendFSGUIMessage(FS_RT_STATUS, tmps, 1000);
    //fprintf(stderr, "sending status %s", tmps);
}


RTFilter::RTFilter(void) {
    autoSettle = true;
    timestamp = UINT32_MAX;
    lastStimTime = 0;
    for (int i = 0; i < FS_MAX_STATE_SCRIPT_FUNCTIONS; i++) {
        functionNum[i] = -1;
        functionValid[i] = false;
    }

    resetRealtimeProcessing();
}

void RTFilter::resetRealtimeProcessing() {
    QString filename="RTDebug.txt";
    QFile file( filename );QTextStream stream( &file );
    file.open(QIODevice::WriteOnly|QIODevice::Append);


    stream << "init spatial";
    spatialFilter.Init();
    stream << ", init ripple";
    rippleFilter.Init(numNTrodes);
    //thetaFilter.Init();
    stream << ", init latency" << endl;
    latencyFilter.Init();
    stream << ", init phase";
    phaseFilter.Init(numNTrodes);
    stream << ", all complete!" << " w/ numNTrodes = " << numNTrodes << endl;
    lastStimTime = 0;
}

void RTFilter::startStimulation(int fd, sockaddr_in socketAddress) {
    for (int i = 0; i < FS_MAX_STATE_SCRIPT_FUNCTIONS; i++) {
        if (functionValid[i]) {
            sendECUShortcutMessage(functionNum[i], fd, socketAddress);
            fprintf(stderr,"stim %d %d\n",this->timestamp, functionNum[i]);
        }
    }
}

void RTFilter::sendSettleCommand(int fd, sockaddr_in socketAddress) {
    uint8_t settleCommand = 0x66;
    SendHardwareMessage(fd, (char*) &settleCommand, sizeof(uint8_t), socketAddress);
    fprintf(stderr, "Sent settle command\n");

}

void RTFilter::sendECUShortcutMessage(uint16_t funcNum, int fd, sockaddr_in socketAddress) {
    //uint16_t funcNumEndianSwapped = ENDIAN_SWAP_UINT16(funcNum);
    SendHardwareMessage(fd, (char *)&funcNum,
            sizeof(uint16_t), socketAddress);
}

void RTFilter::stopStimulation(int fd, sockaddr_in socketAddress) {
    for (int i = 0; i < FS_MAX_STATE_SCRIPT_FUNCTIONS; i++) {
        if (functionValid[i]) {
            sendECUShortcutMessage(functionNum[i]+1, fd, socketAddress);
        }
    }
}



void RTFilter::sendStatusUpdate() {
    char tmps[100];

    int offset = 1;

    // put in a single character at the beginning to indicate the status type.
    tmps[0] = FS_STIM_STATUS;
    if ((stimEnabled) && (timestamp - lastStimTime > 5000)) {
        sprintf(tmps+offset, "Feedback ENABLED");
    }
    else if ((stimEnabled) && (timestamp - lastStimTime < 30000)) {
        sprintf(tmps+offset, "Feedback TRIGGERED");
    }
    else {
        sprintf(tmps+offset, "Feedback OFF");
    }
    SendFSGUIMessage(FS_RT_STATUS, tmps, 100);
}
