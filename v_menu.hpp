/*
 v_menu.hpp
 ---------
 00.08.2019 - ymasur@microclub.ch
 */
#include <arduino.h>
#include "v_cmd.hpp"
#include "v_rtc.hpp"

#ifndef V_MENU_HPP
#define V_MENU_HPP

#define LN_LCD_LENGHT 20

#define B_ACT 0   // define logical definition
#define B_PLUS 1
#define B_MINUS 2
#define B_OK 3

#ifdef MAIN
  #define CLASS
#else
  #define CLASS extern
#endif

CLASS volatile u8 menu;
CLASS volatile u8 smenu;

// fct prototypes
void menu_select();
void display_menu();
void display_info(String);
void display_info(String, u8);

/*
  class Sw
  --------
  Follow the switch sate.
  Called in a periode of 100 ms, compute the time of actived/desactived switch.
  The hardware connection must be contact closed to 0V -> On state.
*/
class Sw
{
  private:
  volatile unsigned short timer;  //0..30000 * 10 ms, 0..300 s
  volatile bool state; //true: active; false: inact.
  String name;
  u8 pin;
  
  public:
  Sw(u8 pin_in, const String pin_name)
  {
    pin = pin_in;
    pinMode(pin, INPUT_PULLUP);   // useful: no external resistor needed
    digitalWrite(pin, HIGH);      // ensure the level high trough pull-up
    timer = 0;
    state = false;                // true: switch is On
    name = pin_name;
  }

/*  scan()
    ------
    Must be called each 100 ms
 */
  void scan()
  {
    if (timer<30000) timer++;
    bool in = !digitalRead(pin);  // get the reversed value
    if (in != state)    // Q: pin state changed?
    {                   // A: yes,
      state = !state;
      timer = 0;
    }
  }

  public:
  inline bool getSt(){ return state; }
  inline char getStChr(){ return (state ? '1':'0'); }
  inline short getTm(){ return timer; }
  inline String getName(){ return name; }
  inline bool getPressed(){ if (state==true && timer>=1) return true; return false; }
  inline bool getChanged(){ if (timer<=1) return true; return false; }
  inline bool getActivated(){ if (getChanged() && getPressed()) return true; return false; }

};

CLASS Sw* sw[4];  // 4 switchs are needed (instancied with 'new')
#endif