#include <avr/wdt.h>
#include <LiquidCrystal_I2C.h>
//#define BLYNK_DEBUG // Optional, this enables lots of prints
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <BlynkSimpleEthernet.h>
#include <Ethernet.h>
#include <Wire.h>
#include <SPI.h>
LiquidCrystal_I2C lcd(0x3F, 20, 4);
#include "RTClib.h"
RTC_DS1307 RTC;
#include <SimpleTimer.h>

char auth[] = "30a0c70e138a4b3aa3d0d4ce50b6afbe";
/////////////////////////////////TIMERS DECLERATION CODE STARTS/////////////////////////////////
SimpleTimer timer;
SimpleTimer voltageprocess;
SimpleTimer rtc;
SimpleTimer current;
SimpleTimer empty;
/////////////////////////////////TIMERS DECLERATION CODE ENDS/////////////////////////////////

/////////////////////////////////BRIDGE DECLERATION CODE STARTS/////////////////////////////////
WidgetBridge bridge1(V25);
/////////////////////////////////TIMERS DECLERATION CODE ENDS/////////////////////////////////

///////////////////// Temperature Monitor Section Starts/////////////////////////
// Temperature Related Reading
const int tempInPin = A0;
int tempInValue ; //temperature read
int tempOutDeg ;
int tempadjustment = 45;
float h;
///////////////////// Temperature Monitor Section ENDS///////////////////////////

WidgetLED led1(V3); //register to virtual pin 3
WidgetLED ledR1(V10); //register to virtual pin 3
WidgetLED ledR2(V11); //register to virtual pin 3
WidgetLED ledR3(V12); //register to virtual pin 3
WidgetLED ledR4(V13); //register to virtual pin 3
WidgetLED ledP(V17); //register to virtual pin 3
WidgetLED ledD(V20); //register to virtual pin 3
const int relay1 = 28;  //CFL
const int relay2 = 26;  //LEFT LED
const int relay3 = 24;  //RIGHT LED
const int relay4 = 22;  //AUX
int g;  ///////////////// remote arduino uptime variable

///////////////////// Voltage Monitor Section Starts//////

int BatIn = A2;
float vout = 0.0;
float vin = 0.0;
float R1 = 100000.0; // resistance of R1 (100K)
float R2 = 10000.0; // resistance of R2 (10K)
int value = 0;

//////////////////////Voltage Monitor Section Ends////////

//////////////////////Current Monitor Section Starts////////
const int currentPin = A3;
const unsigned long sampleTime = 100000UL;
const unsigned long numSamples = 250UL;
const unsigned long sampleInterval = sampleTime / numSamples;
const int adc_zero = 510;
//////////////////////Current Monitor Section Ends////////

/////////////////////////STATIC IP CODE STARTS/////////////////
// Mac address should be different for each device in your LAN
byte arduino_mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xEE };
IPAddress arduino_ip ( 192,   168,   0,  2);
IPAddress dns_ip     (  8,   8,   8,   8);
IPAddress gateway_ip ( 192,   168,   0,  1);
IPAddress subnet_mask(255, 255, 255,   0);
/////////////////////////STATIC IP CODE ENDS/////////////////

void setup()
{
  lcd.init();
  lcd.setBacklight(HIGH);
  Serial.begin(9600);
  lcd.setCursor (0, 0);
  lcd.print(" PLEASE  WAIT WHILE ");
  lcd.setCursor (0, 1);
  lcd.print(" THE CONNECTION  TO ");
  lcd.setCursor (0, 2);
  lcd.print("  THE BLYNK SERVERS ");
  lcd.setCursor (0, 3);
  lcd.print("   IS  ESTABLISHED  ");
  // Blynk.begin(auth);
  Blynk.begin(auth, "cloud.blynk.cc", 8442, arduino_ip, dns_ip, gateway_ip, subnet_mask, arduino_mac);
  wdt_enable(WDTO_8S); ///////////// watchdog
  lcd.clear();
  lcd.setBacklight(HIGH);
  Wire.begin();
  pinMode(BatIn, INPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);


/////////////////////////////////FUNCTION TIMERS DELAY DECLERATION CODE STARTS/////////////////////////////////
  timer.setInterval(500L, sendUptime); // temperature and uptime
  voltageprocess.setInterval(1000L, voltagesense);
  rtc.setInterval(1000L, lcdclock);
  current.setInterval(1000L, amps);
  empty.setInterval(1000L, emptyroutine);
  /////////////////////////////////FUNCTION TIMERS DELAY DECLERATION CODE ENDS/////////////////////////////////
}
bool isFirstConnect = true;

