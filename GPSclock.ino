//#define LOCALTZ_POSIX	"CET-1CEST,M3.5.0/2,M10.5.0/3"		// Time in Berlin

#include <TimeLib.h>                  

#include <SevSeg.h>
SevSeg sevseg; //Instantiate a seven segment controller object
void setup() {

  byte digitPins[] = {A1, 15, 16, 9, 7, 5};
  //                    A    B   C   D   E   F   G  DP
  byte segmentPins[] = {A0,  6, A3, 10, 14,  4,  8, A2}; 
//  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
  sevseg.begin(COMMON_CATHODE, 6, digitPins, segmentPins, true, false, false, false);
  sevseg.setBrightness(100);

    Serial1.begin(9600); 
    Serial.begin(115200);
    //byte settingsArray[] = {0x03, 0xFA, 0x00, 0x00, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //
    //configureUblox(settingsArray); 


}

int GMToffset=1;
int dstOffset=1;

unsigned long tick;
char rxChar;
String rxString;
String timeString;

bool gotTime=false;
bool gotDate=false;
unsigned long gotDateAt;
unsigned long gotTimeAt;

int yearUTC;
byte monthUTC, dayUTC, hourUTC, Minute, Second; 

void loop() {

//  if(Serial1.available()) Serial.print(Serial1.read(),HEX); 
//  if(Serial1.available()) Serial.write(Serial1.read()); 
  if(Serial1.available()) 
  {
    rxChar=Serial1.read();
    rxString+=String(rxChar);
    if(rxChar=='\n') {
          String identifier=rxString.substring(0,6);
          //Serial.print(rxString);
          if(identifier == "$GPGGA") {
              //Serial.print(rxString);
              timeString=rxString.substring(7,13);
              //Serial.println(timeString);
              
              if(timeString==",,,,,0") gotTime=false; //this is what the substring looks like before GPS fix

              else{
              
                hourUTC=(byte)timeString.substring(0,2).toInt();
                Minute=(byte)timeString.substring(2,4).toInt();
                Second=(byte)timeString.substring(4,6).toInt();

                gotTime=true;
                gotTimeAt=millis();

                if(gotDate){
              
                  setTime(hourUTC, Minute, Second, dayUTC, monthUTC, yearUTC);

                  adjustTime(GMToffset*SECS_PER_HOUR); 

                  if(isDst(now())) adjustTime(SECS_PER_HOUR);
                }
            }
              //Serial.print(hourUTC); Serial.print(':'); Serial.print(Minute); Serial.print(':'); Serial.println(Second);

          }
          else if(identifier == "$GPRMC") {

            if(rxString=="$GPRMC,,V,,,,,,,,,,N*53") gotDate=false;
            else{
            //$GPRMC,222920.00,V,,,,,,,150419,,,N*7C
              Serial.println(rxString);
              int dateStartIndex=rxString.lastIndexOf(',');
              //Serial.println("dateStartIndex: " + String(dateStartIndex)); 
              for(int commaCounter=1; commaCounter<4; commaCounter++) {
                dateStartIndex=rxString.lastIndexOf(',', dateStartIndex-1); //work backwards through the commas
                //Serial.println("dateStartIndex: " + String(dateStartIndex)); 
              }
              dateStartIndex++; //position of the first char of the date.
            
              dayUTC   =(byte)rxString.substring(dateStartIndex,dateStartIndex+2).toInt();
              monthUTC =(byte)rxString.substring(dateStartIndex+2,dateStartIndex+2+2).toInt();
              yearUTC  = 2000 + rxString.substring(dateStartIndex+4,dateStartIndex+4+2).toInt();
              gotDate=true;
              gotDateAt=millis();
            

              //Serial.print(dayUTC); Serial.print(' '); Serial.print(monthUTC); Serial.print(' '); Serial.println(yearUTC);
            }        
          }
          
          //else if(identifier == "$GPZDA") Serial.print(rxString); //i'd like to see one of these...
          //Serial.println("------");
            
      rxString="";
    }
  }

  if(millis()>gotTimeAt+1500) gotTime=false;
  if(millis()>gotDateAt+1500) gotDate=false;


  if(gotDate && gotTime){
    if(millis()>tick+1000){
      tick=millis();
      String timeConcat="";
      if(hour()<10) timeConcat+="0";
      timeConcat+=String(hour());
      if(minute()<10) timeConcat+="0";
      timeConcat+=String(minute());
      if(second()<10) timeConcat+="0";
      timeConcat+=String(second());

      char charArray[7]; //HHMMSS\0
      timeConcat.toCharArray(charArray,7);
      sevseg.setChars(charArray);
      //Serial.println(charArray);
    }
  }
  else sevseg.setChars("NO GPS");


  sevseg.refreshDisplay(); // Must run repeatedly


}


/*
 * Edited from avr-libc/include/util/eu_dst.h
 * (c)2012 Michael Duane Rice All rights reserved.
*/

#define MARCH 3
#define OCTOBER 10
#define SHIFTHOUR 2 

bool isDst(time_t epoch){ //eats local time

        uint8_t         mon, mday, hh, day_of_week, d;
        int             n;
                        
                        //DST goes true on the last sunday of march @ 2:00
                        //DST goes false on the last sunday of october @ 3:00

                        mon = month(epoch)-1;
                        day_of_week = weekday(epoch)-1; //paul's library sets sunday==1, this code expects "days since sunday" http://www.cplusplus.com/reference/ctime/tm/
                        mday = day(epoch) - 1;
                        hh = hour(epoch);

       if              ((mon > MARCH) && (mon < OCTOBER))
                            return true;

        if              (mon < MARCH)
                            return false;
        if              (mon > OCTOBER)
                            return false;

         //determine mday of last Sunday 
                        n = mday;
                        n -= day_of_week;
                        n += 7;
                        d = n % 7;  // date of first Sunday

                        n = 31 - d;
                        n /= 7; //number of Sundays left in the month 

                        d = d + 7 * n;  // mday of final Sunday 

        if              (mon == MARCH) {
            if (d < mday)
                return false;
            if (d > mday)
                return true;
            if (hh < SHIFTHOUR)
                return false;
            return true;
        }
        //the month is october:
        if              (d < mday)
                            return true;
        if              (d > mday)
                            return false;
        if              (hh < SHIFTHOUR+1)
                            return true;
                        return false;

    }