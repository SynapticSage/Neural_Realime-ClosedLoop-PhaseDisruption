#include <fsDefines.h>
#include <QFile>
#include <QTextStream>
#include <ctgmath>

#define PRINTSKIPCONST 10

/* ----------------  FILTER PARAMETERS OBJECT  ------------ */
Dsp::Filter* PhaseFiltParameters::getFilter()
{
    //QString filename="RTDebug.txt";
    //QFile file( filename );QTextStream stream( &file );
    //file.open(QIODevice::WriteOnly|QIODevice::Append);

    Dsp::Filter *output = NULL;

    // Select which type and order of butterworth filter to create ... small tweak to this would easily allow chebychev too
    if (smooth)
    {
       switch (order)
       {
       case 1:
            output = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass<1>, 1>(smoothWidth);
            break;
       case 2:
            output = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass<2>, 1>(smoothWidth);
            break;
       case 3:
            output = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass<3>, 1>(smoothWidth);
            break;
       case 4:
            output = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass<4>, 1>(smoothWidth);
            break;
       case 5:
            output = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass<5>, 1>(smoothWidth);
            break;
       case 6:
            output = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass<6>, 1>(smoothWidth);
            break;
       case 7:
            output = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass<7>, 1>(smoothWidth);
            break;
       default:
            output = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass<8>, 1>(smoothWidth);
            break;
       }
    }
    else
    {
        switch (order)
        {
        case 1:
            output = new Dsp::FilterDesign <Dsp::Butterworth::Design::BandPass<1>, 1>;

        case 2:
            output = new Dsp::FilterDesign <Dsp::Butterworth::Design::BandPass<2>, 1>;
            break;
        case 3:
            output = new Dsp::FilterDesign <Dsp::Butterworth::Design::BandPass<3>, 1>;
            break;
        case 4:
            output = new Dsp::FilterDesign <Dsp::Butterworth::Design::BandPass<4>, 1>;
            break;
        case 5:
            output = new Dsp::FilterDesign <Dsp::Butterworth::Design::BandPass<5>, 1>;
            break;
        case 6:
            output = new Dsp::FilterDesign <Dsp::Butterworth::Design::BandPass<6>, 1>;
            break;
        case 7:
            output = new Dsp::FilterDesign <Dsp::Butterworth::Design::BandPass<7>, 1>;
            break;
        default:
            output = new Dsp::FilterDesign <Dsp::Butterworth::Design::BandPass<8>, 1>;
            break;
        }
    }

    if ((order > 0) && (freqHigh > freqLow) && (output!=NULL)) // order 0 happens when rtf first initializes, before phasefilter has a chance to be set .., need this or it errors out when trying to set 0 order filter
    {

        //stream << endl << "Order=" << order << endl;
        //for (int i = 0; i < output->getNumParams(); i++)
            //stream << output->getName().data() << " " << output->getParamInfo(i).getName() << " " << output->getParamInfo(i).getLabel() << "||";

        Dsp::Params dspParams;
        double bandwidth = (double) (freqHigh-freqLow);
        double centerFreq = (double) (bandwidth)/2 + freqLow;

        dspParams[0] = 1500; //hZ
        dspParams[1] = order;
        dspParams[2] = centerFreq;
        dspParams[3] = bandwidth;
        output->setParams(dspParams);
    }
    else
        output = NULL;

    // Spit filter out
    return output;
}

