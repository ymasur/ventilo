/*
  v_cmd.cpp
  ---------
  24.08.2019 - ymasur@microclub.ch
 */
#ifndef MAIN  // this is the main module
#define MAIN

#include <Arduino.h>
// #include <EEPROM.h>   // local to this module

#include "v_rtc.hpp"
#include "v_cmd.hpp"
#include "v_menu.hpp"

void setup() 
{
  // initialize the digital pin for LEDs as an output.
  pinMode(LED13, OUTPUT);
  digitalWrite(LED13, LOW);
  pinMode(LED_C, OUTPUT);
  digitalWrite(LED_C, LOW);
  pinMode(REL, OUTPUT);
  RELAY_OFF();

  lcd = new jm_LCM2004_I2C_Plus(0x27);

  // init an array of button control
  sw[0] = new Sw(SW_ACT, "ACT");
  sw[1] = new Sw(SW_PLUS, "[+]");
  sw[2] = new Sw(SW_MINUS, "[-]");
  sw[3] = new Sw(SW_OK, "OK");

  // I2C
  Wire.begin();
  // LCD
  lcd->begin();  lcd->clear_display(); 
  lcd->print(__PROG__ " " VERSION);  // on line 1 of 4
  display_info(F("LCD init done...")); delay(2000);

// we use serial for log messages
  Serial.begin(9600);
  display_info(F("Serial started..."));
  blink(2,10); //LED13 blink n pulses, 1 sec

// RTC, start
  display_info(F("Start RTC, read.... ")); delay(1000);
  
  if (! rtc.begin()) 
  {
    display_info(F("Couldn't find RTC")); delay(2000);
    blink(30,1);  // pulse LED13 3.0 sec at 0.1 Hz 
  }

  if (rtc.lostPower()) 
  {             //012345678901234567890
    display_info("RTC lost power!     "); delay(2000);
    blink(10,1);  // pulse LED13 n at 0.1 Hz 
  }

  if (EEPROM.read(0) != EEPROM_SIGNATURE)
  {
    display_info("EEPROM signature != "); delay(2000);
    eepromInit();
    blink(10,1);  // pulse LED13 n at 0.1 Hz
  }
  else
  {
    display_info("Commutations read..."); delay(2000);
    for (u16 i = 0; i < 4; i++)
    {
      pTimeCommute[i].read_EEPROM(i);
    }
  }

  myTime = rtc.now(); // get the RTC time

  menu = smenu = 0; // clear menu setting

              //012345678901234567890
  display_info(F("Start polling loops  ")); delay(1000);
  pulse_500ms.start(poll_loop_5, 1000L * 500);
  pulse_100ms.start(poll_loop_100ms, 1000L * 50);

  display_info(F("Programme pret.      "));
  log_msg(F(__PROG__ " " VERSION "\n"));

} // setup

/*  eepromInit()
    ------------
    Prepare the datas of the EEPROM
*/
void eepromInit()
{
      // Formatter l'EEPROM
    for (uint16_t i = 0 ; i < EEPROM.length() ; i++) 
       EEPROM.write( i, 0 );
    Serial.println( "EEPROM cleared!" );
    
    // Valeur par défaut pour savoir si l'EEPROM est formatté
    EEPROM.write( 0, EEPROM_SIGNATURE ); 
    Serial.println( "EEPROM signature written..." );

    // Définir une version
    EEPROM.write( 1, EEPROM_VERSION ); 
    Serial.println( "Version written." );
    
}

/* blink(short n=1, short t=1)
   ---------------------------
   Blink n times, with impulse cycle t (1/10 sec)
   *** Used only in setup operations ***
   I/O used: LED13
   return: -
*/
void blink(short n=1, short t=1)
{
  for (short i=0; i<n; i++)
  {
    digitalWrite(LED13, LOW);
    delay(50 * t);
    digitalWrite(LED13, HIGH);
    delay(50 * t);
  }
}

void log_msg(String msg)
{
  Serial.println(msg);
}

/*  display_clock(void)
    ------------------
    Write the value of date & time on line 3
    Global var modified:
    - myTime
    - dateTimeStr, with the new value
*/
void display_clock(void)
{
  snprintf(dateTimeStr, sizeof(dateTimeStr), "%04d-%02d-%02d %02d:%02d:%02d",
          myTime.year(),
          myTime.month(),
          myTime.day(),
          myTime.hour(),
          myTime.minute(),
          myTime.second()
          );
  lcd->set_cursor(0, 1); // column, line
  lcd->print(dateTimeStr);  
}

/* CmuteRel::on_LED()
   ------------------
   Load timer, and set LED_C to blink.
   Does not take caution of the REL state
 */
void CmutRel::on_LED()
{
  LED_ON();
  timer_LED = 60 * POLL_FREQ;  // loaded for one minute
  state = CR_LED;
}

/* CmuteRel::on()
   --------------
   Load timer, and set REL and LED_C to On state
 */
void CmutRel::on()
{
  RELAY_ON();
  LED_ON();
  //timer = REL_ON_DURATION * 60 * POLL_FREQ;  // loaded for an hour
  loadTimer();
  state = CR_ON;
}

void CmutRel::off()
{
    timer = 0; 
    timer_LED = 0;
    RELAY_OFF();  
    LED_OFF();
    state = CR_OFF;
}

/*  run()
    -----
    Machine state for relay commutation
    while a minute, the LED_C is blinking. Then,
    the relay is ON while the timer is counting down.
    - IO used: LED_C, REL
    - var used: state, timer_LED, timer
 */
uint8_t CmutRel::run()
{
  switch (state)
  {
  case CR_OFF :
  break;

  case CR_LED :
    if (timer_LED == 0) // Q: time of blinking LED done?
    {                   // A: yes, relay must be on
      on();
    }
    else                // A: no, decrement timer
    {
      --timer_LED;      // and blink the LED as the half second
      digitalWrite(LED_C, (timer_LED & 0x1));     
    }
  break;

  case CR_ON:
    if (timer <= 0) // Q: timer done?
    {
      off();        // A: yes, relay off
    }
    else
    {
      --timer;      // A: non, decrement timer
    }
  break;
    
  default:
    // off();
    break;
  }
  return state;
}

/*  chk_relay_time()
    ----------------
    Once a minute, check in the commutation time if a match is found.
    Start the LED_C to blink
    00:00 is considered as uninitialized and do nothing.
    
 */
void chk_relay_time()
{
  for (short i=0; i<4; i++)
    if (pTimeCommute[i].chk_time() )
    {
      char ln_menu[21];
      cmutRel.on_LED();
      cmutRel.commute_table = i+1; // memorize the table found

      snprintf(ln_menu, sizeof(ln_menu), "COMM. %1d -> %02d:%02d ", 
      cmutRel.commute_table, pTimeCommute[i].get_hh(), pTimeCommute[i].get_mm());        
      display_info(ln_menu, 3);
    }
        
}

/*  poll_loop_5()
    -------------
    Called each 500 ms by the scheduler
    Main loop of the programm
 */
void poll_loop_5()
{
  myTime = rtc.now(); // take the actual time
  digitalWrite(LED13, !digitalRead(LED13)); // phase reversed
  display_clock();  
  display_menu();
  if (myTime.second() == 1) // once per minute
    chk_relay_time();
  cmutRel.run();
}

void poll_loop_100ms()
{
  // scan all switches
  for(short i=0; i<SW_NB; i++)
  {
    sw[i]->scan();
  }
  menu_select();
}

/*  main loop
 *  ---------
 */
void loop()
{
  jm_Scheduler::cycle();
  yield();
} // end loop()

#endif // MAIN