// This function will run every time Blynk connection is established
BLYNK_CONNECTED() {
  bridge1.setAuthToken("6fb37a42248c40eb84c1922aacca6555");
  if (isFirstConnect) {
    Blynk.syncAll();
    isFirstConnect = false;
  }
  led1.on();
  ledR1.off();
  ledR2.off();
  ledR3.off();
  ledR4.off();
  ledP.on();

}

BLYNK_WRITE(V2)
{
  int i = param.asInt();
  if (i == 1) {
    lcd.setBacklight(HIGH);
    led1.on();
    Blynk.email("ahsan@alahsan.xyz", "LCA UNIT's LCS (AUTO TURNED ON) STATUS UPDATE", "LCA UNIT's LCD JUST TURNED ON !");
  }
  else {
    lcd.setBacklight(LOW);
    led1.off();
    Blynk.email("ahsan@alahsan.xyz", "LCA UNIT's LCS (AUTO TURNED OFF) STATUS UPDATE", "LCA UNIT's LCD JUST TURNED OFF !");

  }
}
BLYNK_WRITE(V16)
{
  int i = param.asInt();
  if (i == 1) {
    lcd.setBacklight(HIGH);
    led1.on();
    Blynk.email("ahsan@alahsan.xyz", "LCA UNIT's LCS (MANUALLY TURNED ON) STATUS UPDATE", "LCA UNIT's LCD JUST TURNED ON !");
  }
  else {
    lcd.setBacklight(LOW);
    led1.off();
    Blynk.email("ahsan@alahsan.xyz", "LCA UNIT's LCS (MANUALLY TURNED OFF) STATUS UPDATE", "LCA UNIT's LCD JUST TURNED OFF !");

  }
}
////////////////////////////////////////////INVERTED BUTTON CODE ENDS/////////////////////////////////////////////////////////////////////////////

BLYNK_WRITE(V6)
{
  int i = param.asInt();
  if (i == 0) {
    digitalWrite(relay1, HIGH);
    ledR1.off();
    lcd.setCursor (4, 3);
  }
  else {
    digitalWrite(relay1, LOW);
    ledR1.on();
    lcd.setCursor (4, 3);
  }

}

BLYNK_WRITE(V7)
{
  int i = param.asInt();
  if (i == 0) {
    digitalWrite(relay2, HIGH);
    ledR2.off();
    lcd.setCursor (11, 3);

  }
  else {
    digitalWrite(relay2, LOW);
    ledR2.on();
    lcd.setCursor (11, 3);

  }
}

BLYNK_WRITE(V8)
{
  int i = param.asInt();
  if (i == 0) {
    digitalWrite(relay3, HIGH);
    ledR3.off();
    lcd.setCursor (19, 3);
    ;
  }
  else {
    digitalWrite(relay3, LOW);
    ledR3.on();
    lcd.setCursor (19, 3);

  }
}
BLYNK_WRITE(V9)
{
  int i = param.asInt();
  if (i == 0) {
    digitalWrite(relay4, HIGH);
    ledR4.off();
  }
  else {
    digitalWrite(relay4, LOW);
    ledR4.on();
  }
}
////////////////////////////////////////////INVERTED BUTTON CODE ENDS/////////////////////////////////////////////////////////////////////////////

