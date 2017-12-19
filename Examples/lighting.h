/*
 * Copyright (c) 2017, Erik Bellido < erikbegr@gmail.com >
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/*---------------------------------------------------------------------------*/
#ifndef lighting_H_
#define lighting_H_
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "mqtt-sensors.h"
#include "dev/pwm.h"
/*---------------------------------------------------------------------------*/
enum { DIGITAL_SENSOR_LIGHT = 0 };
enum { lighting_COMMAND = 0 };
/*---------------------------------------------------------------------------*/
/* Sensor process events */
extern process_event_t lighting_sensors_data_event;
extern process_event_t lighting_sensors_alarm_event;
/*---------------------------------------------------------------------------*/
extern sensor_values_t lighting_sensors;
/*---------------------------------------------------------------------------*/
extern command_values_t lighting_commands;
/*---------------------------------------------------------------------------*/
/* PUBLISH strings */
#define DEFAULT_PUBLISH_EVENT_LIGHT           "light"
#define DEFAULT_PUBLISH_ALARM_LIGHT           "alarm_light"

/* SUBSCRIBE strings */
#define DEFAULT_SUBSCRIBE_CFG_LIGHTHR         "light_thresh"

/* Minimum and maximum values for the sensors */
#define DEFAULT_TSL2561_LIGHT_MIN             0
#define DEFAULT_TSL2561_LIGHT_MAX             30000

#define DEFAULT_WRONG_VALUE                   (-300)

/* Default sensor state and thresholds (not checking for alarms) */
#define DEFAULT_TSL2561_LIGHT_THRESH          DEFAULT_TSL2561_LIGHT_MAX
#define DEFAULT_TSL2561_LIGHT_THRESL          DEFAULT_TSL2561_LIGHT_MIN

/*led color red*/
#define DEFAULT_COMMAND_EVENT_LED          "/led_toggle/lv"
#define DEFAULT_CMD_STRING                    DEFAULT_COMMAND_EVENT_LED

#define DEFAULT_CONF_ALARM_TIME               80

/*define te pins of the led in the re-mote_revb*/
#if PLATFORM_HAS_LEDS == 1
#define LEDS_RED_PIN 4
#define LEDS_RED_PORT GPIO_D_NUM
#define LEDS_GREEN_PIN 7
#define LEDS_GREEN_PORT GPIO_B_NUM
#define LEDS_BLUE_PIN 6
#define LEDS_BLUE_PORT GPIO_B_NUM
#endif

/*---------------------------------------------------------------------------*/

#ifndef LED_CONF_FREQ
#define LED_DEFAULT_FREQ       244 /*< Hz */
#else
#define LED_DEFAULT_FREQ       LED_CONF_FREQ
#endif

#ifndef LED_CONF_MIN_VAL
#define LED_MIN_VAL            0
#endif

#ifndef LED_CONF_MAX_VAL
#define LED_MAX_VAL            100
#endif

#endif /* lighting_H_ */