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
#include "contiki.h"
#include "sys/etimer.h"
#include "dev/pwm.h"
#include "dev/gpio.h"
#include "dev/leds.h"
#include "dev/adc-zoul.h"
#include "dev/tsl256x.h"
#include "lighting.h"
#include "mqtt-res.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#if DEBUG_APP
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
sensor_values_t lighting_sensors;
command_values_t lighting_commands;
/*---------------------------------------------------------------------------*/
process_event_t lighting_sensors_data_event;
process_event_t lighting_sensors_alarm_event;
/*---------------------------------------------------------------------------*/
static uint8_t detect_sensor = 0;
/*---------------------------------------------------------------------------*/
PROCESS(lighting_sensors_process, "lighting process");
/*---------------------------------------------------------------------------*/
static int
activate_color_led(int arg)
{
  uint8_t port, pin, i;
  uint16_t count;
  uint8_t timer_num, timer_ab; /* variable timers */

  if(arg > LED_MAX_VAL) {
    printf("Servo: invalid value (max %u)\n", LED_MAX_VAL);
    return -1;
  }

  timer_num = PWM_TIMER_0;
  timer_ab = PWM_TIMER_B;

  printf("PWM: %uHz GPTNUM %u GPTAB %u --> %u%c (%u)\n", LED_DEFAULT_FREQ, timer_num, timer_ab, arg, 0x25, count); /* 0x25 in ASCII is % */
  if(arg) {
    for(i = 1; i <= 3; i++) {
      switch(i) {

      case 1:
        pin = LEDS_GREEN_PIN;         /*green reva PD4 revb PB7 firefly PD4*/
        port = LEDS_GREEN_PORT;
        timer_num = PWM_TIMER_0;
        timer_ab = PWM_TIMER_B;
        count = 650 * arg;
        printf("GREEN pin = %u port = %u --> %u%c (%u)\n", pin, port, arg, 0x25, count);
        break;

      case 2:
        pin = LEDS_BLUE_PIN;          /*blue reva PD3 revb PB6 firefly PD3*/
        port = LEDS_BLUE_PORT;
        timer_num = PWM_TIMER_0;
        timer_ab = PWM_TIMER_B;
        count = 650 * arg;
        printf("BLUE pin = %u port = %u --> %u%c (%u)\n", pin, port, arg, 0x25, count);
        break;

      case 3:
        pin = LEDS_RED_PIN;           /*red reva PD5 revb PD4 firefly PD5*/
        port = LEDS_RED_PORT;
        timer_num = PWM_TIMER_1;
        timer_ab = PWM_TIMER_A;
        count = 500 * arg;
        printf("RED pin = %u port = %u --> %u%c (%u)\n", pin, port, arg, 0x25, count);
        break;
      }
      /* Use count as argument instead of percentage */
      if(pwm_enable(LED_DEFAULT_FREQ, 0, count, timer_num, timer_ab) != PWM_SUCCESS) {
        printf("Servo: failed to configure the pwm channel (enable)\n");
        return -1;
      }
      if(pwm_start(timer_num, timer_ab, port, pin) != PWM_SUCCESS) {
        printf("Servo: failed to initialize the pwm channel (start)\n");
        return -1;
      }
    }
  } else {
    for(i = 1; i <= 3; i++) {
      switch(i) {

      case 1:
        pin = LEDS_GREEN_PIN;         /*green reva PD4 revb PB7 firefly PD4*/
        port = LEDS_GREEN_PORT;
        timer_num = PWM_TIMER_0;
        timer_ab = PWM_TIMER_B;
        count = 650 * arg;
        printf("GREEN pin = %u --> %u%c (%u)\n", pin, arg, 0x25, count);
        break;

      case 2:
        pin = LEDS_BLUE_PIN;          /*blue reva PD3 revb PB6 firefly PD3*/
        port = LEDS_BLUE_PORT;
        timer_num = PWM_TIMER_0;
        timer_ab = PWM_TIMER_B;
        count = 650 * arg;
        printf("BLUE pin = %u --> %u%c (%u)\n", pin, arg, 0x25, count);
        break;

      case 3:
        pin = LEDS_RED_PIN;           /*red reva PD5 revb PD4 firefly PD5*/
        port = LEDS_RED_PORT;
        timer_num = PWM_TIMER_1;
        timer_ab = PWM_TIMER_A;
        count = 500 * arg;
        printf("RED pin = %u --> %u%c (%u)\n", pin, arg, 0x25, count);
        break;
      }

      /* Use count as argument instead of percentage */
      if(pwm_stop(timer_num, timer_ab, port, pin, 0) != WM_SUCCESS) {
        printf("Servo: failed to configure the pwm channel\n");
        return -1;
      }
    }
  }
  printf("DATA_COMMAND is %u\n", arg);
  process_poll(&lighting_sensors_process);
  return 0;
}
static void
poll_sensors(void)
{
  if(!detect_sensor) {
    if(!tsl256x.configure(TSL256X_ACTIVE, 1)) {
      lighting_sensors.sensor[DIGITAL_SENSOR_LIGHT].value = tsl256x.value(TSL256X_VAL_READ);
    } else {
      lighting_sensors.sensor[DIGITAL_SENSOR_LIGHT].value = DEFAULT_WRONG_VALUE;
      detect_sensor = 1;
    }
  }
  mqtt_sensor_check(&lighting_sensors, lighting_sensors_alarm_event,
                    lighting_sensors_data_event);
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(lighting_sensors_process, ev, data)
{
  static struct etimer et;

  /* This is where our process start */
  PROCESS_BEGIN();

  /* Load sensor defaults */
  lighting_sensors.num = 1;

  /* Register digital sensors */
  mqtt_sensor_register(&lighting_sensors, DIGITAL_SENSOR_LIGHT,
                       DEFAULT_WRONG_VALUE, DEFAULT_PUBLISH_EVENT_LIGHT,
                       NULL, DEFAULT_SUBSCRIBE_CFG_LIGHTHR,
                       DEFAULT_TSL2561_LIGHT_MIN, DEFAULT_TSL2561_LIGHT_MAX,
                       DEFAULT_TSL2561_LIGHT_THRESH, DEFAULT_TSL2561_LIGHT_THRESL, 0);

  /* Sanity check */
  if(lighting_sensors.num != DEFAULT_SENSORS_NUM) {
    printf("Ubidots sensors: error! number of sensors mismatch\n");
    printf("Because sensor_values_t is %u and DEFAULT_SENSORS_NUM is %u\n", lighting_sensors.num, DEFAULT_SENSORS_NUM);
    PROCESS_EXIT();
  }

  /* Load commands default */
  lighting_commands.num = 1;

  memcpy(lighting_commands.command[0].command_name, DEFAULT_COMMAND_EVENT_LED, strlen(DEFAULT_COMMAND_EVENT_LED));

  lighting_commands.command[0].cmd = activate_color_led;

  if(lighting_commands.num != DEFAULT_COMMANDS_NUM) {
    printf("Ubidots sensors: error! number of commands mismatch\n");
    PROCESS_EXIT();
  }

  /* Get an event ID for our events */
  lighting_sensors_data_event = process_alloc_event();
  lighting_sensors_alarm_event = process_alloc_event();

  /* Activate external RGB LEDS for GPIO activation SENSORS_ACTIVATE(leds); */

  /* Start the periodic process */
  etimer_set(&et, DEFAULT_SAMPLING_INTERVAL);

  while(1) {

    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER && data == &et) {
      poll_sensors();
      etimer_reset(&et);
    } else if(ev == sensors_stop_event) {
      PRINTF("Ubidots: sensor readings paused\n");
      etimer_stop(&et);
    } else if(ev == sensors_restart_event) {
      PRINTF("Ubidots: sensor readings enabled\n");
      etimer_reset(&et);
    }
  }

  PROCESS_END();
}