void PhaseFiltParameters::print(void)
{
//    static int count = 0;

//    if(count++ >= PRINTSKIPCONST)
//    {
//        count = 0;

        QString toScreen;
        QTextStream stream(&toScreen);
        stream << "===============================================================" << endl;
        stream << "Phase Filter Parameters" << endl;
        stream << "===============================================================" << endl;
        stream << "filterType = " << filterType << endl;
        stream << "order = " << order << endl;
        stream << "filterChannels = " << filterChannels << endl;
        stream << "-------------------------------------------------" << endl;
        stream << "freqHigh = " << freqHigh << endl;
        stream << "freqLow = " << freqLow << endl;
        stream << "smooth = " << smooth << endl;
        stream << "smoothWidth =" << smoothWidth << endl;
        stream << "-------------------------------------------------" << endl;
        stream << "bufferSize = " << bufferSize << endl;
        stream << "enablePhase =" << enablePhase << endl;
        stream << "enablePower =" << enablePower << endl;
        stream << "enabled = " << enabled << endl;
        stream << "nRequired = " << nRequired << endl;
        stream << "-------------------------------------------------" << endl;
        stream << "desiredStartPhase = " << desiredStartPhase << endl;
        stream << "desiredStopPhase = " << desiredStopPhase << endl;
        stream << "-------------------------------------------------" << endl;
        stream << "lockoutTime = " << lockoutTime << endl;
        stream << "-------------------------------------------------" << endl;
        stream << "detectAbsence = " << detectAbsence << endl;
        stream << "absenceTime = " << absenceTime << endl;
        stream << "================================================================" << endl;

        fprintf(stderr, toScreen.toStdString().data());
//    }
}
void PhaseFiltParameters::printCSV(u32 timestamp)
{
    fprintf(phaseParametersFile,"%d, %d, %d, %d, %3.2f, %2.2f, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %3.1f, %d", timestamp,
            filterType, order, filterChannels, freqHigh, freqLow, smooth, smoothWidth, enablePhase,desiredStartPhase, desiredStopPhase, desiredDelay, nRequired,detectAbsence,absenceTime,enablePower,desiredPower,lockoutTime);


}
/* ------------------------------------------------------*/
/* ------------------  SINGLE CHANNEL OBJECT ------------ */
/* ------------------------------------------------------*/

SingleChanPhase::SingleChanPhase()
{
    enabled = false;
}

void SingleChanPhase::ResetData(PhaseFiltParameters parentParm)
{
    bufferSize = parentParm.bufferSize;
    parm = parentParm;

    channelBuffer = new double[bufferSize];
    analyzeHere = new double*;
    *analyzeHere = channelBuffer;
    currInd = 0;
    prevInd = bufferSize-1;

    memset(channelBuffer,0,sizeof(double)*bufferSize);
    refreshAmplitudeEst(); //should be zero, after memseting all to zero.

    currphase       = NO_PHASE;
    currFiltVal     =0;
    prevFiltVal     =0;

    stimOn=false;

}

SingleChanPhase::~SingleChanPhase()
{
    // Removes the only data that this object is responsible for
    // bufferr and channel process actually point to data held by the parent
    // class
    //delete [] phase_guess; // used to be array storing past phases; might add back in
    delete [] chanFilter;
}

char* SingleChanPhase::statusMessage()
{
    char* outString = new char[50];
    char* writeLoc = outString;
    int offset = 0;

    offset+=sprintf(offset+writeLoc,"%i - CurrPhase: %i - CurrAmp: %f\n",
                    nTrodeId, currphase, currAmplitude);

    return outString;
}

void SingleChanPhase::print(void)
{
    static int count = 0;

    if(count++ >= 30000)
    {
        count = 0;

        QString toScreen;
        QTextStream stream(&toScreen);
        stream << "================================================================" << endl;
        stream << "Phase Filter, SingleChan[" << this->nTrodeId << "]" << endl;
        stream << "================================================================" << endl;
        stream << "(currInd=" << currInd << ", prevInd=" << prevInd << ")" << endl;
        stream << "(currFiltVal=" << currFiltVal << ", prevFiltVal=" << prevFiltVal << ")" << endl;
        stream << "(currphase=" << currphase << ", currAmp=" << currAmplitude << ")" << endl;
        stream << "stimOn=" << stimOn << endl;
        stream << "================================================================" << endl;

        fprintf(stderr, stream.readAll().toStdString().data());
    }
}

void SingleChanPhase::printBuffer(void)
{
    for (int i = 0; i < bufferSize-1; i++)
        fprintf(phaseBufferFile,"%f, ",this->channelBuffer[i]);
    fprintf(phaseBufferFile,"%f\n", this->channelBuffer[bufferSize-1]);
}

void SingleChanPhase::printCSV(double datum, u32 timestamp, bool masterStimOn, bool inLockout, bool printParams)
{
    fprintf(phaseProcessFile,"%i, %f, %i, %i, %i, %i, %i, %f, %f, %i, %f, %i, %i, %i",
            timestamp, datum, masterStimOn, inLockout,
            bufferSize,
            currInd,prevInd,
            currFiltVal,prevFiltVal,
            currphase,currAmplitude,stimOn,
            enabled,nTrodeId);

    // If caller requests, print parameter state also to the file
    if (printParams == true)
        fprintf(phaseProcessFile,", %i, %i, %i, %f, %f, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %d, %i, %i",
                parm.filterType,parm.order,parm.filterChannels,parm.freqHigh,parm.freqLow,parm.smooth,parm.smoothWidth,parm.bufferSize,
                parm.enablePhase,parm.desiredStartPhase,parm.desiredStopPhase,parm.desiredDelay,parm.nRequired,parm.detectAbsence,
                parm.absenceTime,parm.enablePower,parm.enablePhase,parm.lockoutTime,parm.enabled);

    // Terminate the line
    fprintf(phaseProcessFile,"\n");
}

