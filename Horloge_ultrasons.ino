/*

nterruptions matérielles : Les microcontrôleurs Arduino ont des broches spécifiques qui peuvent
 être configurées pour déclencher une interruption matérielle lorsqu'un certain événement se produit. 
 Par exemple, une interruption peut être déclenchée lorsqu'une broche change d'état (LOW à HIGH ou HIGH à LOW).

Interruptions logicielles : En plus des interruptions matérielles, vous pouvez également utiliser des interruptions
logicielles. Celles-ci sont générées par des événements logiciels spécifiques, comme une minuterie logicielle atteignant 
une certaine valeur.

Prévoir la gestion de la led lorsque l'alarme sonne
Prévoir la gestion différente de l'alarme si semaine ou du lundi au vendredi

*/


#include <Wire.h> 
#include <RtcDS1307.h>
RtcDS1307<TwoWire> Rtc(Wire);
const int DS1307_SDA_PIN = A4;
const int DS1307_SCL_PIN = A5;

// Gestion des leds sur pin pwm
#include <RGB_LED.h>
RGB_LED LED1(9,10,11);
const int BLUELEDRGB=5;
const int GREENLEDRGB=6;

// Gestion IR
#include <IRremote.h>
const int IR_PIN = 13;

// Detecteur ultrasons
#include <HCSR04.h>
// definition des broches du capteur ultrasons
const int TRIGPIN = 8;
const int ECHOPIN = 7;
// initialisation du capteur avec les broches utilisees.
UltraSonicDistanceSensor distanceSensor(TRIGPIN, ECHOPIN);

// Connexion alarme et leds
const uint8_t ALPIN[] = {2,4};

// Boutons 
const uint8_t BUTT[]={10,12};

// Affichage LCD
#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27,16,2);

// Gros chiffres
#include <BigFont02_I2C.h>
BigFont02_I2C  big(&lcd); // construct large font object, passing to it the name of our lcd object

//Définition des contrastes
const uint8_t BRIGHTNESS_PIN=3;   // Must be a PWM pin
const uint8_t LDR=A7;  // composante photorésistance sur la pin A7

unsigned long touch;
char aff[5],com[5];
uint8_t r,g,b,x,a,nbr,h,m,mode,s,jr,mo,an,mes,bright,alh[2],alm[2];
int t=0,wait=300,but[2];
int maxi;
//float maxi;
bool al[2]={false,false},antial[2]={false,false},we[2],pop[2]={false,false};

// Déclaration des constantes pour les modes
enum Mode {
  MODE_Heure,
  MODE_Reglage_h,
  MODE_ALARM_INFO,
  MODE_Reglage_al,
  MODE_memlewe
};

// Déclarations de fonctions
void infoalarm(uint8_t x);
void reglagealarme(uint8_t x);
void affichheure();
void touchir();
void Retroeclairage();
void telecir();
void iwait();
void ecrannet();
void settime(int maxi);
void afficheinput();
void memlewe(uint8_t x);
void affectnbr(int maxi);
void afficheinput();
void effaceinput();
void Turncolor();
void Ledalarm();


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
IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);
  
//LCD
lcd.init(); // initialisation de l’afficheur
big.begin();
lcd.backlight();
Retroeclairage();
  
// Pin's
pinMode(BRIGHTNESS_PIN, OUTPUT);
pinMode(GREENLEDRGB, OUTPUT);
pinMode(BLUELEDRGB, OUTPUT);
Serial.begin(115200);
Mode mode = MODE_Heure;
  
for (x=0;x<2;x++){
  alh[x]=Rtc.GetMemory((x+1)*2);alm[x]=Rtc.GetMemory(1+((x+1)*2));
  al[x]=Rtc.GetMemory(5+x);
  we[x-1]=Rtc.GetMemory(7+x);
  pinMode(ALPIN[x], OUTPUT);
  pinMode(BUTT[x],INPUT);
  }
}

