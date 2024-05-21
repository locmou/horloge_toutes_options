/* REVEIL MULTIFONCTION */

/*
 * Composants :
 * ----------
 * Detecteur par ultrasons : HC-SR04
 * Horloge avec espaces mémoires : DS1307
 * Detecteur infrarouge : TL1838
 * résistance photosensible
 * Ecran LCD I2C 16*2
 * 3 leds RGB anodes communes
 * 2 relais optocoupleurs
 * Alimentation AC220/DC5V
 * Arduino nano
 * 
 * 
 *  
 * Tableau espace mémoire :
 * ----------------------
 * al[5][2] => 5 lignes, 2 colonnes:
 *         al          ligne   1 | 2
 *        --------------------------------------
 *        colonne               0 | 1
 *
 *        On/off          0   15  | 16
 *        Memlewe         1   17  | 18
 *        Prise 1         2   19  | 20
 *        Prise 2         3   21  | 22
 *        Lumière A/B     4   23  | 24 
 *
 * Mémoire alh[0]  10
 * Mémoire alm[0]  11
 * Mémoire alh[1]  12
 * Mémoire alm[1]  13
 */



/*
************************************************
******réglage de l'heure et de la date à controler ******
************************************************
*/




#include <Arduino.h>
#include <SPI.h> 

#include <RTClib.h>
RTC_DS1307 rtc;
const int DS1307_SDA_PIN = A4;
const int DS1307_SCL_PIN = A5;

// Gestion des leds sur pin pwm 
#include <RGB_LED.h>
RGB_LED LED1(9,10,6);

// Gestion des leds sur pin digitaux
const int DIGITLED1R=15; //A1
const int DIGITLED1G=16; //A2
const int DIGITLED1B=17; //A3
const int DIGITLED2R=13; 
const int DIGITLED2G=11;

// Gestion IR
#include <IRremote.hpp>
const int IR_PIN = 2;

// Detecteur ultrasons
#include <HCSR04.h>
// definition des broches du capteur ultrasons
const int TRIGPIN = 8;
const int ECHOPIN = 7;
// initialisation du capteur avec les broches utilisees.
UltraSonicDistanceSensor distanceSensor(TRIGPIN, ECHOPIN);

// Connexion alarme et leds
const uint8_t ALPIN[] = {3,4};

// Boutons 
const uint8_t BUTT[]={14,12};

// Affichage LCD
#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27,16,2);

// Gros chiffres
#include <BigFont02_I2C.h>
BigFont02_I2C  big(&lcd); // construct large font object, passing to it the name of our lcd object

//Définition des contrastes
const uint8_t BRIGHTNESS_PIN=5;   // Must be a PWM pin
const uint8_t LDR=A7;  // composante photorésistance sur la pin A7

// Déclaration des variables
unsigned long touch;
char aff[5],com[5];
uint8_t r,g,b,n,a,h,m,s,jr,mo,mes,bright,alh[2],alm[2];
int t=0,wait,but[2];
int maxi,nbr,an;
//float maxi;
bool al[5][2],pop[2],r1,b1,g1,r2,g2,alon[2];
//bool antial[2];
DateTime now;

// Déclaration des constantes pour les modes
enum Mode 
{
  MODE_Heure,
  MODE_Reglage_h,
  MODE_ALARM_INFO,
  MODE_Reglage_al,
  MODE_memlewe,
  MODE_P1,
  MODE_P2,
  MODE_alled
};
Mode mode;

// Déclarations de fonctions
void infoalarm(uint8_t a);
void reglagealarme(uint8_t a);
void modifal(uint8_t a);
void affichheure();
void touchir();
void Retroeclairage();
void reglageheuredate();
void telecir();
void iwait();
void ecrannet();
void settime(int maxi);
void afficheinput();
void memlewe(uint8_t a);
void Prise1(uint8_t a);
void Prise2(uint8_t a);
void typeledalarm(uint8_t a);
void affectnbr(int maxi);
void afficheinput();
void effaceinput();
void Turncolor();
void Ledalarm(uint8_t a);
void Checkserie();
void scrollText(int row, String message, int delayTime, int lcdColumns);