/* ------------------Process Data ----------------------*/

void SingleChanPhase::ProcessData(double datum)
{
    stimOn=false;

    if (chanFilter!=NULL)
    {
        prevInd = currInd;
        if (++currInd >= bufferSize)
        {
            currInd = 0;
            *analyzeHere = channelBuffer;
        }
        else
            (*analyzeHere)++;

        double overwriteValue = channelBuffer[currInd]; //store value before we overwrite .. we'll substract this component off the amp est
        channelBuffer[currInd] = datum; assert(**analyzeHere == channelBuffer[currInd]);
        chanFilter->process(1,analyzeHere);  // filtered result is stored at memory address

        prevFiltVal = currFiltVal;
        currFiltVal = channelBuffer[currInd];    // get new filtered result

        /* -- Power Detection --*/
        if (parm.enablePower)
        {
            // Estimate amplitude averaged over buffer
            this->currAmplitude += (fabs(currFiltVal) - fabs(overwriteValue))/bufferSize;
            if (currAmplitude > parm.desiredPower)
                stimOn=true;

            if (!parm.enablePhase)
                return;
        }
        else
            currAmplitude=0;

        /* -- Phase Detection --*/

        if (parm.enablePhase)
        {
            auto phaseStimulate = false;
            auto tmpphase = currphase;
            if (currFiltVal < prevFiltVal && currFiltVal > 0 && currphase != FALLING_POS)
            {
                if (parm.desiredStartPhase == PEAK)
                    phaseStimulate=1;
        //        else if (parm.desiredStopPhase == PEAK)
        //            phaseStimulate=0;

                 tmpphase = FALLING_POS;
            }
            else if (currFiltVal < 0 && prevFiltVal >= 0 && currphase != FALLING_NEG)
            {
                if (parm.desiredStartPhase == FALLING_ZERO)
                        phaseStimulate=1;
        //        else if (parm.desiredStopPhase == FALLING_ZERO)
        //                phaseStimulate=0;

                tmpphase = FALLING_NEG;
            }
            else if (currFiltVal > prevFiltVal && currFiltVal< 0 && currphase != RISING_NEG)
            {
                if (parm.desiredStartPhase == TROUGH)
                    phaseStimulate=1;
    //            else if(parm.desiredStopPhase == TROUGH)
    //                phaseStimulate = 0;

                tmpphase = RISING_NEG;
            }
            else if (currFiltVal > 0 && prevFiltVal <= 0 && currphase != RISING_POS)
            {
                if (parm.desiredStartPhase == RISING_ZERO)
                   phaseStimulate=1;
        //        else if(parm.desiredStopPhase == RISING_ZERO)
        //            phaseStimulate=0;

                tmpphase = RISING_POS;
            }

            // Set current phase!
            currphase = tmpphase;

            if (!parm.enablePower)
                stimOn = phaseStimulate;
            else
                stimOn = stimOn && phaseStimulate;


        }
    }
}

void SingleChanPhase::refreshAmplitudeEst()
{
    currAmplitude=0;
    for (int i = 0; i < bufferSize; i++)
        currAmplitude+=fabs(channelBuffer[i]);
    currAmplitude/=bufferSize;
}

/* ------------------------------------------------------*/
/* ------------------  PHASE FILTER OBJECT ------------ */
/* ------------------------------------------------------*/

/*  ------------------Initiialization and Destruction ------------------- */
PhaseFilter::PhaseFilter()
{
    stimOn=false;
    inLockout=false;
    bufferSize=0;
}

void PhaseFilter::Init(int num)
{
//    QString filename="RTDebug.txt";
//    QFile file( filename );QTextStream stream( &file );
//    file.open(QIODevice::WriteOnly|QIODevice::Append);

    //stream << "PhaseConstructor" << endl;

    // Initialize tracking variables
    numNTrodes = num;
    stimOn = false;
    inLockout = false;

    // Ensure data and counters are cleared
    ResetData();
    ResetCounters();

    // Initialize each single channel buffer
    for (int i = 0; i < numNTrodes; i++)
    {
        // Assign each of the single channel structures it's own filter object with its own
        // history of what's going on.
        singlePhaseFilt[i].chanFilter = parm.getFilter();
    }

    //

}