void loop (){
// Pression sur le bouton 10 al1 ou 12 al2
for (x=0;x<2;x++){
  if (digitalRead(BUTT[x]) == LOW) {
    if (pop[x]==true) {
      pop[x]=false;
      if (but[x]>2) {
        //Appui long= réglage alarm
        al[x]= !al[x];Rtc.SetMemory(5+x,al[x]);
        antial[x]=false;
        }
      else {
        // Appui court = on/off
        antial[x]=!antial[x];
        }
       but[x]=0;
      }
    }
  else {
    pop[x]=true;
    but[x]++;
    }
  }
 
// Alarme qui se déclenche durant les 20' qui suivent l'heure
for (x=1;x<3;x++){
  if (al[x-1]==true && 60*h+m>=(60*alh[x-1])+alm[x-1] && 60*h+m<=(60*alh[x-1])+alm[x-1]+15) {
    if (60*h+m==60*(alh[x-1])+(alm[x-1])+15) antial[x-1]=false;
    if (antial[x-1]==false) digitalWrite (x*2,LOW);  else  digitalWrite (x*2,HIGH);
    } 
  else {
    if (antial[x-1]==false) digitalWrite (x*2,HIGH); else digitalWrite (x*2,LOW);
    }
  }
 
// reception infrarouge ?
touchir();

//
// Provisoire pour déclencher l'alarme
//
if (touch==3927310080) Ledalarm();


// déclenché par CH+ ou CH-
if (touch==3125149440  ||touch==3091726080  ) {
  wait=800;
  mode=MODE_Reglage_h ;ecrannet();aff[0]=0;an=mo=jr=h=m=0;
  }
// délenché par 100+, 200+
if (touch==3860463360  ||touch==4061003520  ) {
  telecir(); if (strcmp(com,"+100")==0) a=1; else a=2;
  wait=800;
  mode=MODE_ALARM_INFO;ecrannet();
  }
//déclenché par EQ
if (touch==4127850240) {
  if (mode==MODE_ALARM_INFO){
    telecir();
    wait=800;
    mode=MODE_Reglage_al;ecrannet();alh[a-1]=alm[a-1]=0;
    }
  }

// Ajustement leds
LED1.set(255-r,255-g,255-b);
digitalWrite(BLUELEDRGB,255);
digitalWrite(GREENLEDRGB,255);

//Depart vers les sous programmes
if (mode==MODE_Heure) affichheure();
else if (mode==MODE_Reglage_h ) reglageheuredate();
else if (mode==MODE_ALARM_INFO) infoalarm(a);
else if (mode==MODE_Reglage_al) {reglagealarme(a);}
else memlewe(a);
}

// Affiche les heures des alarmes
void infoalarm(uint8_t(x)) {
Turncolor();
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Ala. "+String(x)+"->");
lcd.print(String(alh[x-1])+ ":" +String(alm[x-1]));
if (we[x-1]==1) lcd.print("we+"); else lcd.print("we-");
lcd.setCursor(0,1);
lcd.print("EQ pour modif");
iwait();
}

//Réglage des alarmes 1 et 2x
void reglagealarme(uint8_t(x)){
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Reglage alarme "+String(x));

if (alh[x-1]==0) {settime(24);alh[x-1]=nbr;}
else{  settime(60);  alm[x-1]=nbr;Serial.println ("alm : "+String(alm[x-1]));
  if (alm[x-1]!=0) {   wait=100;Rtc.SetMemory(2*x,alh[x-1]);Rtc.SetMemory(1+(2*x),alm[x-1]); mode=MODE_memlewe   ;    ecrannet();}
  //Serial.print ("heure: "+String(Rtc.GetMemory(2*x))+ " minutes :"+String(Rtc.GetMemory(1+(2*x))));delay(1000);
  }
iwait();
}

//Input demande si l'alarme fonctionne le we
void memlewe(uint8_t(x)){
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Alarme "+String(x)+"le we?");
lcd.setCursor(0,1);
lcd.print("oui=tr+, non=tr-");

//tr-
if (touch==3141861120) {
  we[x-1]=true;
  Rtc.SetMemory(7+x,true);
  mode=MODE_Heure;  
  }

//tr+  
if (touch==3208707840) {  
  we[x-1]=false;
  Rtc.SetMemory(7+x,false);
  mode=MODE_Heure;  
  }

iwait();
/*
now.DayOfWeek()
*/
}

