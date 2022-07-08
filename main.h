/**
 * @file main.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Globals and Includes
 * @version 0.1
 * @date 2022-04-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <Arduino.h>
#ifndef _MAIN_H_
#define _MAIN_H_

// Debug
// Debug output set to 0 to disable app debug output
#ifndef MY_DEBUG
#define MY_DEBUG 1
#endif

#if MY_DEBUG > 0
#define MYLOG(tag, ...)                  \
	do                                   \
	{                                    \
		if (tag)                         \
			Serial.printf("[%s] ", tag); \
		Serial.printf(__VA_ARGS__);      \
		Serial.printf("\n");             \
	} while (0);                         \
	delay(100)
#else
#define MYLOG(...)
#endif

#ifndef RAK_REGION_AS923_2
#define RAK_REGION_AS923_2 9
#endif
#ifndef RAK_REGION_AS923_3
#define RAK_REGION_AS923_3 10
#endif
#ifndef RAK_REGION_AS923_4
#define RAK_REGION_AS923_4 11
#endif

// Globals
extern char g_dev_name[];

/** Fport to be used to send data */
extern uint8_t g_fport;

// Send frequency, default is off
extern uint32_t g_send_repeat_time;

// Flag to enable confirmed messages
extern bool confirmed_msg_enabled;

/** Module stuff */
#include "module_handler.h"
#endif // _MAIN_H_
