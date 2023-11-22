// CONNECTIONS: LCD Et RTC
// DS1307 SDA --> SDA
// DS1307 SCL --> SCL
// DS1307 VCC --> 5v
// DS1307 GND --> GND
#include <Wire.h> 
#include <RtcDS1307.h>
RtcDS1307<TwoWire> Rtc(Wire);

// Gestion de la led
#include <RGB_LED.h>
RGB_LED LED(3,9,11);

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
const uint8_t alPin[] = {2,4};

// Affichage LCD
#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27,16,2);

// Gros chiffres
#include <BigFont02_I2C.h>
BigFont02_I2C     big(&lcd); // construct large font object, passing to it the name of our lcd object

//Définition des contrastes
#define BRIGHTNESS_PIN  6   // Must be a PWM pin
#define LDR A7  // composante photorésistance sur la pin A7

//int nbr,h,m,s,jr,mo,an,mes,bright,wait=300,mode=0;
unsigned long touch;
String com,aff="--";
/*
byte al1[8] = {  B00001,  B00001,  B00001,  B00000,  B00000,  B00000,  B00000,  B00000 } ;
byte al2[8] = {  B01001,  B01001,  B01001,  B00000,  B00000,  B00000,  B00000,  B00000 } ;
byte al12[8] = {  B00001,  B00001,  B00001,  B00000,  B01001,  B01001,  B01001,  B00000 } ;*/
uint8_t r,g,b,x,a,h8,m8,nbr,h,m,s,jr,mo,an,mes,bright,mode=0,bout[2]={10,12};
int t=0,wait=300,but[2];
float maxi;
bool al[2]={false,false},antial[2]={false,false},pop=false;

// Déclarations de fonctions
void infoalarm(uint8_t x);
void reglagealarme(uint8_t x);
void affichheure();
void touchir();
void Retroeclairage();
void telecir();
void iwait();
void ecrannet();
void settime(float maxi);


void setup (){
Rtc.Begin();
/*// Pour remettre à l'heure lorsque le port série est relié à l'ordi
RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
RtcDateTime now = Rtc.GetDateTime();
Rtc.SetDateTime(compiled);*/
// never assume the Rtc was last configured by you, so
// just clear them to your needed state
Rtc.SetSquareWavePin(DS1307SquareWaveOut_Low);
// Infrarouges  
IrReceiver.begin(5, ENABLE_LED_FEEDBACK);
//LCD
lcd.init(); // initialisation de l’afficheur
big.begin();
lcd.backlight();
Retroeclairage();
// Pin's
pinMode(2, OUTPUT);
pinMode(4, OUTPUT);
pinMode(10,INPUT);
pinMode(12,INPUT);
Serial.begin(115200);
}


void loop (){
  
// Pression sur le bouton 10 al1 ou 12 al2
for (x=0;x<2;x++){
  if (digitalRead(bout[x]) == LOW) {
    if (pop==true) {
      pop=false;
      but[x]=0;
      if (but[x]>5) {
        //Appui long= réglage alarm
        al[x]=!al[x];
        antial[x]=false;
        Serial.print("alarme "+String(x)+" :");Serial.println(al[x]);
        } 
      else {
        // Appui court = on/off       
        but[x]=0;
        antial[x]=!antial[x];
        }   
      }
    }
  else {
    pop=true;
    but[x]++;Serial.print("Alarme : ");Serial.print(x);Serial.print(" Bouton :");Serial.print(bout[x]);Serial.print("comptage bouton :");Serial.println(but[x]);
    }
}
 
// Alarme qui se déclenche durant les 20' qui suivent l'heure
for (x=1;x<3;x++){
if (al[x-1]=true && 60*h+m>=60*(Rtc.GetMemory(x*2))+(Rtc.GetMemory(1+(x*2))) && 60*h+m<=60*(Rtc.GetMemory(x*2))+(Rtc.GetMemory(1+(x*2)))+20) {
  if ( 60*h+m==60*(Rtc.GetMemory(x*2))+(Rtc.GetMemory(1+(x*2)))+20) antial[x-1]=false;
  if (antial[x-1]==false) digitalWrite (x*2,LOW);  else  digitalWrite (x*2,HIGH);
  } 
else {
  if (antial[x-1]==false) digitalWrite (x*2,HIGH); else digitalWrite (x*2,LOW);
  }
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
//déclenché par EQ
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

// Affiche les heures des alarmes
void infoalarm(uint8_t(x)) {
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Alarme "+String(x)+"-> ");
lcd.print(String(Rtc.GetMemory(2*x))+ ":"+String(Rtc.GetMemory(1+(2*x))));
lcd.setCursor(0,1);
lcd.print("EQ pour modif");
iwait();
}

//Réglage des alarmes 1 et 2
void reglagealarme(uint8_t(x)){
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Reglage alarme "+String(x));

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
    Rtc.SetMemory(2*x,h8);Rtc.SetMemory(1+(2*x),m8);
    mode=0;
    nbr=0;
    ecrannet();
    Serial.print ("heure: "+String(Rtc.GetMemory(2*x))+ " minutes :"+String(Rtc.GetMemory(1+(2*x))));delay(1000);
  }
}
iwait();
}