//Réglage de l'heure et de la date
void reglageheuredate(){
Retroeclairage();
lcd.setCursor(0,0);lcd.print(F("Reglage pendule"));

//réglage de l'heure
if (h==0) {  settime(24);  h=nbr; Serial.println ("h : "+String(h)); }
//réglage des minutes
else  if (m==0){   settime(60);    m=nbr;}
//réglage du jour
else  if (jr==0) {   settime(32);        jr=nbr;}
//réglage du mois
else  if (mo==0){   settime(13);        mo=nbr;}
//réglage de l'année
else if  (an==0) {     settime(9999);        an=nbr;}
if (an!=0) {   ecrannet();        wait=300;        Rtc.SetDateTime(RtcDateTime(an, mo, jr, h, m, 0));        mode=MODE_Heure;        }
     
iwait();
}

// Permet la saisie de la date et des heures et alarmes
void settime(int(maxi)){
  nbr=0;
lcd.setCursor(0,1);
if      (maxi==24) lcd.print(F("Heure   :"));
else if (maxi==60) lcd.print(F("Minutes :"));
else if (maxi==32) lcd.print(F("Jour    :"));
else if (maxi==13) lcd.print(F("Mois    :"));
else               lcd.print(F("Annee   :"));

// S'execute lorsqu'un chiffre est saisi sur la commande infrarouge
if (touch==3910598400 ||touch==4077715200  ||touch==3877175040 ||touch==2707357440  ||touch==4144561920  
||touch==3810328320  ||touch==2774204160  ||touch==3175284480 ||touch==2907897600  ||touch==3041591040   ) {
  telecir();
  if (maxi!=9999){
    if (aff[0]==0) {
      if ( atoi(com)<=int((maxi-1)/10)) {
       Serial.println ("maxi: "+String(maxi)+ " aff :"+String(aff)+ " com :"+String(com));delay(200);
       aff[0]=com[0];aff[1]='\-';afficheinput();
       }
      else {
       aff[0]='0'; aff[1]=com[0]; afficheinput();
       affectnbr(maxi);effaceinput();
       }
    }
    else {
      aff[1]=com[0];afficheinput();
      affectnbr(maxi);effaceinput();
    }
  } 
  else {
    if (aff[0]==0){
      aff[0]=com[0];aff[1]='\-';aff[2]='\-';aff[3]='\-';afficheinput();}
    else if (aff[1]=='\-') {
      aff[1]=com[0];afficheinput();}
    else if (aff[2]=='\-'){
      aff[2]=com[0];afficheinput();}
    else if (aff[3]=='\-'){
      aff[3]=com[0];affectnbr(maxi);afficheinput();aff[1]=aff[2]=aff[3]=0;}   
  }
wait=800;
}  
iwait();
}

void afficheinput(){
  lcd.setCursor(9,1);lcd.print(aff);delay(400);
}

void affectnbr(int maxi){
  nbr=atoi(aff);aff[0]=0;Serial.println ("nbr: "+String(nbr));
if (nbr>=maxi) nbr=0;
Serial.println ("maxi: "+String(maxi));
}

void effaceinput(){
  lcd.setCursor(9,1);
  if (maxi!=9999) lcd.print("    ");
    else lcd.print("----");
}

void affichheure(){
// Requete heure 
RtcDateTime now = Rtc.GetDateTime();    
h=now.Hour(), DEC;m=now.Minute(), DEC;s=now.Second(), DEC;jr=now.Day(), DEC;mo=now.Month(), DEC;an=now.Year(), DEC;
// Détection ultrasons?
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
  lcd.print(F(" "));
  lcd.write(165);
} else if (al[1] == true) {
   lcd.print(F(" "));
   lcd.write(58);
} else lcd.print(F("  "));


//Affichage lorsque les ultrasons détectent une présence <50cm
if (mes<80 and mes!=0) {
  Retroeclairage();
  wait=800;
  }
  
