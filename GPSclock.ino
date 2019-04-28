//#define LOCALTZ_POSIX	"CET-1CEST,M3.5.0/2,M10.5.0/3"		// Time in Berlin

#define GMToffset 1 //offset in hours from UTC/GMT

#include <TimeLib.h> //Thanks Paul! :)                  
#include <SevSeg.h>

SevSeg sevseg; //Instantiate a seven segment controller object

void setup() {
  Serial1.begin(9600); 
  Serial.begin(115200);

  byte digitPins[] = {A1, 15, 16, 9, 7, 5};
  //                    A    B   C   D   E   F   G  DP
  byte segmentPins[] = {A0,  6, A3, 10, 14,  4,  8, A2}; 
  //sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
  sevseg.begin(COMMON_CATHODE, 6, digitPins, segmentPins, true, false, false, true);
  sevseg.setBrightness(100);
}

void loop() {
  static byte yearUTC, monthUTC, dayUTC, hourUTC, Minute, Second; 
  static String rxString;
  static bool gotTime=false;
  static bool gotDate=false;
  static unsigned long gotDateAt;
  static unsigned long gotTimeAt;

  if(Serial1.available()) {
    char rxChar=Serial1.read();
    rxString+=String(rxChar);
    if(rxChar=='\n') {
      //Serial.print(rxString);
      String identifier=rxString.substring(0,6);
      if(identifier == "$GPGGA") {
        String timeString=rxString.substring(7,13);
        //Serial.println(timeString);
        
        if(timeString==",,,,,0") gotTime=false; //this is what the substring looks like before GPS fix
        else{
          hourUTC=(byte)timeString.substring(0,2).toInt();
          Minute=(byte)timeString.substring(2,4).toInt();
          Second=(byte)timeString.substring(4,6).toInt();
          //Serial.print(hourUTC); Serial.print(':'); Serial.print(Minute); Serial.print(':'); Serial.println(Second);

          gotTime=true;
          gotTimeAt=millis();

          if(gotDate){
            setTime(hourUTC, Minute, Second, dayUTC, monthUTC, yearUTC);
            adjustTime(GMToffset*SECS_PER_HOUR); 
            if(isDst(now())) adjustTime(SECS_PER_HOUR);
            printTime();
          }
        }
      }
      
      else if(identifier == "$GPRMC") {
        if(rxString=="$GPRMC,,V,,,,,,,,,,N*53") gotDate=false;
        else {
          //$GPRMC,222920.00,V,,,,,,,150419,,,N*7C
          //Serial.println(rxString);
          int dateStartIndex=rxString.lastIndexOf(',');
          //Serial.println("dateStartIndex: " + String(dateStartIndex)); 
          for(int commaCounter=1; commaCounter<4; commaCounter++) {
            dateStartIndex=rxString.lastIndexOf(',', dateStartIndex-1); //work backwards through the commas
            //Serial.println("dateStartIndex: " + String(dateStartIndex)); 
          }
          dateStartIndex++; //position of the first char of the date.
        
          dayUTC   = (byte)rxString.substring(dateStartIndex,dateStartIndex+2).toInt();
          monthUTC = (byte)rxString.substring(dateStartIndex+2,dateStartIndex+2+2).toInt();
          yearUTC  = (byte)rxString.substring(dateStartIndex+4,dateStartIndex+4+2).toInt();
          
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
  if(!gotDate || !gotTime) sevseg.setChars("NO GPS");

  sevseg.refreshDisplay(); // Must run repeatedly
}

static inline printTime() {
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

/*
 * Edited from avr-libc/include/util/eu_dst.h
 * (c)2012 Michael Duane Rice All rights reserved.
*/

//DST goes true on the last sunday of march @ 2:00 (CET)
//DST goes false on the last sunday of october @ 3:00 (CEST)
 
#define MARCH 3
#define OCTOBER 10
#define SHIFTHOUR 2 

bool isDst(time_t epoch) { //eats local time (CET/CEST)

  uint8_t mon, mday, hh, day_of_week, d;
  int n;

  mon = month(epoch);
  day_of_week = weekday(epoch)-1; //paul's library sets sunday==1, this code expects "days since sunday" http://www.cplusplus.com/reference/ctime/tm/
  mday = day(epoch)-1; //zero index the day as well
  hh = hour(epoch);

  if ((mon > MARCH) && (mon < OCTOBER)) return true;
  if (mon < MARCH) return false;
  if (mon > OCTOBER) return false;

  //determine mday of last Sunday 
  n = mday;
  n -= day_of_week;
  n += 7;
  d = n % 7;  // date of first Sunday
  if(d==0) d=7; //if the month starts on a monday, the first sunday is on the seventh.

  n = 31 - d;
  n /= 7; //number of Sundays left in the month 

  d = d + 7 * n;  // mday of final Sunday 

  //If the 1st of the month is a thursday, the last sunday will be on the 25th.
  //Apparently this algorithm calculates it to be on the 32nd...
  //Dirty fix, until something smoother comes along:
  if(d==31) d=24;

  if (mon == MARCH) {
    if (d > mday) return false;
    if (d < mday) return true;
    if (hh < SHIFTHOUR) return false;
    return true;
  }
  //the month is october:
  if (d > mday) return true; 
  if (d < mday) return false; 
  if (hh < SHIFTHOUR+1) return true;
  return false;
}