void loop()
{
  wdt_reset(); ///////////// watchdog
  Blynk.run();
  timer.run(); // Initiates SimpleTimer
  voltageprocess.run();
  current.run();
  rtc.run();
  empty.run();
}
void blynkAnotherDevice()
{

}
void sendUptime()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V1, millis() / 1000);
  lcd.setCursor (11, 2);
  lcd.print("UpT:");
  lcd.setCursor (15, 2);
  lcd.print(((millis() / 1000) / 60));
  {
    //Read Temperature Sensor
    tempInValue = analogRead(tempInPin);

    tempOutDeg = (5 * tempInValue * 100 / 1024) + tempadjustment;

    lcd.setCursor (0, 1);
    lcd.print("RmTEMP:");

    lcd.setCursor (7, 1);
    if (tempOutDeg < 10) {
      lcd.print(tempOutDeg);
      lcd.setCursor (8, 1);
      lcd.print(" ");
    }
    else
    {
      lcd.print(tempOutDeg);
    }

  }
  Blynk.virtualWrite(V0, tempOutDeg);
  bridge1.virtualWrite(V5, tempOutDeg);
  lcd.setCursor (11, 1);
  lcd.print("OdTEMP:");
  lcd.setCursor (18, 1);
  if (h > 100) {

    lcd.print("Er");
  }
  else if (h < 10) {
    lcd.print(h , 0);
    lcd.setCursor (19, 1);
    lcd.print(" ");
  }
  else
  {
    lcd.print(h , 0);
  }

}