//Coupe le rétroéclairage en cas d'inactivité prolongée
wait--;
if (wait<0)  {
  analogWrite(BRIGHTNESS_PIN, 0);
  LED1.set(255,255,255);
  digitalWrite(BLUELEDRGB,255);
  digitalWrite(GREENLEDRGB,255);
  wait=0;
  } 
}

// Renseigne dans la variable touch le code infrarouge détecté lorsque c'est le cas
void touchir(){
if (IrReceiver.decode())  {
  touch=IrReceiver.decodedIRData.decodedRawData;
  }
IrReceiver.resume();// Receive the next value
}

// Ajuste le rétroéclairage en fonction de la mesure de luminosité ambiante
void Retroeclairage(){
//réglage de l'intensité lumineus du LCD selon la lumière ambiante
bright=255-(analogRead(LDR)/4);if (bright<0) bright=0;
analogWrite(BRIGHTNESS_PIN, bright);
}

void Turncolor();{
//LED
t++;
if (t>400) t=0;
if (t<50)  { r=bright;  g=bright-(t*bright/50);  b=(t*bright/50);}
else if (t>=50 and t<100) { r=bright-(t-50)*bright/50;  g=(t-50)*bright/50;  b=bright;}
else if (t>=100 and t<150) { r=0;  g=bright-(t-100)*bright/50;  b=bright;}
else if (t>=150 and t<200) { r=0;  g=(t-150)*bright/50;  b=bright-(t-150)*bright/50;}
else if (t>=200 and t<250) { r=(t-200)*bright/50;  g=bright-(t-200)*bright/50;  b=0;}
else if (t>=250 and t<300) { r=bright-(t-250)*bright/50;  g=(t-250)*bright/50;  b=0;}
else if (t>=300 and t<350) { r=0;  g=bright;  b=(t-300)*bright/50;}
else { r=(t-350)*bright/50;  g=bright;  b=bright-(t-350)*bright/50;}
//Serial.println ("rouge : "+ String(r)+", vert : "+String(g)+", bleu : "+String(b));
}


// Motif led alarm testé par la touche v+
void Ledalarm();{
for (x=1;x<50, x++){
  
  LED1.set(0,255,255);
  digitalWrite(BLUELEDRGB,0);
  digitalWrite(GREENLEDRGB,255);
  Delay(50);

    LED1.set(255,255,255);
  digitalWrite(BLUELEDRGB,255);
  digitalWrite(GREENLEDRGB,0);
  Delay();
}
}

// Assigne à la variable com la chaine correspondant au code infrarouge détécté.
void telecir(){
strcpy(com,"");
if (touch==3125149440) strcpy(com,"ch-");
if (touch==3108437760) strcpy(com,"ch"); 
if (touch==3091726080) strcpy(com,"ch+"); 
if (touch==3141861120) strcpy(com,"tr-");  
if (touch==3208707840) strcpy(com,"tr+");
if (touch==3158572800) strcpy(com,"pl"); 
if (touch==4161273600) strcpy(com,"v-"); 
if (touch==3927310080) strcpy(com,"v+");
if (touch==4127850240) strcpy(com,"eq");
if (touch==3910598400) strcpy(com,"0"); 
if (touch==3860463360) strcpy(com,"+100"); 
if (touch==4061003520) strcpy(com,"+200");  
if (touch==4077715200) strcpy(com,"1");
if (touch==3877175040) strcpy(com,"2"); 
if (touch==2707357440) strcpy(com,"3"); 
if (touch==4144561920) strcpy(com,"4");
if (touch==3810328320) strcpy(com,"5");
if (touch==2774204160) strcpy(com,"6"); 
if (touch==3175284480) strcpy(com,"7"); 
if (touch==2907897600) strcpy(com,"8");  
if (touch==3041591040) strcpy(com,"9");
touch=0;
}

//Renvoie vers l'affichage de l'heure après un temps d'inactivité dans les réglages d'heure ou d'alarme
void iwait(){
wait--;
if (wait<0) {
    wait=800;
    mode=MODE_Heure;strcpy(aff,"--");
    Retroeclairage();ecrannet();
    }
}

//Nettoyage de l'écran LCD
void ecrannet(){
lcd.init();
big.begin();
}