/*
 * logger.h
 *
 *  Created on: Oct 29, 2016
 *      Author: hmerchan
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdint.h>
/******************** DEFINITIONS ********************************/

// simple entry for a log message definition
typedef struct
{
    uint32_t timestamp;
    uint8_t  log_type;
    uint8_t	 log_data[3];
}__attribute__((packed)) LOG_EVT_t;

typedef enum
{
	LOG_TYPE_START,
	LOG_TYPE_RX,
	LOG_TYPE_TX,
	LOG_TYPE_STOP,
	LOG_TYPE_DUMP_DATA
}LOG_TYP_e;



#define LOG_START_ADDR (loggerStartPtr)
#define LOG_MAX_SIZE 64
#define LOG_END_ADDR (LOG_START_ADDR + LOG_MAX_SIZE)
#define LOG_PUSH_THRESH (32)
#define LOG_GET_AVAIL_SIZE (LOG_MAX_SIZE - (logger_getSize()))
#define LOG_TIMER 5

void logger_init(void);
uint32_t logger_getSize(void);
void logger_send(void);
LOG_EVT_t* logger_getPtr(LOG_TYP_e logType);

#endif /* LOGGER_H_ */
