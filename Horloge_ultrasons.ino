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

#include <IRremote.h>
//int RECV_PIN = 5; //define input pin on Arduino


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
#define BRIGHTNESS_PIN  6   // Must be a PWM pin
#define LDR A7  // composante photorésistance sur la pin A7

int h,m,s,jr,mo,an,mes,bright,wait=300,mode=0;
unsigned long touch;
String com;

void setup (){     
Rtc.Begin();
RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
RtcDateTime now = Rtc.GetDateTime();
    
Serial.begin(57600);
//irrecv.enableIRIn(); // Start the receiver
IrReceiver.begin(5, ENABLE_LED_FEEDBACK);

lcd.init(); // initialisation de l’afficheur
big.begin();
lcd.backlight();
Retroeclairage();
}

void loop (){
// reception infrarouge        
if (IrReceiver.decode())  {
  touch=IrReceiver.decodedIRData.decodedRawData;
  if (touch==3125149440 || touch==3108437760 ||touch==3091726080 || touch==3141861120 ||touch==3208707840 || touch==3158572800 ||touch==4161273600 ||
  touch==3927310080 ||touch==4127850240   || touch==3910598400  ||touch==3860463360 || touch==4061003520 ||touch==4077715200 || touch==3877175040 ||
  touch==2707357440 || touch==4144561920 ||touch==3810328320 || touch==2774204160 ||touch==3175284480 || touch==2907897600 ||touch==3041591040 ) {
    telecir(); 
    mode=1;
   //  Serial.println (touch);
  }
   
} 

if (mode==1) reglageheure();

if (mode==0) affichheure();
  delay (1000); 
}

void Retroeclairage(){
//réglage de l'intensité lumineus du LCD selon la lumière ambiante
bright=255-(analogRead(LDR)/4);if (bright<0) bright=0;
//Serial.print(analogRead(LDR));Serial.print(" ");Serial.println(bright);
analogWrite(BRIGHTNESS_PIN, bright);
}

void telecir()
{
   //Serial.println(touch);
  if (touch==3125149440) com="ch-";
  if (touch==3108437760) com="ch"; 
  if (touch==3091726080) com="ch+"; 
  if (touch==3141861120) com="tr-";  
  if (touch==3208707840) com="tr+";
  if (touch==3158572800) com="pl"; 
  if (touch==4161273600) com="v-"; 
  if (touch==3927310080) com="v+";
  if (touch==4127850240) com="eq";
  if (touch==3910598400) com="0"; 
  if (touch==3860463360) com="+100"; 
  if (touch==4061003520) com="+200";  
  if (touch==4077715200) com="1";
  if (touch==3877175040) com="2"; 
  if (touch==2707357440) com="3"; 
  if (touch==4144561920) com="4";
  if (touch==3810328320) com="5";
  if (touch==2774204160) com="6"; 
  if (touch==3175284480) com="7"; 
  if (touch==2907897600) com="8";  
  if (touch==3041591040) com="9";
  Serial.println (com);
  IrReceiver.resume(); // Receive the next value
}

void affichheure(){
  // Requete heure 
    RtcDateTime now = Rtc.GetDateTime();    
    h=now.Hour(), DEC;m=now.Minute(), DEC;s=now.Second(), DEC;jr=now.Day(), DEC;mo=now.Month(), DEC;an=now.Year(), DEC;
    mes=(int)distanceSensor.measureDistanceCm()+1;
       
// Affichage heure minutes
    big.writeint(0,0,h,2,true); 
    big.writeint(0,6,m,2,true); 
//Affichage secondes
    lcd.setCursor(12,0);
    if (s<10) lcd.print(" "); 
    lcd.print(s);
//Affiche jour mois
    lcd.setCursor(12,1);
    if (jr<10) lcd.print(" "); 
    lcd.print(jr);
    lcd.setCursor(14,1);
    if (mo<10) lcd.print(" "); 
    lcd.print(mo);


//Affichage lorsque les ultrasons détectent une présence <50cm
if (mes<50 and mes!=0) {
    Retroeclairage();
    wait=300;
    }

wait--;
if (wait<0) {
    wait=0;
    analogWrite(BRIGHTNESS_PIN, 0);
    }
}

void reglageheure(){
lcd.init();
delay(1000);
  
  mode=0;

}
