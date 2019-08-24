/*
  v_menu.hpp
  ---------
  24.08.2019 - ymasur@microclub.ch
*/
#include <Arduino.h>
#include "v_cmd.hpp"
#include "v_rtc.hpp"
#include "v_menu.hpp"

#ifndef V_MENU_CPP
#define V_MENU_CPP

static DateTime newTime; // Used to handle the time of the menu CLOCK

/*
    menu_select()
    -------------
*/
void menu_select()
{

  if ( sw[B_ACT]->getPressed() && sw[B_OK]->getActivated() )  // Q: ACT and OK pressed?
  { menu = smenu = 0; }                                       // A:yes, reset menu

  if (menu == 0 && smenu == 0 &&                              // Q: ACT and OK pressed ~ 5 seconds?
      sw[B_ACT]->getPressed() && sw[B_ACT]->getTm() > 400 &&
      sw[B_OK]->getPressed() && sw[B_OK]->getTm() > 400     ) 
  { 
    EEPROM.read(1); // bidon - compiler warning
    eepromInit();                                             // A: yes, EEPROM data cleared
    smenu = 1; 
  }                               

  if (sw[B_ACT]->getActivated())
  {
    menu ++; // 0..3 menus
    smenu = 0;
    if (menu > 3) menu = 0;
  } 
  
  if (menu == 1)  // --- MENU RELAIS ---
  {
    if (smenu == 0)
      smenu = 1 + cmutRel.getSt();  // direct show the state

    if (sw[B_PLUS]->getActivated()) // + is REL On
      smenu = 2;

    if (sw[B_MINUS]->getActivated()) // - is REL Off
      smenu = 1;
    
    if (sw[B_OK]->getActivated()) // OK get the last value and apply
    {
      if(smenu==2)
        cmutRel.on();
      else
        cmutRel.off();  

      menu = smenu = 0; // end
    }
  }

  if (menu == 2) // --- MENU COMMUTATION
  {
    static u8 commute_nb=0;

    if (sw[B_OK]->getActivated())  
    {
      if (smenu == 0)
        smenu = 1;   // selct the table number
      else
        smenu+=10;  // next field of table
    }

    if (smenu == 0)
    {
      /* */
    }
    else if (smenu <10)       // Table selection (1..4)
    {
      if (sw[B_PLUS]->getActivated()) smenu++;
      if (sw[B_MINUS]->getActivated()) smenu--;
      BOUND(smenu, 1, 4);
      commute_nb = smenu-1;
    }
    else if (smenu < 20)      // OK pressed once: HH modified (00-23)
    {
      if (sw[B_PLUS]->getRepeted())
        pTimeCommute[commute_nb].inc_hh();

      if (sw[B_MINUS]->getRepeted())
        pTimeCommute[commute_nb].dec_hh();
    }
    else if(smenu < 30)       // OK pressed twice: MM modified (00-59)
    {
      if (sw[B_PLUS]->getRepeted())
        pTimeCommute[commute_nb].inc_mm();

      if (sw[B_MINUS]->getRepeted())
        pTimeCommute[commute_nb].dec_mm();     
    }
    else if(smenu < 40)       // OK pressed thirdly: wd modified (0..7)
    {
      if (sw[B_PLUS]->getActivated())
        pTimeCommute[commute_nb].dec_wd(); // inc and dec reversed, visually more intuitive

      if (sw[B_MINUS]->getActivated())
        pTimeCommute[commute_nb].inc_wd();     
    }

    if (smenu >50)          // save values, end
    {
      pTimeCommute[commute_nb].save_EEPROM(commute_nb);
      menu = smenu = 0;     //reset menu
    } 
  } // menu 2

  if (menu == 3)      // --- MENU HORLOGE ---
  {
    uint16_t yy=0;
    uint8_t mm=0, dd=0, hh=0, mi = 0;

    if (smenu <1)            // Time selection
    {
      newTime = rtc.now();  // read actual value of the RTC
    }
    else if (smenu < 2)      // year
    {
      if (sw[B_PLUS]->getRepeted()) yy++;
      if (sw[B_MINUS]->getRepeted()) yy--;
    } 
    else if (smenu < 3)      // month
    {
      if (sw[B_PLUS]->getRepeted()) mm++;
      if (sw[B_MINUS]->getRepeted()) mm--;
    }
    else if (smenu < 4)      // day
    {
      if (sw[B_PLUS]->getRepeted()) dd++;
      if (sw[B_MINUS]->getRepeted()) dd--;
    }
    else if (smenu < 5)      // hour
    {
      if (sw[B_PLUS]->getRepeted()) hh++;
      if (sw[B_MINUS]->getRepeted()) hh--;
    }
    else if (smenu < 6)      // minute
    {
      if (sw[B_PLUS]->getRepeted()) mi++;
      if (sw[B_MINUS]->getRepeted()) mi--;
    }

    if (sw[B_OK]->getActivated())  smenu+=1;

    if (smenu > 6)          // last field pointed, end
    {
      rtc.adjust(newTime);
      menu = smenu = 0;     //reset menu
    } 
    newTime = DateTime(newTime.year()+yy, 
              newTime.month()+mm, newTime.day()+dd, 
              newTime.hour()+hh, newTime.minute()+mi, 0 );
  } // Menu 3
} // fct menu_select() ;

// --- DISPLAY PART ----------------------------------------------------------

// LCD display info (standard: line 3 of 4)
void display_info(String info, u8 ln=2)
{
  lcd->set_cursor(0, ln); // column, line
  lcd->print(info);
}

void display_info(String info)
{
  display_info(info, 2);
}

/*  display_tm_left()
    -----------------
    Display the time left if the REL is On.
    Return: -
 */
