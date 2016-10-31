/*
 ============================================================================
 Name        : logger_test.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "logger.h"


void waitFor (unsigned int secs)
{
    unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime);               // Loop until it arrives.
}

int main(void) {

	LOG_EVT_t* logPtr;
	printf("\n INIT");
	logger_init();
	logPtr = logger_getPtr(LOG_TYPE_START);
	logPtr = logger_getPtr(LOG_TYPE_RX);
	logPtr->log_data[0] = 0xAA;
	logPtr->log_data[1] = 0xAB;
	logPtr->log_data[2] = 0xAC;
	logPtr = logger_getPtr(LOG_TYPE_TX);
	logPtr->log_data[0] = 0xFA;
	logPtr->log_data[1] = 0xFB;
	logPtr->log_data[2] = 0xFC;
	logPtr = logger_getPtr(LOG_TYPE_STOP);
	waitFor(LOG_TIMER + 5);  // to get any dumps out
	printf("\n DONE");




	return EXIT_SUCCESS;
}