void voltagesense()
{
  value = analogRead(BatIn);
  vout = (value * 5.0) / 1024.0; // see text
  vin = vout / (R2 / (R1 + R2));

  {
    if (vin < 0.09) {
      vin = 0.0; //statement to quash undesired reading !
    }

    lcd.setCursor (0, 2);
    lcd.print("BAT:");
    lcd.print(vin);

    if (vin >= 13.75) {
      lcd.setCursor (4, 2);
      lcd.print(vin , 1);
    }
    else {
      lcd.setCursor (4, 2);
      lcd.print(vin , 1);
    }
    //
    //    lcd.setCursor (11, 2);
    //    lcd.print("Each:");
    //
    //    //  lcd.setCursor (5, 3);
    //    //  lcd.print((vin/2));
    //
    //    if ((vin / 2) >= 13.87) {
    //      lcd.setCursor (16, 2);
    //      lcd.print("FULL ");
    //    }
    //    else {
    //      lcd.setCursor (16, 2);
    //      lcd.print((vin / 2));
    //    }
  }

  Blynk.virtualWrite(V4, vin);
}
bool    fHasLooped  = false;
void lcdclock()
{
  DateTime now = RTC.now();
  lcd.home (); // set cursor to 0,0
  lcd.setCursor (0, 0);
  if (now.hour() < 10) { // Add a zero, if necessary, as above
    lcd.print(0);
  }
  ////////////////////////////////24 hour to 12 conversion starts//////////////////////////////
  if (now.hour() >= 13) {
    int thour = (now.hour() - 12);
    if (thour < 10) { // Add a zero, if necessary, as above
      lcd.print(0);
    }
    lcd.print(thour);
  }
  else {
    lcd.print(now.hour()); // Display the current hour
  }
  ////////////////////////////////24 hour to 12 conversion ends/////////////////////////////////
  lcd.setCursor (8, 0);
  switch (now.hour()) {
    case  13:
      lcd.print("PM");
      break;
    case 14:
      lcd.print("PM");
      break;
    case 15:
      lcd.print("PM");
      break;
    case 16:
      lcd.print("PM");
      break;
    case 17:
      lcd.print("PM");
      break;
    case 18:
      lcd.print("PM");
      break;
    case 19:
      lcd.print("PM");
      break;
    case 20:
      lcd.print("PM");
      break;
    case 21:
      lcd.print("PM");
      break;
    case 22:
      lcd.print("PM");
      break;
    case 23:
      lcd.print("PM");
      break;
    default:
      lcd.print("AM");
      break;
  }

  lcd.setCursor (2, 0);
  lcd.print(":");
  lcd.setCursor (3, 0);

  if (now.minute() < 10) { // Add a zero, if necessary, as above
    lcd.print(0);
  }
  lcd.print(now.minute(), DEC); // Display the current minutes

  lcd.setCursor (5, 0);
  lcd.print(":");
  lcd.setCursor (6, 0);

  if (now.second() < 10) { // Add a zero, if necessary, as above
    lcd.print(0);
  }
  lcd.print(now.second(), DEC); // Display the current seconds

  if  (now.day() < 10) {
    lcd.setCursor (11, 0);
    lcd.print("0");
    lcd.setCursor (12, 0);
    lcd.print(now.day(), DEC);
  }
  else {
    lcd.setCursor (11, 0);
    lcd.print(now.day(), DEC);
  }

  lcd.setCursor (13, 0);
  lcd.print("- ");

  lcd.setCursor (14, 0);
  // lcd.print(now.month(), DEC);
  switch (now.month()) {
    case  1:
      lcd.print("JAN");
      break;
    case 2:
      lcd.print("FEB");
      break;
    case 3:
      lcd.print("MAR");
      break;
    case 4:
      lcd.print("APR");
      break;
    case 5:
      lcd.print("MAY");
      break;
    case 6:
      lcd.print("JUN");
      break;
    case 7:
      lcd.print("JUL");
      break;
    case 8:
      lcd.print("AUG");
      break;
    case 9:
      lcd.print("SEP");
      break;
    case 10:
      lcd.print("OCT");
      break;
    case 11:
      lcd.print("NOV");
      break;
    case 12:
      lcd.print("DEC");
      break;

    default:
      lcd.print("!");
      break;
  }

  lcd.setCursor (17, 0);
  lcd.print("-");

  lcd.setCursor (18, 0);
  //  lcd.print(now.year(), DEC);
  switch (now.year()) {
    case  2015:
      lcd.print("15");
      break;
    case 2016:
      lcd.print("16");
      break;
    case 2017:
      lcd.print("17");
      break;
    default:
      lcd.print("!");
      break;
  }

    ////////////////////////////// 9.30pm ALARM CODE STARTS////////////////////////////////////////
  if (now.hour() == 9)
  {
    if (now.minute() == 25)
    {
      if (now.second() == 0 ) {
        lcd.clear();
          lcd.setCursor (0, 0);
  lcd.print(" PLEASE STOP DOING  ");
  lcd.setCursor (0, 1);
  lcd.print("WHATEVER U ARE DOING");
  lcd.setCursor (0, 2);
  lcd.print("& HIBERNATE  YOUR PC");
  lcd.setCursor (0, 3);
  lcd.print("ETA 5 UNTIL BLACKOUT ");
  
        if ( fHasLooped == false )
        {

          for ( int x = 0; x < 10; x++ )
          {
            digitalWrite(24, HIGH);
            digitalWrite(26, LOW);
            delay(200);
            digitalWrite(24, LOW);
            digitalWrite(26, HIGH);
            delay(200);
          }
          fHasLooped = true;
        }
        digitalWrite(24, HIGH);
        digitalWrite(26, HIGH);
         lcd.clear();
      }
    }
  }
  //////////////////////////////9.30pm ALARM CODE ENDS//////////////////////////////////////////
      ////////////////////////////// 11.30pm ALARM CODE STARTS////////////////////////////////////////
  if (now.hour() == 11)
  {
    if (now.minute() == 25)
    {
      if (now.second() == 0 ) {
        lcd.clear();
          lcd.setCursor (0, 0);
  lcd.print(" PLEASE STOP DOING  ");
  lcd.setCursor (0, 1);
  lcd.print("WHATEVER U ARE DOING");
  lcd.setCursor (0, 2);
  lcd.print("& HIBERNATE  YOUR PC");
  lcd.setCursor (0, 3);
  lcd.print("ETA 5 UNTIL BLACKOUT ");
  
        if ( fHasLooped == false )
        {

          for ( int x = 0; x < 10; x++ )
          {
            digitalWrite(24, HIGH);
            digitalWrite(26, LOW);
            delay(200);
            digitalWrite(24, LOW);
            digitalWrite(26, HIGH);
            delay(200);
          }
          fHasLooped = true;
        }
        digitalWrite(24, HIGH);
        digitalWrite(26, HIGH);
         lcd.clear();
      }
    }
  }
  //////////////////////////////11.30pm ALARM CODE ENDS//////////////////////////////////////////
      ////////////////////////////// 2.30pm ALARM CODE STARTS////////////////////////////////////////
  if (now.hour() == 14)
  {
    if (now.minute() == 25)
    {
      if (now.second() == 0 ) {
        lcd.clear();
          lcd.setCursor (0, 0);
  lcd.print(" PLEASE STOP DOING  ");
  lcd.setCursor (0, 1);
  lcd.print("WHATEVER U ARE DOING");
  lcd.setCursor (0, 2);
  lcd.print("& HIBERNATE  YOUR PC");
  lcd.setCursor (0, 3);
  lcd.print("ETA 5 UNTIL BLACKOUT ");
  
        if ( fHasLooped == false )
        {

          for ( int x = 0; x < 10; x++ )
          {
            digitalWrite(24, HIGH);
            digitalWrite(26, LOW);
            delay(200);
            digitalWrite(24, LOW);
            digitalWrite(26, HIGH);
            delay(200);
          }
          fHasLooped = true;
        }
        digitalWrite(24, HIGH);
        digitalWrite(26, HIGH);
         lcd.clear();
      }
    }
  }
  //////////////////////////////2.30pm ALARM CODE ENDS//////////////////////////////////////////
  ////////////////////////////// 5.30pm ALARM CODE STARTS////////////////////////////////////////
  if (now.hour() == 17)
  {
    if (now.minute() == 25)
    {
      if (now.second() == 0 ) {
                lcd.clear();
          lcd.setCursor (0, 0);
  lcd.print(" PLEASE STOP DOING  ");
  lcd.setCursor (0, 1);
  lcd.print("WHATEVER U ARE DOING");
  lcd.setCursor (0, 2);
  lcd.print("& HIBERNATE  YOUR PC");
  lcd.setCursor (0, 3);
  lcd.print("ETA 5 UNTIL BLACKOUT ");
  
        if ( fHasLooped == false )
        {

          for ( int x = 0; x < 10; x++ )
          {
            digitalWrite(24, HIGH);
            digitalWrite(26, LOW);
            delay(200);
            digitalWrite(24, LOW);
            digitalWrite(26, HIGH);
            delay(200);
          }
          fHasLooped = true;
        }
        digitalWrite(24, HIGH);
        digitalWrite(26, HIGH);
         lcd.clear();
      }
    }
  }
  //////////////////////////////5.30pm ALARM CODE ENDS//////////////////////////////////////////
    ////////////////////////////// 10.30pm ALARM CODE STARTS////////////////////////////////////////
  if (now.hour() == 22)
  {
    if (now.minute() == 25)
    {
      if (now.second() == 0 ) {
                lcd.clear();
          lcd.setCursor (0, 0);
  lcd.print(" PLEASE STOP DOING  ");
  lcd.setCursor (0, 1);
  lcd.print("WHATEVER U ARE DOING");
  lcd.setCursor (0, 2);
  lcd.print("& HIBERNATE  YOUR PC");
  lcd.setCursor (0, 3);
  lcd.print("ETA 5 UNTIL BLACKOUT ");
  
        if ( fHasLooped == false )
        {

          for ( int x = 0; x < 10; x++ )
          {
            digitalWrite(24, HIGH);
            digitalWrite(26, LOW);
            delay(200);
            digitalWrite(24, LOW);
            digitalWrite(26, HIGH);
            delay(200);
          }
          fHasLooped = true;
        }
        digitalWrite(24, HIGH);
        digitalWrite(26, HIGH);
         lcd.clear();
      }
    }
  }
  //////////////////////////////10.30pm ALARM CODE ENDS//////////////////////////////////////////
//    //////////////////////////////TEST  ALARM CODE STARTS////////////////////////////////////////
//  if (now.hour() == 17)
//  {
//    if (now.minute() == 00)
//    {
//      if (now.second() == 0 ) {
//                lcd.clear();
//          lcd.setCursor (0, 0);
//  lcd.print(" PLEASE STOP DOING  ");
//  lcd.setCursor (0, 1);
//  lcd.print("WHATEVER U ARE DOING");
//  lcd.setCursor (0, 2);
//  lcd.print("& HIBERNATE  YOUR PC");
//  lcd.setCursor (0, 3);
//  lcd.print("ETA 5 UNTIL BLACKOUT ");
//  
//        if ( fHasLooped == false )
//        {
//
//          for ( int x = 0; x < 10; x++ )
//          {
//            digitalWrite(24, HIGH);
//            digitalWrite(26, LOW);
//            delay(200);
//            digitalWrite(24, LOW);
//            digitalWrite(26, HIGH);
//            delay(200);
//          }
//          fHasLooped = true;
//        }
//        digitalWrite(24, HIGH);
//        digitalWrite(26, HIGH);
//         lcd.clear();
//      }
//    }
//  }
//  //////////////////////////////TEST ALARM CODE ENDS//////////////////////////////////////////
}


