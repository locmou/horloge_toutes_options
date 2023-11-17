
// CONNECTIONS: LCD Et RTC
// DS1307 SDA --> SDA
// DS1307 SCL --> SCL
// DS1307 VCC --> 5v
// DS1307 GND --> GND
#include <Wire.h> 
#include <RtcDS1307.h>
RtcDS1307<TwoWire> Rtc(Wire);

// Gestion IR
#include <IRremote.h>

// Detecteur ultrasons
#include <HCSR04.h>
// definition des broches du capteur ultrasons
const int trigPin = 8;
const int echoPin = 7;
// initialisation du capteur avec les broches utilisees.
UltraSonicDistanceSensor distanceSensor(trigPin, echoPin);

// Connexion alarme et leds
const uint8_t alPin[] = {2, 4};
//const uint8_t alPin[2]=4;
const uint8_t redLedPin=3;
const uint8_t blueLedPin=5;
const uint8_t GreenLedPin=9;

// Gros chiffres
#include <BigFont02_I2C.h>

// Affichage LCD
#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27,16,2);
BigFont02_I2C     big(&lcd); // construct large font object, passing to it the name of our lcd object

//Définition des contrastes
#define BRIGHTNESS_PIN  6   // Must be a PWM pin
#define LDR A7  // composante photorésistance sur la pin A7

//int nbr,h,m,s,jr,mo,an,mes,bright,wait=300,mode=0;
unsigned long touch;
String com,aff="--";
byte al1[] = {  B00001,  B00001,  B00001,  B00000,  B00000,  B00000,  B00000,  B00000 } ;
byte al2[] = {  B01001,  B01001,  B01001,  B00000,  B00000,  B00000,  B00000,  B00000 } ;
byte al12[] = {  B00001,  B00001,  B00001,  B00000,  B01001,  B01001,  B01001,  B00000 } ;
uint8_t x,a,h8,m8,nbr,h,m,s,jr,mo,an,mes,bright,mode=0;
int wait=300;
float maxi;

void setup (){
  
Rtc.Begin();
/*
if (Serial) {
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Rtc.SetDateTime(compiled);}*/
    
    
/*RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
RtcDateTime now = Rtc.GetDateTime();*/
// never assume the Rtc was last configured by you, so
// just clear them to your needed state
Rtc.SetSquareWavePin(DS1307SquareWaveOut_Low);

//Rtc.SetDateTime(compiled);

// This line sets the RTC with an explicit date & time, for example to set
// January 21, 2014 at 3am you would call:
//Rtc.SetDateTime(RtcDateTime(2014, 1, 21, 3, 0, 0));

IrReceiver.begin(5, ENABLE_LED_FEEDBACK);

lcd.init(); // initialisation de l’afficheur
big.begin();
lcd.backlight();
Retroeclairage();

Serial.begin(115200);
}

void loop (){
 // Test alarme
x=1
while (x<2) {
If (60*h+m>=60*(Rtc.GetMemory(x*2))+(Rtc.GetMemory(1+(x*2))) && 60*h+m<60*(Rtc.GetMemory(x*2))+(Rtc.GetMemory(1+(x*2)))+20) {
  digitalWrite (alPin[x],HIGH);
  } 
else {
  digitalWrite (alPin[x],LOW);
  }
x++
}

// reception infrarouge ?        
touchir();
// déclenché par CH+ ou CH-
if (touch==3125149440  ||touch==3091726080  ) {
  wait=800;
  mode=1;ecrannet();an=mo=jr=h=m=s=0;
  }

// délenché par 100+, 200+
if (touch==3860463360  ||touch==4061003520  ) {
  telecir(); if (com=="+100") a=1; else a=2;
  wait=800;
  mode=2;ecrannet();
  }

//déclenché par eq
if (touch==4127850240) {
  telecir();
  wait=800;
  mode=3;ecrannet();h8=m8=0;
  }

if (mode==0) affichheure();
if (mode==1) reglageheuredate();
if (mode==2) infoalarm(a);
if (mode==3) reglagealarme(a);

}

