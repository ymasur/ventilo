/*
 v_cmd.hpp
 ---------
 00.08.2019 - ymasur@microclub.ch
 */
#define __PROG__ "ventilo_cmd"
#define VERSION "0.33 " // Module version
#define DEBUG 1 // level 0 - 1 - 2

#include <Arduino.h>
#include <avr/wdt.h>
#include <string.h>
#include <Wire.h>
#include <time.h>
#include <RTClib.h>
//#include <EEPROM.h>
#include <jm_Scheduler.h>
#include <jm_LCM2004_I2C.h>
#include <jm_LCM2004_I2C_Plus.h>

#ifndef V_CMD_HPP
#define V_CMD_HPP

#ifdef MAIN
  #define CLASS
#else
  #define CLASS extern
#endif
			/* variable limited  */
#define BOUND(val,min,max)	\
{                               \
   if (val > max) val = max;    \
   if (val < min) val = min;    \
}

// IO pin are defined here
#define LED_C A0    // LED C, display state of relay cmd
#define LED_ON() digitalWrite(LED_C, 0)
#define LED_OFF() digitalWrite(LED_C, 0)

#define REL A1
#define RELAY_ON() digitalWrite(REL, 0) //the relay is active LOW
#define RELAY_OFF() digitalWrite(REL, 1)
#define REL_ON_DURATION 60  // in minutes

#define SW_ACT 4   // Input hard definition
#define SW_PLUS 5
#define SW_MINUS 6
#define SW_OK 7
#define SW_NB 4 // nb of used switches

#define B_ACT 0   // define button logical definition
#define B_PLUS 1
#define B_MINUS 2
#define B_OK 3

#define LED13 13    // LED red is connected to pin 13

// prototypes (needed for VSCode/PlateformIO)
void blink(short, short);
void poll_loop_5();
#define POLL_FREQ (2)
void poll_loop_100ms();
void display_info(String info);
void log_msg(String msg);
void dateTime_up_ascii();


class CmutRel
{
  private:
  uint8_t state;    // little state machine, of commutation relay:
  #define CR_OFF 0  // relay is Off
  #define CR_LED 1  // LED_C blink 
  #define CR_ON 2   // relay is On

  volatile uint16_t timer_LED;  // time in polling unit where LED_C must blink
  volatile uint16_t timer;  // time in polling unit, left to switch to Off

  public:
  uint8_t commute_table;

  public:
  inline uint8_t getSt() {return state;}
  inline uint16_t getTimeLeft(){return (timer+50)/(60*POLL_FREQ);} // in minutes
  inline uint16_t getTimer(){return timer;}  // raw value
  // timer loaded for an hour by default
  inline void loadTimer(uint16_t time = REL_ON_DURATION){timer = time * 60 * POLL_FREQ;}

  void on_LED();
  void on();      // make the relay ON while a time
  void off();     // makes the relais and LED off

  uint8_t run();
};

//Global inst. are defined here
CLASS CmutRel cmutRel;

// instance of LCD
CLASS jm_LCM2004_I2C_Plus *lcd;

// list of scheduler used
CLASS jm_Scheduler pulse_500ms;
CLASS jm_Scheduler pulse_100ms;

//CLASS RTC_DS3231 rtc;
//CLASS DateTime myTime;

#endif // V_CMD_HPP