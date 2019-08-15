/*
 v_rtc.hpp
 ---------
 00.08.2019 - ymasur@microclub.ch
*/
#ifndef V_RTC_HPP
#define V_RTC_HPP
#include <time.h>
#include <RTClib.h>
#include "v_cmd.hpp"

#ifdef MAIN
  #define CLASS
#else
  #define CLASS extern
#endif

// time given by Unix cmd              1         2
//                           012345678901234567890
// asci format, as          "2019-08-29 22:10:42"
#define LN_LCD_LENGHT 20
CLASS char dateTimeStr[LN_LCD_LENGHT+1]; 

class TimeCommute
{
  private:
  u8 hh; // hour
  u8 mm; // minute
  u8 wd;

  public:
  TimeCommute()
  {
    hh = 22;  // by default
    mm = 00;
            // -LMMJVSD
    wd = 4; // 0 mean all days, Monday=1
  }

  bool chk_time();
  void inc_hh(){hh++; if (hh>23) hh=0;} // roll down
  void inc_mm(){mm++; if (mm>59) mm=0;}
  void dec_hh(){hh--; if (hh>23) hh=23;} // goes to 255...
  void dec_mm(){mm--; if (mm>59) mm=59;}
  void inc_wd(){wd++; if (wd >7) wd=0;}
  void dec_wd(){wd--; if (wd==254) wd=0;}
  int get_hh(){return hh;}
  int get_mm(){return mm;}
  int get_wd(){return wd;}

  void display();

};

CLASS const char *swd[] // String for day of week
#ifdef MAIN //Mon  Tue  Wed  Thu  Fri  Sat  Sun
= {"LMMJVSD", "L", "M", "M", "J", "V", "S", "D" };
#else
;
#endif

// 4 tables for commutation, static storage
CLASS TimeCommute timeCommute1, timeCommute2, timeCommute3, timeCommute4;
CLASS TimeCommute pTimeCommute[]  // useful in an array
#ifdef MAIN
= {timeCommute1, timeCommute2, timeCommute3, timeCommute4};
#else
;
#endif

CLASS RTC_DS3231 rtc;
CLASS DateTime myTime;

#endif // V_RTC_HPP