void infoalarm(a) {
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Alarme "+String(a)+"->");
lcd.print(String(Rtc.GetMemory(2*a))+ ":"+String(Rtc.GetMemory(1+(2*a)))+ "mn");
lcd.setCursor(0,1);
lcd.print("eq pour la modifier.")
iwait();
}

void reglagealarme(a){
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Reglage alarme "+String(a));

if (h8==0) {
settime(24);
h8=nbr;
}
else{
  settime(60);
  m8=nbr;
  if (m8!=0) {
    aff="--";      
    wait=300;
    Rtc.SetMemory(a,h8);Rtc.SetMemory(1+(2*a),m8);
    mode=0;
    nbr=0;
    ecrannet();
    Serial.print ("heure: "+String(Rtc.GetMemory(2*a))+ " minutes :"+String(Rtc.GetMemory(1+(2*a))));delay(1000);
  }
}
 
iwait();
}

void reglageheuredate(){
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Reglage pendule");

if (h==0) {
settime(24);
h=nbr;
}
else{ 
  if (m==0){
    settime(60);
    m=nbr;}
  else {
    if (jr==0) {
      settime(32);
      jr=nbr;}
    else {
      if (mo==0){
        settime(13);
        mo=nbr;}
      else {
        settime(100);
        an=nbr;
        if (an!=0){aff="--";
        ecrannet();
        wait=300;
        Rtc.SetDateTime(RtcDateTime(2000+an, mo, jr, h, m, 0));
        mode=0;
        nbr=0;}
        }     
    }
  }
}
iwait();
}

void settime(float(maxi)){
nbr=0;lcd.setCursor(0,1);
if (maxi==24) lcd.print("Heure :");
if (maxi==60) lcd.print("Minutes :");
if (maxi==32) lcd.print("Jour :");
if (maxi==13) lcd.print("Mois :");
if (maxi==100) lcd.print("Annee :");

// S'execute lorsqu'un chiffre est saisi sur la commande infrarouge
if (touch==3910598400 ||touch==4077715200  ||touch==3877175040 ||touch==2707357440  ||touch==4144561920  ||touch==3810328320  ||touch==2774204160  ||touch==3175284480 ||touch==2907897600  ||touch==3041591040   ) {
  telecir();
  if (aff=="--") {
    if (com.toInt()<=int((maxi-1)/10)) {
    //if (com.toInt()!=0 && com.toInt()<=int((maxi-1)/10)) {
    aff=com+"-";lcd.setCursor(9,1);lcd.print(aff);
    }
    else {  
    nbr=com.toInt();lcd.setCursor(9,1);lcd.print("0"+String(nbr));delay(300);aff="--";lcd.setCursor(0,1);lcd.print("                            ");
    } 
  }
  else{
    aff=String(aff.charAt(0))+com;nbr=aff.toInt();lcd.setCursor(9,1);lcd.print(nbr);delay(300);aff="--";lcd.setCursor(0,1);lcd.print("                            ");
  }
if (nbr>=int(maxi)) {aff="--";com="";nbr=0;}
wait=800;  
}

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
    wait=800;
    }
    
wait--;
if (wait<0)  analogWrite(BRIGHTNESS_PIN, 0);
    
}

void touchir(){
touch=0;
if (IrReceiver.decode())  {
  touch=IrReceiver.decodedIRData.decodedRawData;
  }
IrReceiver.resume();// Receive the next value
//delay (800); 
}

void Retroeclairage(){
//réglage de l'intensité lumineus du LCD selon la lumière ambiante
bright=255-(analogRead(LDR)/4);if (bright<0) bright=0;
//Serial.print(analogRead(LDR));Serial.print(" ");Serial.println(bright);
analogWrite(BRIGHTNESS_PIN, bright);
}

void telecir(){
com="";
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
touch=0;
}

void iwait(){
wait--;
if (wait<0) {
    wait=800;
    mode=0;aff="--";
    Retroeclairage();ecrannet();
    }
}

void ecrannet(){
lcd.init();
big.begin();
}