void display_tm_left()
{
  char timeLeft[LN_LCD_LENGHT+1];
  snprintf(timeLeft, sizeof(timeLeft), "RELAIS On pour:%03d m", cmutRel.getTimeLeft() );
  lcd->print(timeLeft);
  String msg = F("Time left:"); msg += cmutRel.getTimer(); log_msg(msg);
}

// display messages for main menu
const char menu_msg[][LN_LCD_LENGHT+1] = {
// 01234567890123456789
  "RELAIS          ",   // mention ": ON" / ":OFF" complete the line
  "COMMUTATIONS        ", 
  "HORLOGE             "
  };

/*  display_menu()
    --------------
    Display menus, according to state of menu ans sub-menu
    Then, shows satus
    Globals vars used:
    - menu: main menu level
    - smenu: sub-menu level, or field
    Vars modified:
    - nothing
    Return: -
 */
void display_menu()
{
  char ln_menu[LN_LCD_LENGHT+1]; // used to prepare a line written to the LCD

  lcd->set_cursor(0, 2); // we use lines 3 and 4
      
  if (menu)
  {
    snprintf(ln_menu, sizeof(ln_menu), "menu:%d - smenu:%d", menu, smenu);
    Serial.println(ln_menu);
  }

  switch(menu)
  {
    /* ---------------- BASE MENU -------------- */
    case 0:
      switch(cmutRel.getSt()) // follow REL activity
      {
      case CR_OFF:    // resting state
        //          12345678901234567890
        lcd->set_cursor(0,3);
        lcd->print(F("AC:choix, OK:valider"));
        lcd->set_cursor(0,2);
        lcd->print(F("Menu principal      "));
      break;
      case CR_LED:  // relay ready to be commuted to On
        lcd->print(F("Encl. imminent      "));
      break;
      case CR_ON:   // relay is On state
        display_tm_left();
      break;
      }
    break; // base menu

    /* ---------------- RELAY MENU --------------- */
    case 1:   
      //log_msg(menu_msg[0]);
      lcd->print(menu_msg[0]);

      if (smenu == 1)
      {
        log_msg(F("RELAY:OFF"));
        lcd->print(F(":OFF"));
      }     

      if (smenu == 2)
      {
        log_msg(F("RELAY: ON"));
        lcd->print(F(": ON"));
      }

      break; // relay menu

    /* ---------------- SWITCHING TABLE ------------- */
    case 2: 
      if (smenu == 0)
      {
        log_msg(menu_msg[1]);
        lcd->print(menu_msg[1]);
      }
      else
      {
        u8 n = smenu%10;  // compute index of TimeCommute table
        u8 c, field;
        n = n-1;  // smenu 10 gives the field
        snprintf(ln_menu, sizeof(ln_menu), "COMM.%1d %02d:%02d LMMJVSD", 
          smenu%10, pTimeCommute[n].get_hh(), pTimeCommute[n].get_mm());     
        display_info(ln_menu, 2);

        snprintf(ln_menu, sizeof(ln_menu), "COMM.%1d %02d:%02d J:%d", 
          smenu%10, pTimeCommute[n].get_hh(), pTimeCommute[n].get_mm(), 
          pTimeCommute[n].get_wd());    
        Serial.println(ln_menu);

        field = smenu / 10; field++;
        switch(field)     // compute the field used to inc/dec
        {
          case 1: c = 5; break; // Comm. table
          case 2: c = 8; break; // HH
          case 3: c = 11; break;  // MM
          case 4: // day of the week is special
            if (pTimeCommute[n].get_wd()==0)  // Q: all days?
              c = 0;  // A: yes, special
            else
              c = 12 + pTimeCommute[n].get_wd();
          break;
          case 5: 

          default: 
            c = 0; 
          break;  
        }
        // clear bottom line
        lcd->set_cursor(0, 3); lcd->print_space(LN_LCD_LENGHT); 

        if (field < 5)
        {
          if (c)  // Q: field given?
          {       // A: yes, show it
            lcd->set_cursor(c, 3); lcd->print("*");
          }
          else    // special: all days to show
          {
            lcd->set_cursor(13,3); lcd->print(F("-------"));
          }
        }
        else
        {
          lcd->set_cursor(0, 3); lcd->print(F("Sauver? "));
        }
        
      }
      
      break; // case 2, switching table menu

    /* -------------- CLOCK ADJUST -------------- */
    case 3: 
      if (smenu == 0)
      {
        log_msg(menu_msg[2]);
        lcd->print(menu_msg[2]);
      }
      else // fields are selected to be modified: diplay values
      { 
        char dateTimeStr[LN_LCD_LENGHT+1];
        snprintf(dateTimeStr, sizeof(dateTimeStr), "MOD.%04d-%02d-%02d %02d:%02d",
          newTime.year(),
          newTime.month(),
          newTime.day(),
          newTime.hour(),
          newTime.minute()
          );

        lcd->set_cursor(0, 2); // display new date on line 3 of LCD
        lcd->print(dateTimeStr);
        Serial.print(" -> ");
        Serial.println(dateTimeStr);
        // clear bottom line
        lcd->set_cursor(0, 3); lcd->print_space(LN_LCD_LENGHT);

        if (smenu < 6)  // Q: fields to be modified?
        {               // A: yes, display field on the bottom line
          lcd->set_cursor(4+(smenu*3), 3);
          lcd->print("*");
        }
        else            // A: no, ask the user for writing
        {
          lcd->set_cursor(0, 3);
          lcd->print(F("Ajuster? "));
        }
        
      } // if smenu <> 0
      break; // clock menu

    default:
      break;
  } // switch(menu)
} // fct display_menu()

#endif