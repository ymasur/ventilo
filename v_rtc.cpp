/*
 v_rtc.cpp
 ---------
 24.08.2019 - ymasur@microclub.ch
*/
#ifndef V_RTC_CPP
#define V_RTC_CPP
#include <time.h>
#include <RTClib.h>
#include "v_rtc.hpp"

/*  TimeCommute::display()
    ----------------------
    Write on LCD the time of the commutation table, and the day of the week
    Return: -
*/
void TimeCommute::display()
{
  char sTimecommute[20];  // "hh:22 mm:00"
  const char *p = swd[wd];
  snprintf(sTimecommute, sizeof(sTimecommute), ("hh:%02d mm:%02d %s"), hh, mm, p);
  Serial.println(sTimecommute);
  lcd->set_cursor(0, 2); // column, line
  lcd->print("TimeCommute:");
  lcd->print(sTimecommute);
}

/*  TimeCommute::chk_time()
    -----------------------
    Check if it is time to commute.
    The uninitilized data, also 00:00 make nothing.
    The weekday can be 1..7, with 7=Sunday
    If weekday is 0, the commute time is available for all days

    The check mus be called each minute.
    
    Return: true if the time match, else false.
*/
bool TimeCommute::chk_time()
{
  if (hh+mm == 0) // uninitialized datas
    return false;
  // wd = 0 is for all days
  if (wd == 0 && myTime.hour() == hh && myTime.minute() == mm)
    return true;
  // check with wd...
  u8 day_of_w = myTime.dayOfTheWeek();
  if (day_of_w == 0) day_of_w = 7;  // for us, Sunday is 7, not 0!
  if (day_of_w == wd && myTime.hour() == hh && myTime.minute() == mm)
    return true;
  // nothing found
  return false;
}

/*  read_EEPROM()
    -------------
    Read values stored in the EEPROM.
    Parm: n = set of TimeCommute table (0.. n)

    Modified values:
    - hh, mm and wd

    Return: nothing
*/
void TimeCommute::read_EEPROM(u16 n)
{
    u16 pos = n * 3 + EEPROM_DATA_OFFSET;

    hh = EEPROM.read(pos);
    mm = EEPROM.read(++pos);
    wd = EEPROM.read(++pos);
}

void TimeCommute::save_EEPROM(u16 n)
{
  if( EEPROM.read(1) == EEPROM_VERSION)
  {
    uint16_t pos = n * 3 + EEPROM_DATA_OFFSET;
    EEPROM.write(pos, hh);
    EEPROM.write(++pos, mm);
    EEPROM.write(++pos, wd);
  }
  else
  { //error
  };
}
#endif