////////////////////////////////////////////////////////////////


void setup ()
{
rtc.begin();
if (! rtc.isrunning()) {
    //Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

// Pour remettre à l'heure lorsque le port série est relié à l'ordi
// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));


// never assume the Rtc was last configured by you, so
// just clear them to your needed state
rtc.writeSqwPinMode(DS1307_OFF);

// Infrarouges  
IrReceiver.begin(IR_PIN, DISABLE_LED_FEEDBACK);

//LCD
lcd.init(); // initialisation de l’afficheur
big.begin();
lcd.backlight();
r=10;b=180;g=10;r1=r2=b1=false;g1=g2=true;
Retroeclairage();
  
// Pin's
pinMode(9,OUTPUT);
pinMode(10,OUTPUT);
pinMode(6,OUTPUT);
pinMode(BRIGHTNESS_PIN, OUTPUT);
pinMode(DIGITLED1R, OUTPUT);
pinMode(DIGITLED1G, OUTPUT);
pinMode(DIGITLED1B, OUTPUT);
pinMode(DIGITLED2R, OUTPUT);
pinMode(DIGITLED2G, OUTPUT);
Serial.begin(115200);
Mode mode = MODE_Heure;

// Etalonnage des variables
for (a=0;a<2;a++)
{
  alh[a]=rtc.readnvram(10+(a*2));alm[a]=rtc.readnvram(11+(a*2));
  pinMode(ALPIN[a], OUTPUT);
  pinMode(BUTT[a], INPUT);  
  for (n=0;n<5;n++)
  {
   al[n][a]=rtc.readnvram(15+(2*n)+a);
  }
  pop[a]=false;
}
  wait=300;
}

///////////////////////////////////////////////////////////////////////


void loop ()
{
// Pression sur le bouton 10 al1 ou 12 al2
for (n=0;n<2;n++)
{
  if (!digitalRead(BUTT[n])) 
  {
    if (pop[n]) 
    {
      pop[n]=false;
      if (but[n]>3) 
      {
        //Appui long= réglage alarm
        al[0][n]= !al[0][n];rtc.writenvram(15+n, al[0][n]);
      }
      else 
      {
        // Appui court = on/off
        Serial.println(digitalRead (ALPIN[n]));

        if (!digitalRead (ALPIN[n]))  digitalWrite (ALPIN[n],HIGH);
        else digitalWrite (ALPIN[n],LOW);
      }
       but[n]=0;
    }
  }
  else 
  {
    pop[n]=true;
    but[n]++;
  }
 
// Alarme qui se déclenche durant les 15' qui suivent l'heure -1mn
//
//Serial.println( " al[0][n] : "+String(al[0][n])+", 60*alh[n])+alm[n] :"+String((60*alh[n])+alm[n])+", 60*h+m+1 :"+String(60*h+m+1)+", now.dayOfTheWeek :"+String(now.dayOfTheWeek()));  
//
  if (al[0][n] && 60*h+m+1>=(60*alh[n])+alm[n] && 60*h+m+1<=(60*alh[n])+alm[n]+15 
  && (al[1][n] || (!al[1][n] && now.dayOfTheWeek()>1)))
  {
    if (!alon[n])
    {
      alon[n]=true;
      modifal(n);
    }
  } 
  else 
  {
    if (alon[n]) 
    {
      alon[n]=false;
      modifal(n);
    }
  }
}

/*
// Provisoire pour déclencher l'alarme
//
if (touch==3927310080) Ledalarm(1);
*/

// reception infrarouge ?
touchir();

// déclenché par CH+ ou CH-
if (touch==3125149440  ||touch==3091726080  )
{
  wait=800;
  mode=MODE_Reglage_h ;ecrannet();an=9999;mo=jr=h=m=99;aff[0]=aff[1]='-';aff[2]=aff[3]=0;
}

// délenché par 100+, 200+
if (touch==3860463360  ||touch==4061003520  ) 
{
  telecir(); if (strcmp(com,"+100")==0) a=0; else a=1;
  wait=2;
  mode=MODE_ALARM_INFO;ecrannet();
}

/*
//déclenché par EQ
if (touch==4127850240) 
{
  if (mode==MODE_ALARM_INFO)
  {
    telecir();
    wait=800;
    mode=MODE_Reglage_al;ecrannet();aff[0]='-';aff[1]='-';alh[a]=alm[a]=99;
  }
}
*/

//Depart vers les sous programmes
if (mode==MODE_Heure) affichheure();
else if (mode==MODE_Reglage_h ) reglageheuredate();
else if (mode==MODE_ALARM_INFO) infoalarm(a);
else if (mode==MODE_Reglage_al) reglagealarme(a);
else if (mode==MODE_memlewe) memlewe(a);
else if (mode==MODE_P1) Prise1(a);
else if (mode==MODE_P2) Prise2(a);
else typeledalarm(a);

//Affichage serie des variables
// Checkserie();
}

///////////////////////////////////////////////////////////////////////


void modifal(uint8_t a)
{
// Alarme qui vient de démarrer
if (alon[a])
{
  Ledalarm(a);
  if (al[2][a])
  {
    if (digitalRead (ALPIN[0])==LOW)  digitalWrite (ALPIN[0],HIGH);
    else  digitalWrite (ALPIN[0],LOW);
  }
  if (al[3][a])
  {
    if (digitalRead (ALPIN[1])==LOW)  digitalWrite (ALPIN[1],HIGH);
    else digitalWrite (ALPIN[1],LOW);
  }
}
// Alarme qui vient de s'éteindre
else
{
    if (al[2][a])
  {
    digitalWrite (ALPIN[0],HIGH);
  }
  if (al[3][a])
  {
    digitalWrite (ALPIN[1],HIGH);
  }
}
}

///////////////////////////////////////////////////////////////////////////////////////////////


// Affiche les heures des alarmes
void infoalarm(uint8_t a)
{
Turncolor();g1=g2=b1=false;
if (a==0) {r1=true;r2=false;} else {r1=false;r2=true;}
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Ala."+String(a+1)+"->");
lcd.print(String(alh[a])+ ":" +String(alm[a]));
if (al[1][a]) lcd.print(" we+"); else lcd.print(" we-");
lcd.setCursor(0,1);
scrollText(1, "Prise 1 : "+ txt (2,a)+", prise 2 : "+txt (3,a)+", Leds : "+txt (4,a)+". modif: EQ ..", 300, 16);
iwait();
}


String txt(uint8_t n,uint8_t a)
{
if (n<4) 
{
  if (al[n][a]) return "on"; else return "off";
}
else {
  if (al[4][a]) return "cool"; else return "hard";
}
}

///////////////////////////////////////////////////////////////////////


//Réglage des alarmes 1 et 2x
void reglagealarme(uint8_t(a))
{
Turncolor();g1=g2=b1=false;
if (a==0) {r1=true;r2=false;} else {r1=false;r2=true;}
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Reglage alarme "+String(a+1));
if (alh[a]==99) {settime(24);alh[a]=nbr;}
else{  settime(60);  alm[a]=nbr;
if (alm[a]!=99)
  {
   wait=800;
   rtc.writenvram(10+(a*2), alh[a]);
   rtc.writenvram(11+(a*2), alm[a]); 
   mode=MODE_memlewe;    
   ecrannet();
  }
}
iwait();
}

//////////////////////////////////////////////////////////////////////


//Input demande si l'alarme fonctionne le we
void memlewe(uint8_t(a))
{
Turncolor();g1=g2=r1=r2=false;b1=true;
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Alarme "+String(a+1)+" le we?");
lcd.setCursor(0,1);
lcd.print("oui=tr+, non=tr-");
//tr-
if (touch==3141861120)
{
  al[1][a]= false;
  rtc.writenvram(17+a, false);
  r=10;b=180;g=10;r1=r2=b1=false;g1=g2=true;
  Retroeclairage();wait=800;touch=0;
  mode=MODE_P1;  
}
//tr+  
if (touch==3208707840)
{  
  al[1][a]=true;
  rtc.writenvram(17+a, true);
  r=10;b=180;g=10;r1=r2=b1=false;g1=g2=true;
  Retroeclairage(); wait=800;touch=0;
  mode=MODE_P1;  
}
iwait();
}
  
///////////////////////////////////////////////////////////////////////


void Prise1(uint8_t(a))
{
Turncolor();g1=g2=r1=r2=false;b1=true;
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Al. "+String(a+1)+": prise 1?");
lcd.setCursor(0,1);
lcd.print("oui=tr+, non=tr-");
//tr-
if (touch==3141861120)
{
  al[2][a]=false;
  rtc.writenvram(19+a, false);
  r=10;b=180;g=10;r1=r2=b1=false;g1=g2=true;
  Retroeclairage();wait=800;touch=0;
  mode=MODE_P2;  
}
//tr+  
if (touch==3208707840)
{  
  al[2][a]=true;
  rtc.writenvram(19+a, true);
  r=10;b=180;g=10;r1=r2=b1=false;g1=g2=true;
  Retroeclairage(); wait=800;touch=0;
  mode=MODE_P2;  
}
iwait();
}

///////////////////////////////////////////////////////////////////////


void Prise2(uint8_t(a))
{
Turncolor();g1=g2=r1=r2=false;b1=true;
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Al. "+String(a+1)+": prise 2?");
lcd.setCursor(0,1);
lcd.print("oui=tr+, non=tr-");

//tr-
if (touch==3141861120)
{
  al[3][a]=false;
  rtc.writenvram(21+a, false);
  r=10;b=180;g=10;r1=r2=b1=false;g1=g2=true;
  Retroeclairage();wait=800;touch=0;
  mode=MODE_alled;  
}
//tr+  
if (touch==3208707840)
{  
  al[3][a]=true;
  rtc.writenvram(21+a, true);
  r=10;b=180;g=10;r1=r2=b1=false;g1=g2=true;
  Retroeclairage(); wait=800;touch=0;
  mode=MODE_alled;  
}
iwait();
}

///////////////////////////////////////////////////////////////////////


void typeledalarm(uint8_t(a))
{
Turncolor();g1=g2=r1=r2=false;b1=true;
Retroeclairage();
lcd.setCursor(0,0);
lcd.print("Al. "+String(a+1)+": led cool?");
lcd.setCursor(0,1);
lcd.print("oui=tr+, non=tr-");
//tr-
if (touch==3141861120)
{
  al[4][a]=false;
  rtc.writenvram(23+a, false);
  r=10;b=180;g=10;r1=r2=b1=false;g1=g2=true;
  Retroeclairage();wait=800;touch=0;
  mode=MODE_Heure;  
}
//tr+  
if (touch==3208707840)
{  
  al[4][a]=true;
  rtc.writenvram(23+a, true);
  r=10;b=180;g=10;r1=r2=b1=false;g1=g2=true;
  Retroeclairage(); wait=800;touch=0;
  mode=MODE_Heure;  
}
iwait();
}

///////////////////////////////////////////////////////////////////////


//Réglage de l'heure et de la date
void reglageheuredate()
{

// Jeux de couleurs des leds pendant le réglage
Turncolor();
if (t%3==0)
{
  r1=random(2);
  r2=random(2);
  g1=random(2);
  g2=random(2);
  b1=random(2);
}
Retroeclairage();
lcd.setCursor(0,0);lcd.print(F("Reglage pendule"));

//réglage de l'heure
if (h==99)
{
  settime(24);  
  h=nbr;
}
//réglage des minutes
else  if (m==99)
{
  settime(60);
  m=nbr;
}
//réglage du jour
else  if (jr==99)
{
  settime(32);
  jr=nbr;
}
//réglage du mois
else  if (mo==99)
{
  settime(13);
  mo=nbr;
}

//réglage de l'année
else if  (an==9999)
{
  settime(9999);// aff[0]=aff[1]=aff[2]=aff[3]='-';
  an=nbr;
}
if (an!=9999)
{
  //aff=ensemble vide...??
  ecrannet();
  wait=300;
  rtc.adjust(DateTime(an, mo, jr, h, m, 0));  
  r=10;b=180;g=10;r1=r2=b1=false;g1=g2=true;
  Retroeclairage();
  mode=MODE_Heure;
}    
iwait();
}

////////////////////////////////////////////////////////////////////////


// Permet la saisie de la date et des heures et alarmes
void settime(int(maxi))
{
nbr=99;
lcd.setCursor(0,1);
if      (maxi==24) lcd.print(F("Heure   :"));
else if (maxi==60) lcd.print(F("Minutes :"));
else if (maxi==32) lcd.print(F("Jour    :"));
else if (maxi==13) lcd.print(F("Mois    :"));
else               lcd.print(F("Annee   :"));
afficheinput();

// S'execute lorsqu'un chiffre est saisi sur la commande infrarouge
if (touch==3910598400 ||touch==4077715200  ||touch==3877175040 ||touch==2707357440  ||touch==4144561920  
||touch==3810328320  ||touch==2774204160  ||touch==3175284480 ||touch==2907897600  ||touch==3041591040   ) 
{
  telecir();
  if (maxi!=9999)
  {
    if (aff[0]=='-') 
    {
      if ( atoi(com)<=int((maxi-1)/10)) 
      {
       aff[0]=com[0];afficheinput();

      }
      else 
      {
       aff[0]='0';aff[1]=com[0];afficheinput();
       affectnbr(maxi);
      }     
    }
    else 
    {
      aff[1]=com[0];afficheinput();
      affectnbr(maxi);
      
    }
  } 
  else 
  {
    nbr=9999;
    if (aff[0]=='-')
    {
      aff[0]=com[0];afficheinput();
    }
    else if (aff[1]=='-') 
    {
      aff[1]=com[0];afficheinput();
    }
    else if (aff[2]=='-')
    {
      aff[2]=com[0];afficheinput();
    }
    else if (aff[3]=='-')
    {
      aff[3]=com[0];affectnbr(maxi);afficheinput();
    }   
  }
wait=800;
}  
iwait();
}

////////////////////////////////////////////////////////////////////////


void afficheinput()
{
  lcd.setCursor(9,1);lcd.print(aff);
}

///////////////////////////////////////////////////////////////////////


void affectnbr(int maxi)
{
  if (nbr<maxi) 
  {
    nbr=atoi(aff);
    aff[0]='-';aff[1]='-';
    delay(400);
    effaceinput();
    if (maxi==32) {aff[2]=aff[3]='-';}
    if (maxi==9999) {aff[0]=aff[1]=aff[2]=aff[3]=0;}
  }
  else
  {
    if (maxi!=99)
    {
      aff[0]=aff[1]=aff[2]=aff[3]='-';
      nbr=9999;
    } 
    else 
    {
      aff[0]=aff[1]='-';aff[2]=aff[3]=0;
      nbr=99;
    }
  }
}

///////////////////////////////////////////////////////////////////////


void effaceinput()
{
  lcd.setCursor(9,1);
  if (maxi!=99) lcd.print("    ");
    else lcd.print("----");
}

////////////////////////////////////////////////////////////////////////


void affichheure()
{

DateTime now = rtc.now();

h=now.hour();
m=now.minute();
s=now.second();
jr=now.day();
mo=now.month();
an=now.year();
  
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
if (al[0][0] && al[0][1]) 
{
  lcd.write(165);
  lcd.write(58);
} 
else if (al[0][0]) 
{
  lcd.print(F(" "));
  lcd.write(165);
} 
else if (al[0][1]) 
{
  lcd.print(F(" "));
  lcd.write(58);
} 
else 
{
  lcd.print(F("  "));
}

//Affichage lorsque les ultrasons détectent une présence <50cm
// Détection ultrasons?
mes=(int)distanceSensor.measureDistanceCm()+1;
if (mes<50 and mes!=0) 
{
  // répétition pour éviter de détections parasites ...:
  if ((int)distanceSensor.measureDistanceCm()+1<50)
  {   
    if (digitalRead (ALPIN[0])==LOW)  digitalWrite (ALPIN[0],HIGH);
    if (digitalRead (ALPIN[1])==LOW)  digitalWrite (ALPIN[1],HIGH);        
    r=10;b=200;g=10;r1=r2=b1=false;g1=g2=true;
    Retroeclairage();
    wait=800;
  }
}
  
//Coupe le rétroéclairage en cas d'inactivité prolongée
wait--;
if (wait<0)  
{
  analogWrite(BRIGHTNESS_PIN, 0);
  LED1.set(255,255,255);
  digitalWrite(DIGITLED1R,1);
  digitalWrite(DIGITLED1G,1);
  digitalWrite(DIGITLED1B,1);
  digitalWrite(DIGITLED2R,1);
  digitalWrite(DIGITLED2G,1);
  wait=0;
} 
}

///////////////////////////////////////////////////////////////////////



// Renseigne dans la variable touch le code infrarouge si détecté 
void touchir()
{
if (IrReceiver.decode())  
{
  touch=IrReceiver.decodedIRData.decodedRawData;
}
IrReceiver.resume();// Receive the next value
}


///////////////////////////////////////////////////////////////////////


// Ajuste le rétroéclairage en fonction de la mesure de luminosité ambiante
void Retroeclairage()
{
//réglage de l'intensité lumineus du LCD selon la lumière ambiante
bright=(analogRead(LDR)/4);
analogWrite(BRIGHTNESS_PIN, 255-bright);

 // Ajustement leds

LED1.set(255-(r-r*bright/255),255-(g-g*bright/255),255-(b-b*bright/255));
digitalWrite(DIGITLED1R,!r1);digitalWrite(DIGITLED1G,!g1);digitalWrite(DIGITLED1B,!b1);digitalWrite(DIGITLED2R,!r2);digitalWrite(DIGITLED2G,!g2);
}

///////////////////////////////////////////////////////////////////////


void Turncolor()
{
//LED
t=t+3;
if (t>400) t=0;
if (t<50)  { r=bright;  g=bright-(t*bright/50);  b=(t*bright/50);}
else if (t>=50 and t<100) { r=bright-(t-50)*bright/50;  g=(t-50)*bright/50;  b=bright;}
else if (t>=100 and t<150) { r=0;  g=bright-(t-100)*bright/50;  b=bright;}
else if (t>=150 and t<200) { r=0;  g=(t-150)*bright/50;  b=bright-(t-150)*bright/50;}
else if (t>=200 and t<250) { r=(t-200)*bright/50;  g=bright-(t-200)*bright/50;  b=0;}
else if (t>=250 and t<300) { r=bright-(t-250)*bright/50;  g=(t-250)*bright/50;  b=0;}
else if (t>=300 and t<350) { r=0;  g=bright;  b=(t-300)*bright/50;}
else { r=(t-350)*bright/50;  g=bright;  b=bright-(t-350)*bright/50;}
}

////////////////////////////////////////////////////////////////////////


// Motif led alarm testé par la touche v+
void Ledalarm(uint8_t a)
{
// mode hard
if (!al[4][a]){
digitalWrite(DIGITLED1R,1);digitalWrite(DIGITLED1G,1);digitalWrite(DIGITLED1B,1);digitalWrite(DIGITLED2R,1);digitalWrite(DIGITLED2G,1);
  for (t=1;t<25; t++)
  { 
    LED1.set(0,255,255);
    digitalWrite(DIGITLED1R,0);
    digitalWrite(DIGITLED2R,0);  
    delay(180);
    LED1.set(255,255,255);
    digitalWrite(DIGITLED1R,1);
    digitalWrite(DIGITLED2R,1); 
    delay(50);
  }
}
// mode cool
else
{
 r1=r2=g1=g2=b1=false;
  for (t=1;t<511; t++)
  { 
    if (t<50)  { r=1;  g=0;  b=0;}
    else if (t>=50 and t<100) { r=1;  g=0;  b=0;}
    else if (t>=100 and t<150) { r=2;  g=1;  b=0;}
    else if (t>=150 and t<200) { r=2;  g=1;  b=0;}
    else if (t>=200 and t<250) { r=2+(t-200)/10;  g=1+(t-200)/15;  b=0;}
    else if (t>=250 and t<300) { r=7+(t-250)/8;  g=5+(t-250)/15;  b=0;}
    else if (t>=300 and t<350) { r=14+(t-300)/6;  g=9+(t-300)/10;  b=0;r1=true;}
    else if (t>=350 and t<400) { r=23+(t-350)/4;  g=14+(t-350)/5;  b=2;r2=true;}
    else if (t>=400 and t<450) { r=35+(t-400)*4;  g=24+(t-400)*4;  b=t-398;g1=true;g2=true;}
    else { r=255;  g=255;  b=255;b1=true;}
    Retroeclairage();
    delay(150);
  }
} 
touch=0;
}

////////////////////////////////////////////////////////////////////////


// Assigne à la variable com la chaine correspondant au code infrarouge détécté.
void telecir()
{
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

///////////////////////////////////////////////////////////////////////


//Renvoie vers l'affichage de l'heure après un temps d'inactivité dans les réglages d'heure ou d'alarme
void iwait()
{
wait--;
if (wait<0) 
{
  wait=800;
  mode=MODE_Heure;strcpy(aff,"--");  r=10;b=200;g=10;r1=r2=b1=false;g1=g2=true;
  Retroeclairage();ecrannet();
}
}

///////////////////////////////////////////////////////////////////////


//Nettoyage de l'écran LCD
void ecrannet()
{
lcd.init();
big.begin();
}

////////////////////////////////////////////////////////////////////////


void scrollText(int row, String message, int delayTime, int lcdColumns) 
{
  for (int i=0; i < lcdColumns; i++) 
  {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < int(message.length()); pos++) 
  {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
    touchir();
    //déclenché par EQ
    if (touch==4127850240) 
    {
      if (mode==MODE_ALARM_INFO)
      {
        pos=message.length();
        //telecir();
        wait=800;
        mode=MODE_Reglage_al;ecrannet();aff[0]='-';aff[1]='-';alh[a]=alm[a]=99;
      }
    }
    // délenché par 100+, 200+
    if (touch==3860463360  ||touch==4061003520  ) 
    {
    pos=message.length();
    telecir(); if (strcmp(com,"+100")==0) a=0; else a=1;
    wait=2;
    mode=MODE_ALARM_INFO;ecrannet();
    }
  }
}

///////////////////////////////////////////////////////////////////////

/*
void Checkserie()
{
Serial.println(digitalRead (ALPIN[0]));
Serial.println(digitalRead (ALPIN[1]));
 // Serial.println( " h : "+String(h)+", m :"+String(m)+", jr :"+String(jr)+", mo :"+String(mo)+", an :"+String(an));
 // Serial.println("r : "+ String(r) + ", g : "+String(g)+", b :"+String(b)+", mes :"+String(mes)+", bright :"+String(bright));
  //Serial.println("alh[0] : "+ String(alh[0]) + ", alm[0] : "+String(alm[0])+", alh[1] :"+String(alh[1])+", alm[1] :"+String(alm[1]) +", wait :"+String(wait)+", maxi :"+String(maxi));
delay(100);
}
*/
