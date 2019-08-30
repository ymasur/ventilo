/*
  v_cmd.hpp
  ---------
  30.08.2019 - ymasur@microclub.ch

  Main module definitions
 */
#define __PROG__ "Ventilo_cmd"
#define VERSION "0.92" // Module version
#define EEPROM_VERS 1
#define EEPROM_DATA_OFFSET 10
#define DEBUG 1 // level 0 - 1 - 2

#include <Arduino.h>
#include <avr/wdt.h>
#include <string.h>
#include <Wire.h>
#include <time.h>
#include <RTClib.h>
#include <jm_Scheduler.h>
#include <jm_LCM2004_I2C.h>
#include <jm_LCM2004_I2C_Plus.h>

#ifndef V_CMD_HPP
#define V_CMD_HPP

#ifdef MAIN // Storage class definition
  #define CLASS
#else
  #define CLASS extern
#endif
			/* variable limited  */
#define BOUND(val, min, max)	  \
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

#define LED13 13    // LED connected to pin 13, used as life signal

#define EEPROM_SIGNATURE 0x5A // caution: if the signature is not regnized, erase of datas!
#define EEPROM_VERSION 1  //0 .. 255 - if not recognisez, no writing of datas

// prototypes (needed for VSCode/PlateformIO)
void blink(short, short);
void poll_loop_5();
#define POLL_FREQ (2)
void poll_loop_X_ms();
void display_info(String info);
void log_msg(String msg);
void dateTime_up_ascii();
uint8_t init_time_tables();
void eepromInit();

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

  uint8_t run();  // must be called each 500 ms
};

//Global inst. are defined here
CLASS CmutRel cmutRel;

// instance of LCD
CLASS jm_LCM2004_I2C_Plus *lcd;

// list of scheduler used
CLASS jm_Scheduler pulse_500ms; // refresh of LCD display; main loop
CLASS jm_Scheduler pulse_X_ms;  // scan of buttons

#endif // V_CMD_HPP