void amps()
{
  unsigned long currentAcc = 0;
  unsigned int count = 0;
  unsigned long prevMicros = micros() - sampleInterval ;
  while (count < numSamples)
  {
    if (micros() - prevMicros >= sampleInterval)
    {
      int adc_raw = analogRead(currentPin) - adc_zero;
      currentAcc += (unsigned long)(adc_raw * adc_raw);
      ++count;
      prevMicros += sampleInterval;
    }
  }

  float rms = sqrt((float)currentAcc / (float)numSamples) * (75.7576 / 1024.0);
  //Serial.println(rms);
  Blynk.virtualWrite(V5, rms);
  lcd.setCursor (0, 3);
  lcd.print("AMPs:");
  lcd.setCursor (5, 3);
  lcd.print(rms);
}

int currenttime ;
int lasttime ;

BLYNK_WRITE(V15) {  //////remote arduino uptime
 g = param.asInt();
  Blynk.virtualWrite(V15, g);
  lcd.setCursor (11, 3);
  lcd.print("RUpT:");
  lcd.setCursor (16, 3);
  
  if ((g / 60) < 10) {
    lcd.print((g / 60));
    lcd.setCursor (17, 3);
    lcd.print("  ");
  }
  else
  {
    lcd.print((g / 60));
  }
}


BLYNK_WRITE(V18) {  //// remote arduino temp  or from door lock arduino
  h = param.asInt();
  // h = tempOutDeg ; /// uncomment to manual over ride the values with IN DOOR TEMP !
  if (h > 100) {
    Blynk.virtualWrite(V18, "Err");
  }
  else {
    Blynk.virtualWrite(V18, h);
  }
}


BLYNK_WRITE(V19)
{
  int i = param.asInt();
  if (i == 1) {
    bridge1.virtualWrite(V6 , 0);
    ledD.on();
  }
  else {
    bridge1.virtualWrite(V6 , 1);
    ledD.off();

  }
}

void emptyroutine()
{
}

