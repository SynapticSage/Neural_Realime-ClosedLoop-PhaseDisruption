#include <fsDefines.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

extern SocketInfo clientData[MAX_SOCKETS]; // the structure for sending data
extern SocketInfo ECUMessage; // structure for accessing ECU socket
extern SocketInfo MCUMessage; // structure for accessing ECU socket

//extern FILE *latencyTestFile;

u32 triggered_time = 0;
int skipped = 0;
bool last_state = 0;
u32 last_cont_time = 0;

u32 lastPacketTime = 0;
u32 lastPacketType = 0;


u32 numsamp = 0;
void ProcessData(int datatype, char *data, int datalen, int nTrodeIndex, RTFilter *rtf)
{

    char *dataptr;
    short *electnumptr;

    bool inLockout = false;
    u32 stim_timestamp, curr_timestamp;
    int16_t posdataptr[2];
    bool stim = true;
    int i, j;

    static int flag = 1;

    int port;
    char input;
    char state;

    struct timespec lastReceiveTime;

    /* In a non-Position data stream, the first element of the data is always the timestamp*/
    if (datatype != TRODESDATATYPE_POSITION) {
        rtf->timestamp = *((u32 *) data);

    // update the lockout variables
    int stimTimeDiff = ((int)rtf->timestamp - (int)rtf->lastStimTime);
    if (rtf->rippleFilter.parm.enabled) {
        rtf->rippleFilter.inLockout = (stimTimeDiff < (int) rtf->rippleFilter.parm.lockoutTime);
        inLockout |= rtf->rippleFilter.inLockout;
    }
    if (rtf->spatialFilter.parm.enabled) {
        rtf->spatialFilter.inLockout = (stimTimeDiff < (int) rtf->spatialFilter.parm.lockoutTime);
        inLockout |= rtf->spatialFilter.inLockout;
    }
    if (rtf->phaseFilter.parm.enabled) {
        rtf->phaseFilter.inLockout = (stimTimeDiff < (int) rtf->phaseFilter.parm.lockoutTime);
        inLockout |= rtf->phaseFilter.inLockout;
    }
//    if (rtf->timestamp == 0)
//           fprintf(stderr, "FSDATA: zero timestamp.\n");


        // move past the timestamp to the data
        dataptr = data + sizeof(u32);

    //    if (rtf->timestamp - lastPacketTime < 0 || rtf->timestamp - lastPacketTime > 20) {
    //        fprintf(stderr, "Packet Time Error: ts %u last %d diff %d %d\n", rtf->timestamp, lastPacketTime, rtf->timestamp - lastPacketTime, lastPacketType, datatype);
    //    }
        lastPacketTime = rtf->timestamp;
        lastPacketType = datatype;
    }

    switch (datatype) {
        case TRODESDATATYPE_CONTINUOUS: {
            struct timespec *sendTime;
            sendTime = (struct timespec*) (dataptr + sizeof(int16_t));
            struct timespec receiveTime;

#ifdef __linux__
            clock_gettime(CLOCK_MONOTONIC, &receiveTime);
#endif /** linux **/

            if((rtf->timestamp - last_cont_time) > 20) {
                fprintf(stderr, "Time difference error: %d\n",rtf->timestamp - last_cont_time);
            } else if((rtf->timestamp - last_cont_time) <0) {
                fprintf(stderr, "Packets out of order: %d\n",rtf->timestamp - last_cont_time);
            }
            last_cont_time = rtf->timestamp;

            if (rtf->latencyTestEnabled) {
                if (rtf->latencyFilter.processContinuousData(rtf->timestamp)) {
                    rtf->sendECUShortcutMessage(rtf->latencyFilter.parm.internal.stateScriptFnNum, ECUMessage.fd,
                                           ECUMessage.socketAddress);
                }
            }
            //fprintf(stderr, "FSData: Process Cont Data (%d, %lf)\n", (double) *((short *) dataptr));
            if (rtf->rippleFilter.parm.enabled) {
//                if (rtf->timestamp % 30000 == 0) {
//                    fprintf(stderr, "processing ripple data %d, timestamp = %u, laststimtime = %u, lockoutTime = %u\n", *((short *) dataptr), rtf->timestamp, rtf->lastStimTime, rtf->rippleFilter.parm.lockoutTime);
//                }
                // we need to send in the lockout status because the filter is set to the mean value during lockout
                stim = rtf->rippleFilter.ProcessRippleData(nTrodeIndex, (double) *((short *) dataptr), rtf->stimEnabled,
                                                           rtf->timestamp, sendTime, &receiveTime) && stim;
                //fprintf(stderr,"stim %d",stim);
            }
            if (rtf->thetaFilter.parm.enabled) {
                // process this data point through the theta filter
                //stim = rtf->rippleFilter.ProcessThetaData(nTrodeIndex, (double) *((short *) dataptr)) && stim;
            }
            if (rtf->spatialFilter.parm.enabled) {                


                // the spatialFilter stim value will be updated by position data
                if (!rtf->rippleFilter.parm.enabled && !rtf->thetaFilter.parm.enabled && !rtf->phaseFilter.parm.enabled) {
                    // Special condition if only spatial filter is active, you only want to stim on state change
                    stim = stim && rtf->spatialFilter.stimOn && rtf->spatialFilter.stimChanged;
                } else {
                     stim = stim && rtf->spatialFilter.stimOn;
                }
            }
            if(rtf->phaseFilter.parm.enabled) {

//                if (rtf->timestamp % 30000 == 0) {
//                                    fprintf(stderr, "processing phase data %d, timestamp = %u, laststimtime = %u, lockoutTime = %u\n", *((short *) dataptr), rtf->timestamp, rtf->lastStimTime, rtf->phaseFilter.parm.lockoutTime);
//                }

                stim = stim && rtf->phaseFilter.ProcessData(nTrodeIndex, (double) *((short*) dataptr), rtf->timestamp);
            }

        }
        break;
        case TRODESDATATYPE_POSITION: {
            /* position messages consist of a variable number of packets, therefore, we must decode the
                incoming stream based on reading the flags precceeding them */

            dataptr = data;
            int curLocation = 0;
            int flg, numOfPacketsSent, packetSysTimeStamp;
            uint8_t camNum, zoneID;
            int16_t x,y,linSeg, numLins, segNum, numNodes;
            int32_t diff;
            double linPos, px1, px2, py1, py2, velocity, zx, zy;

            //initialize default values for all variables
            flg = PPT_NULL;
            camNum = -1;
            x = 0;
            y = 0;
            linSeg = -1;
            linPos = -1;

            //read first flag from the data stream
            flg = *(int*)(dataptr + curLocation);
            curLocation += sizeof(int);

            while (flg != PPT_NULL) {
                switch (flg) {
                case PPT_Header: {
                    rtf->timestamp = *(u32*)(dataptr + curLocation);
                    curLocation += sizeof(u32);
                    camNum = *(uint8_t*)(dataptr + curLocation);
                    curLocation += sizeof(uint8_t);
                    numOfPacketsSent = *(int*)(dataptr + curLocation);
                    curLocation += sizeof(int);
                    break;
                }
                case PPT_2DPos: {
                    packetSysTimeStamp = *(int*)(dataptr + curLocation);
                    curLocation += sizeof(int);
                    x = *(int16_t*)(dataptr + curLocation);
                    curLocation += sizeof(int16_t);
                    y = *(int16_t*)(dataptr + curLocation);
                    curLocation += sizeof(int16_t);
                    break;
                }
                case PPT_Lin: {
                    linSeg = *(int16_t*)(dataptr + curLocation);
                    curLocation += sizeof(int16_t);
                    linPos = *(double*)(dataptr + curLocation);
                    curLocation += sizeof(double);
                    break;
                }
                case PPT_LinTrack: {
                    numLins = *(int16_t*)(dataptr + curLocation);
                    curLocation += sizeof(int16_t);
                    //for all lines attached to the data packet
                    for (int i = 0; i < numLins; i++) {
                        segNum = *(int16_t*)(dataptr + curLocation);
                        curLocation += sizeof(int16_t);
                        px1 = *(double*)(dataptr + curLocation);
                        curLocation += sizeof(double);
                        py1 = *(double*)(dataptr + curLocation);
                        curLocation += sizeof(double);
                        px2 = *(double*)(dataptr + curLocation);
                        curLocation += sizeof(double);
                        py2 = *(double*)(dataptr + curLocation);
                        curLocation += sizeof(double);
                        //qDebug() << "-line: " << segNum;
                        //qDebug() << "   -p1(" << px1 << "," << py1 << ") || p2(" << px2 << "," << py2 << ")";
                    }
                    break;
                }
                case PPT_Zone: {
                    zoneID = *(uint8_t*)(dataptr + curLocation);
                    curLocation += sizeof(uint8_t);
                    numNodes = *(int16_t*)(dataptr + curLocation);
                    curLocation += sizeof(int16_t);
                    //qDebug() << "Zone Data Received: " << "zone(" << zoneID << ")";
                    for (int i = 0; i < numNodes; i++) {
                        zx = *(double*)(dataptr + curLocation);
                        curLocation += sizeof(double);
                        zy = *(double*)(dataptr + curLocation);
                        curLocation += sizeof(double);
                        //qDebug() << "   Line " << i << " (" << zx << "," << zy << ")";
                    }
                    break;
                }
                case PPT_Velocity: {
                    velocity = *(double*)(dataptr + curLocation);
                    curLocation += sizeof(double);
                    break;
                }
                default: {
                    //Invalid flag read
                    break;
                }
                }
                flg = *(int*)(dataptr + curLocation);
                curLocation += sizeof(int);
            } //end while()

            rtf->spatialFilter.xpos = x;
            rtf->spatialFilter.ypos = y;
            posdataptr[0] = x;
            posdataptr[1] = y;

            //start; timestamp operations moved from up above
            int stimTimeDiff = ((int)rtf->timestamp - (int)rtf->lastStimTime);
            if (rtf->rippleFilter.parm.enabled) {
                rtf->rippleFilter.inLockout = (stimTimeDiff < (int) rtf->rippleFilter.parm.lockoutTime);
                inLockout |= rtf->rippleFilter.inLockout;
            }
            if (rtf->spatialFilter.parm.enabled) {
                rtf->spatialFilter.inLockout = (stimTimeDiff < (int) rtf->spatialFilter.parm.lockoutTime);
                inLockout |= rtf->spatialFilter.inLockout;
            }
            lastPacketTime = rtf->timestamp;
            lastPacketType = datatype;
            //end; timestamp operations from above

            if (rtf->rippleFilter.parm.enabled) {

                stim = stim && rtf->rippleFilter.stimOn && !inLockout;
            }
            if (rtf->thetaFilter.parm.enabled) {
                stim = stim && rtf->thetaFilter.stimOn;
            }
            if (rtf->spatialFilter.parm.enabled) {
                //fprintf(stderr, "about to process spatial data %d %d %d\n", posdataptr[0], posdataptr[1], rtf->timestamp);
                if (!rtf->rippleFilter.parm.enabled && !rtf->thetaFilter.parm.enabled) {
                    // Special condition if only spatial filter is active, you only want to stim on state change
                    bool lastStim = rtf->spatialFilter.stimOn;
                    stim = rtf->spatialFilter.ProcessSpatialData(posdataptr[0], posdataptr[1], rtf->timestamp) && stim &&
                            rtf->spatialFilter.stimChanged;
                } else {

                    stim = rtf->spatialFilter.ProcessSpatialData(posdataptr[0], posdataptr[1], rtf->timestamp) && stim;
                }

                //fprintf(stderr, "Processed spatial data\n");

            }
            if (rtf->phaseFilter.parm.enabled) {
                stim = stim && rtf->phaseFilter.stimOn && !inLockout;
            }
            break;
        }
        case TRODESDATATYPE_DIGITALIO: {

           // Digital IO information consists of an integer port number, a character input or output value and a character state value
           port = *((int *) dataptr);
           dataptr += sizeof(int);
           input = dataptr[0];
           state = dataptr[1];
           //fprintf(stderr, "%d %d %d %d\n",rtf->timestamp, port, input, state);
           if (rtf->rippleFilter.parm.enabled) {
               stim = stim && rtf->rippleFilter.stimOn && !inLockout;
           }
           if (rtf->thetaFilter.parm.enabled) {
               stim = stim && rtf->thetaFilter.stimOn;
           }
           if (rtf->spatialFilter.parm.enabled) {
               if (!rtf->rippleFilter.parm.enabled && !rtf->thetaFilter.parm.enabled) {
                   // Special condition if only spatial filter is active, you only want to stim on state change
                   stim = stim && rtf->spatialFilter.stimOn && rtf->spatialFilter.stimChanged;
               } else {
                    stim = stim && rtf->spatialFilter.stimOn;
               }
           }
           if(rtf->phaseFilter.parm.enabled) {
               stim = stim && rtf->phaseFilter.stimOn;
           }

           if (rtf->latencyTestEnabled) {
               if (rtf->latencyFilter.processDigitalIOData(rtf->digOutState, rtf->timestamp, port,
                                                           (bool) input, (bool) state)) {
                    // Trigger function again, used if the shortcut function is a flip, reset the state from one to zero
                   rtf->sendECUShortcutMessage(rtf->latencyFilter.parm.internal.stateScriptFnNum, ECUMessage.fd,
                                          ECUMessage.socketAddress);
               }
           }
           // update the state vectors
           if (input) {
               rtf->digInState[port] = state;
           }
           else {
               rtf->digOutState[port] = state;
           }
           break;
        }
    }

    // check to see if the digital port gate is set
    if (rtf->digOutGatePort != 0) {
        stim = stim && (rtf->digOutState[rtf->digOutGatePort] == 0);
    }

    // check to see if stimulation is enabled
    if (rtf->stimEnabled) {
        if (stim && !inLockout) {
            rtf->spatialFilter.stimChanged = false;
            rtf->lastStimTime = rtf->timestamp;
            // Ripple filter also needs last stim time for linear ramp filter during lockout
            rtf->rippleFilter.lastStimTime = rtf->timestamp;
            rtf->phaseFilter.lastStimTime = rtf->timestamp;
            if (ECUMessage.fd != -1) {
                rtf->startStimulation(ECUMessage.fd, ECUMessage.socketAddress);
            }
            else {
                fprintf(stderr, "tried to start Stim - no ECU connection\n");
            }
            if (rtf->autoSettle && MCUMessage.fd != -1) {
                //immediately settle
                rtf->sendSettleCommand(MCUMessage.fd, MCUMessage.socketAddress);
            }
            else if (rtf->autoSettle) {
                fprintf(stderr, "tried to settle amplifiers - no MCU connection\n");
            }
        }
        else if ((rtf->spatialFilter.stimChanged && !inLockout)) {
            rtf->spatialFilter.stimChanged = false;
            // we had been stimulating but we need to stop
            if (ECUMessage.fd != -1) {
                rtf->stopStimulation(ECUMessage.fd, ECUMessage.socketAddress);
            }
            else {
                fprintf(stderr, "tried to stop Stim - no ECU connection\n");
            }
        }
    }
}

void InitPulseArray(void)
{

}





