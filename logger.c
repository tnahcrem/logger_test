/*
 * logger.c
 *
 *  Created on: Oct 29, 2016
 *      Author: hmerchan
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "logger.h"

uint8_t* loggerStartPtr;
uint8_t* logRdIdx;
uint8_t* logWrIdx;
int logisFull;
int dumpFlag;
uint8_t toLog;
uint32_t logRxTs;
uint32_t logRxLen;


// macro to do system level operation
// can be changed depending on the system
#define GET_CUR_TIME()	(clock())
#define START_TIMER()		(alarm(LOG_TIMER))
#define STOP_TIMER() 		(alarm(0))


/*****************************************************************************
*    Function: void log_timeout(int signal)
*
*    Abstract: The timeout callback of the timer
*
*    Input/Output: The signal which is enabled for the timer
*
*    Return:
*
*****************************************************************************/
void log_timeout(int signal)
{
  logger_send();
}

/*****************************************************************************
*    Function: void logger_init(void)
*
*    Abstract: Initialization function of logger
*
*    Input/Output:
*
*    Return:
*
*****************************************************************************/
void logger_init(void)
{
    loggerStartPtr = (uint8_t*)malloc(LOG_MAX_SIZE); // initialize log memory
    // init read write pointers
    logWrIdx = loggerStartPtr;
    logRdIdx = loggerStartPtr;
    dumpFlag = 0;
    logisFull = 0;
    toLog = 0;
    signal(SIGALRM, log_timeout); // on a RTOS based system change this to a more controlled form of timer

}

/*****************************************************************************
*    Function: uint32_t logger_getSize(void)
*
*    Abstract: gets current size utilized by logger
*
*    Input/Output:
*
*    Return:
*
*****************************************************************************/
uint32_t logger_getSize(void)
{

    if(logWrIdx == logRdIdx)
    {
        if(logisFull)
        {
            return LOG_MAX_SIZE;
        }
        else
        {
            return 0;
        }
    }

    if(logWrIdx > logRdIdx)
    {
        return (logWrIdx - logRdIdx);
    }
    else
    {
        return ((LOG_END_ADDR - logRdIdx) + (logWrIdx - LOG_START_ADDR));
    }
}

/*****************************************************************************
*    Function: void logger_send(void)
*
*    Abstract: Function to dump existing log to the output
*
*    Input/Output:
*
*    Return:
*
*****************************************************************************/
void logger_send(void)
{
    uint32_t len;
    uint32_t tmpLen;
    FILE *fp;

    // For this setup the output mode used is file pointer
    // depending on the system; this can be changed to as needed
    fp = fopen( "log.bin" , "w" );

    // loop through the whole log memory
    while(logger_getSize())
    {
        len = logger_getSize();

       // check for wrap around
        if((logRdIdx + len) > LOG_END_ADDR)
        {
            tmpLen = LOG_END_ADDR - logRdIdx;
            fwrite(logRdIdx , 1 , tmpLen , fp );
            fwrite(logRdIdx , 1 , (len - tmpLen) , fp );
        }
        else
        {
            fwrite(logRdIdx , 1 , len , fp );
        }

        // update read pointer
        logRdIdx += len;
        if(logRdIdx >= LOG_END_ADDR)
        {
            logRdIdx = LOG_START_ADDR + (logRdIdx - LOG_END_ADDR);
        }
    }

    // in case of log dumped; add a indicator
    if(dumpFlag)
    {
    	LOG_EVT_t logEvt;
        // send a log event to indicate dumped data
        logEvt.timestamp = 0xADADADAD; /// dummy timestamp
        logEvt.log_type = LOG_TYPE_DUMP_DATA;
        fwrite(&logEvt , 1 , sizeof(LOG_EVT_t) , fp );
        dumpFlag = 0;
    }
    fclose(fp);
    toLog  = 0;
}


/*****************************************************************************
*    Function: LOG_EVT_t* logger_getPtr(LOG_TYP_e logType)
*
*    Abstract: allocated memory for new log entry
*
*    Input/Output:
*
*    Return:
*
*****************************************************************************/
LOG_EVT_t* logger_getPtr(LOG_TYP_e logType)
{
    uint8_t* retPtr;
    uint32_t len;
    LOG_EVT_t* logPtr;

    // in this setup the size of each log entry is fixed;
    // in case of setups with multiple log types with various
    // lengths; this statement can be modified to setup len as per
    // the logType argument
    len = sizeof(LOG_EVT_t);


    if(dumpFlag)
    {
        // once dump flag is set ; dump all until the dumpFlag is cleared
        return 0;
    }

    if(LOG_GET_AVAIL_SIZE < len)
    {
        // set the dump flag
        dumpFlag = 1;
        return 0;
    }


    // allocate new log entry
    retPtr = logWrIdx;
    logPtr = (LOG_EVT_t*)retPtr;

    logPtr->timestamp = clock();
    logPtr->log_type = logType;

    // update write pointer
    logWrIdx += len;
    if(logWrIdx >= LOG_END_ADDR)
    {
        logWrIdx = LOG_START_ADDR + (logWrIdx - LOG_END_ADDR);
    }

    // on a RTOS based system this can be used to trigger
    // a message to idle thread to start pushing out the logs
    // to make more room
    if(logger_getSize() >= LOG_PUSH_THRESH)
    {
        toLog = 1;
    }

    // auto trigger posting out of log
    // in case there is idle activity after a stop transaction
    if(logType == LOG_TYPE_STOP)
    {
    	START_TIMER();
    }

    if(logType == LOG_TYPE_START)
    {
    	STOP_TIMER();
    }

    return logPtr;
}