PhaseFilter::~PhaseFilter()
{
    delete [] singlePhaseFilt;
}

void PhaseFilter::ResetData()
{
    for (int i = 0; i < numNTrodes; i++)
       singlePhaseFilt[i].ResetData(parm);
}

void PhaseFilter::ResetCounters()
{
    samplesUntilEnable=3000;
}

/* -------------Process Data Related ------------ */

bool PhaseFilter::ProcessData(int nTrodeIndex, double datum, u32 currentTime)
{

//    if (currentTime % 30000 == 0) {
//                        fprintf(stderr, "...inside phaseFilter.ProcessData\n");
//    }

    auto shouldStim = false;

    // Check if we are in lockout
    if (inLockout == true)
    {
        // Create a linear ramp of the data to de-emphasize parts of stimulus near the
        // beginning of the lockout period
        datum *= (((double)currentTime - (double)lastStimTime) / (double)parm.lockoutTime);
        if (currentTime % 30000 == 0) {
                            fprintf(stderr, "...phaseFilter.ProcessData -- lockout = true");
        }

        // Reset estimate of amplitude -- may not need this, but for right now, for safety
        singlePhaseFilt[nTrodeIndex].refreshAmplitudeEst();
    }

    // Process the data in this channel
    singlePhaseFilt[nTrodeIndex].ProcessData(datum);

    // Here, we potentially skip the rest of the function if the number of samples until
    // we begin with the natural function of the detector hasn't been reached.
    if (samplesUntilEnable > 0)
    {
        samplesUntilEnable--;
        if (currentTime % 30000 == 0) {
                            fprintf(stderr, "...phaseFilter.ProcessData -- samplesToEnabled > 0, samplesUntilEnable = %i\n", samplesUntilEnable);
        }
    }
    else
    {
        // Consider adding a gain section ... but probably don't need it for
        // the simple act of detecting one of four phases

//        if (currentTime % 30000 == 0) {
//                            fprintf(stderr, "...phaseFilter.ProcessData -- nAtReq() = %d, nReq = %d \n",this->nAtRequirement(), parm.nRequired);
//        }

        // Do we have enough channels at the phase? If so, set decision to stimulate to
        // true and set a tracker that keeps tabs on the most recent starting time of this
        // stimulation time for this current phase (which will be different then the
        // last stimulation time)
        stimOn = (nAtRequirement() >= parm.nRequired) ? true : false;
        if (stimOn)
            lastPhaseStartTime = currentTime;

        // Invert the logic if we're detecting the absence
        if (parm.detectAbsence)
            stimOn = (currentTime - lastPhaseStartTime) > (u32) parm.absenceTime;

    }

    //Print to state file -- which summarises everything about the single channel object, including whether stimulation happens here/now
    //singlePhaseFilt[nTrodeIndex].print();
    singlePhaseFilt[nTrodeIndex].printBuffer();
    singlePhaseFilt[nTrodeIndex].printCSV(datum,currentTime,stimOn,inLockout);

    return stimOn;
}

int PhaseFilter::nAtRequirement()
{
    auto numberTrodesAbove = 0;
    for(int i = 0; i < numNTrodes; i++)
        if (singlePhaseFilt[i].stimOn)
            numberTrodesAbove++;

    return numberTrodesAbove;
}

void PhaseFilter::sendStatusUpdate(uint_fast32_t  timeSinceLast)
{

    char sendData[1000];
    int offset = 1; //offset initialized at one because we prewrite a character

    sendData[0] = FS_PHASE_STATUS;

    if (parm.enabled)
    {
        offset += sprintf(sendData+offset, "Phase Filter Enabled\n\n");

        for (int i = 0; i < numNTrodes; i++)
            if (singlePhaseFilt[i].enabled)
            {
                char* channelMessage = singlePhaseFilt[i].statusMessage();
                offset += sprintf(sendData + offset, channelMessage);
                delete [] channelMessage;
            }

        offset += sprintf(sendData + offset, "\nTimestamps since last %ld\n", timeSinceLast);

    }
    else
        sprintf(sendData+offset, "Ripple Filter Disabled");

    SendFSGUIMessage(FS_RT_STATUS, (const char*) sendData, 1000);
}

