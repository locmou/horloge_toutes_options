#include <ThreeWire.h>
// CONNECTIONS pendule
// DS1302 CLK/SCLK --> 4
// DS1302 DAT/IO --> 3
// DS1302 RST/CE --> 2
// DS1302 VCC --> 3.3v - 5v
// DS1302 GND --> GND
#include <RtcDS1302.h>
#include "RTClib.h"
RTC_Millis rtc;
// definition des broches de la pendule
ThreeWire myWire(3,4,2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

// Detecteur ultrasons
#include <HCSR04.h>
// definition des broches du capteur ultrasons
const int trigPin = 8;
const int echoPin = 7;
// initialisation du capteur avec les broches utilisees.
UltraSonicDistanceSensor distanceSensor(trigPin, echoPin);

// Gros chiffres
#include <BigFont02_I2C.h>

// Affichage
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
BigFont02_I2C     big(&lcd); // construct large font object, passing to it the name of our lcd object

//Définition des contrastes
#define BRIGHTNESS_PIN      6   // Must be a PWM pin
#define LDR A7  // composante photorésistance sur la pin A7

int h,m,s,mes,bright,wait=300;

void setup () 
{     
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    RtcDateTime now = Rtc.GetDateTime();
    
    //Serial.begin(57600);
     // Mise à l'heure
     /*
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }*/

    lcd.init(); // initialisation de l’afficheur
    big.begin();
    lcd.backlight();
    //réglage de l'intensité lumineus du LCD selon la lumière ambiante
    bright=255-(analogRead(LDR)/4);if (bright<0) bright=0;
    //Serial.print(analogRead(LDR));Serial.print(" ");Serial.println(bright);
    analogWrite(BRIGHTNESS_PIN, bright);
}

void loop ()
{
    RtcDateTime now = Rtc.GetDateTime();    
    h=now.Hour(), DEC;m=now.Minute(), DEC;s=now.Second(), DEC;mes=(int)distanceSensor.measureDistanceCm()+1;
    delay (1000);
    wait--;
    if (wait<0) {
      wait=0;
      analogWrite(BRIGHTNESS_PIN, 0);
      }
         
// Affichage heure minutes
    big.writeint(0,0,h,2,true); 
    big.writeint(0,6,m,2,true); 
//Affichage secondes
    lcd.setCursor(12,0);
    if (s<10) lcd.print(" "); 
    lcd.print(s);
//Affiche jour mois
    lcd.setCursor(12,1);
    if (now.Day()<10) lcd.print(" "); 
    lcd.print(now.Day(), DEC);
    lcd.setCursor(14,1);
    if (now.Month()<10) lcd.print(" "); 
    lcd.print(now.Month(), DEC);

//Affichage lorsque les ultrasons détectent une présence <50cm
  if (mes<50 and mes!=0) {
    wait=300;
    //réglage de l'intensité lumineus du LCD selon la lumière ambiante
    bright=255-(analogRead(LDR)/4);
    if (bright<=0) bright=5;
    analogWrite(BRIGHTNESS_PIN, bright);
    }
}