//Réglage de l'heure et de la date
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
        aff="----"
        settime(10000);
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

// Permet la saisie de la date et des heures et alarmes
void settime(float(maxi)){
nbr=0;lcd.setCursor(0,1);
if (maxi==24) lcd.print("Heure :");
if (maxi==60) lcd.print("Minutes :");
if (maxi==32) lcd.print("Jour :");
if (maxi==13) lcd.print("Mois :");
if (maxi==10000) lcd.print("Annee :");

// S'execute lorsqu'un chiffre est saisi sur la commande infrarouge
if (touch==3910598400 ||touch==4077715200  ||touch==3877175040 ||touch==2707357440  ||touch==4144561920  ||touch==3810328320  ||touch==2774204160  ||touch==3175284480 ||touch==2907897600  ||touch==3041591040   ) {
  telecir();
  if (maxi!=10000){
    if (aff=="--") {
      if (com.toInt()<=int((maxi-1)/10)) {
        
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
      
    } else {
    aff=aff+com;lcd.setCursor(9,1);lcd.print(aff);if (aff.toInt()>1000) nbr=aff.toInt();
     
  }
  wait=800;
}

// En mode 0, affiche l'heure la date et les alarmes en marche
void affichheure(){
// Requete heure 
RtcDateTime now = Rtc.GetDateTime();    
h=now.Hour(), DEC;m=now.Minute(), DEC;s=now.Second(), DEC;jr=now.Day(), DEC;mo=now.Month(), DEC;an=now.Year(), DEC;
mes=(int)distanceSensor.measureDistanceCm()+1;
  
// Affichage heure minutes  
big.begin();
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


// Affichage alarme 1/2 on/off
lcd.setCursor (14,0);
if (al[0] == true && al[1] == true) {
  lcd.write(165);lcd.write(58);
} else if (al[0] == true) {
  lcd.print(" ");
  lcd.write(165);
} else if (al[1] == true) {
   lcd.print(" ");
   lcd.write(58);
} else {
  lcd.print("  ");
}  

//LED
t=t+1;
if (t>400) t=0;
if (t<50)  { r=255;  g=255-(t*5.1);  b=(t*5.1);}
if (t>=50 and t<100) { r=255-(t-50)*5.1;  g=(t-50)*5.1;  b=255;}
if (t>=100 and t<150) { r=0;  g=255-(t-100)*5.1;  b=255;}
if (t>=150 and t<200) { r=0;  g=(t-150)*5.1;  b=255-(t-150)*5.1;}
if (t>=200 and t<250) { r=(t-200)*5.1;  g=255-(t-200)*5.1;  b=0;}
if (t>=250 and t<300) { r=255-(t-250)*5.1;  g=(t-250)*5.1;  b=0;}
if (t>=300 and t<350) { r=0;  g=255;  b=(t-300)*5.1;}
if (t>=350 ) { r=(t-350)*5.1;  g=255;  b=255-(t-350)*5.1;}
LED.set(r*bright/255,g*bright/255,b*bright/255); 

//Affichage lorsque les ultrasons détectent une présence <50cm
if (mes<50 and mes!=0) {
    Retroeclairage();
    wait=800;
}
  
//Coupe la le rétroéclairage en cas d'inactivité prolongée
wait--;
if (wait<0)  analogWrite(BRIGHTNESS_PIN, 0);
LED.set(0,0,0);
}

// Renseigne dans la variable touch le code infrarouge détecté lorsque c'est le cas
void touchir(){
if (IrReceiver.decode())  {
  touch=IrReceiver.decodedIRData.decodedRawData;
  }
IrReceiver.resume();// Receive the next value
//delay (800); 
}

// Ajuste le rétroéclairage en fonction de la mesure de luminosité ambiante
void Retroeclairage(){
//réglage de l'intensité lumineus du LCD selon la lumière ambiante
bright=255-(analogRead(LDR)/4);if (bright<0) bright=0;
analogWrite(BRIGHTNESS_PIN, bright);
}

// Assigne à la variable com la chaine correspondant au code infrarouge détécté.
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

//Renvoie vers l'affichage de l'heure après un temps d'inactivité dans les réglages d'heure ou d'alarme
void iwait(){
wait--;
if (wait<0) {
    wait=800;
    mode=0;aff="--";
    Retroeclairage();ecrannet();
    }
}

//Nettoyage de l'écran LCD
void ecrannet(){
lcd.init();
big.begin();
}
