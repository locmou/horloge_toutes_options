
#include <IRremote.h>
//int RECV_PIN = 5; //define input pin on Arduino
unsigned long touch;


void setup()
{
Serial.begin(57600);
//irrecv.enableIRIn(); // Start the receiver
IrReceiver.begin(5, ENABLE_LED_FEEDBACK);
}

void loop() 
{
if (IrReceiver.decode())  affichtouch(IrReceiver.decodedIRData.decodedRawData);
}

void affichtouch(unsigned long(touch))
{
   //Serial.println(touch);
  if (touch==3125149440) Serial.println("ch-");
  if (touch==3108437760) Serial.println("ch"); 
  if (touch==3091726080) Serial.println("ch+"); 
  if (touch==3141861120) Serial.println("tr-");  
  if (touch==3208707840) Serial.println("tr+");
  if (touch==3158572800) Serial.println("pl"); 
  if (touch==4161273600) Serial.println("v-"); 
  if (touch==3927310080) Serial.println("v+");
  if (touch==4127850240) Serial.println("eq");
  if (touch==3910598400) Serial.println("0"); 
  if (touch==3860463360) Serial.println("+100"); 
  if (touch==4061003520) Serial.println("+200");  
  if (touch==4077715200) Serial.println("1");
  if (touch==3877175040) Serial.println("2"); 
  if (touch==2707357440) Serial.println("3"); 
  if (touch==4144561920) Serial.println("4");
  if (touch==3810328320) Serial.println("5");
  if (touch==2774204160) Serial.println("6"); 
  if (touch==3175284480) Serial.println("7"); 
  if (touch==2907897600) Serial.println("8");  
  if (touch==3041591040) Serial.println("9");
 IrReceiver.resume(); // Receive the next value
}
