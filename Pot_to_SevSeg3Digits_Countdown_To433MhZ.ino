/*
 ce code permet d'entrer via un potentiometre un delai (en minutes) à partir duquel les blocs prises commandés en 433Mhz sont mis hors tension.
Afin de fonctionner, il est important de mentionner les codes d'allumage et extinction des differents blocs prises (dans SWITCH_ON et SWITH_OFF cidessous)
ces codes peuvent etre determinés en utilisant la librairie arduino RC-Switch-2.52 
*/

#include <RCSwitch.h> // library for 433Mhz transmitters
RCSwitch mySwitch = RCSwitch();

#include <SevSeg.h>
SevSeg Xseg; //Instantiate a seven segment controller object
/*Liste des pins de l'arduino et affectation
 * A0 kPotPin
 * A1  kPot 5V
 * A2 kpot GND
 * A3
 * A4 TX Data
 * A5
 * A6 
 * A7 
 * 1
 * 2 SevSeg G
 * 3 SevSeg C
 * 4 SevSeg dp
 * 5 SevSeg D
 * 6 SevSeg E
 * 7 SevSeg B
 * 8 SevSeg F
 * 9 SevSeg A
 *10 SevSeg Cathode 1
 *11 SevSeg Cathode 2
 *12 SevSeg Cathode 3
 *13 
*/
const int kPotPin=A0; // entrée du potentiometre
int PotValue;
int PreviousCountDownAtStart=1;
int CountDownAtStart; //valeur du compte à rebour au debut, définie par le potentiomètre
int CountDown;  //la valeur du compte à rebour
int previousCountDown; // la valeur antérieure du countdown, utilisé pour couper les lampes qd on arrive a zero
int CountDownPrint;  // La valeur du compte a rebour imprimée sur les leds
long  millisecondsPrint; 
long previousmillis=0;
const int kPrintDelay=1000; // delay in millis between 2 consecutive prints
boolean LightStatus;
const int kMeasureDelay = 100; // delai entre 2 mesures du potentionmetre
boolean DEBUG=HIGH; // Set to HIGH to print info on serial port
boolean PotChange=HIGH; // used to see if potentiometer has changed in value
const int CountDownUnit=6; // set the unit in secs ==> for minutes, set 60
const long MillisInOneSec=997; // defines the length of one sec in milliseconds
const int MaxCountDown=999; // la valeur maximale du compte a rebours

void setup()  
{
pinMode (A1,OUTPUT);   // 5V for pot
digitalWrite(A1,HIGH);
pinMode (A2,OUTPUT);   // GND for pot
digitalWrite(A2,LOW);

mySwitch.enableTransmit(A4);   // Transmitter is connected to Arduino A4
  byte numDigits = 3;   
  byte digitPins[] = {12, 11, 10}; // <--put one resistor (ex: 220 Ohms, or 330 Ohms, etc, on each digit pin)
  byte segmentPins[] = {9,7,3,5,6,8,2,4}; //Segments: A,B,C,D,E,F,G,Period

  Xseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins);
  Xseg.setBrightness(50); //Note: 100 brightness simply corresponds to a delay of 2000us after lighting each segment. A brightness of 0 
                            //is a delay of 1us; it doesn't really affect brightness as much as it affects update rate (frequency).
                            //Therefore, for a 4-digit 7-segment + pd, COMMON_ANODE display, the max update rate for a "brightness" of 100 is 1/(2000us*8) = 62.5Hz.
                            //I am choosing a "brightness" of 10 because it increases the max update rate to approx. 1/(200us*8) = 625Hz.
                            //This is preferable, as it decreases aliasing when recording the display with a video camera....I think.
    Serial.begin(9600);
    previousmillis=millis();
}

void loop()  
{
////////////////mesure du potentiomètre ///////////////////////
if(millis() % kMeasureDelay==0)  //check if it is time to measure photocellvalue
  {PotValue= analogRead(kPotPin);// save photocell value value in var photocellValue
  }


////////////////Etalonage de LedDispValue entre 0 et MaxCountDown///////////////
CountDownAtStart=map(PotValue,0,1023,MaxCountDown,0);

//// en cas de changement du potentiometre==>reinitialisation
if(PreviousCountDownAtStart> CountDownAtStart+5 ||PreviousCountDownAtStart< CountDownAtStart-5 ) // qd PreviousCountDownAtStart est different de CountDown+/-1
    {PreviousCountDownAtStart = CountDownAtStart; // souvegarde du nouveau CountDownatStart
    CountDown=CountDownAtStart; 
   Switch_ON();
 previousmillis=millis(); }
else //si le potentionmtre n'a pas bougé ==> countdown enclenclé
{ switch (CountDown)
{ case MaxCountDown: //dans ce cas il n'y a pas de countdown, on reste allumé tout le temps
CountDown=MaxCountDown;
break;
case  0 :
if (LightStatus==HIGH) // si on vient d une lumiere allumée
{
Switch_OFF();
}

break;
default:
  if( millis() - previousmillis > (MillisInOneSec*CountDownUnit))
  {previousCountDown=CountDown; // sauve la valeur dans previouscountdown
    CountDown--;
  previousmillis=millis();
}
}
}
////////// Serial Printing if DEBUG = HIGH/////////////
if(DEBUG==HIGH && millis() - millisecondsPrint> kPrintDelay)
  {
  Serial.print("LightStatus=");
  Serial.print(LightStatus);
  Serial.print(" /CountDown=");
  Serial.println(CountDown);

millisecondsPrint=millis();
}
/////////////////////////////////////////////

///////////////////7 Digit Display of LEdDispValue Variable /////

Xseg.setNumber(CountDown,1);
Xseg.refreshDisplay(); // Must run repeatedly; don't use blocking code (ex: delay()) in the loop() function or this won't work right
}

////////switch functions////////

void Switch_ON() //les codes des blocs prises ci-dessous doivent être fournis!
  {
  mySwitch.send("010001010101010101010101");  //65557 lampe 1 salon allumée
  mySwitch.send("010001010100010000010101");  //65557 lampe 2 salon allumée
  mySwitch.send("010001010101010000010101");  //65557 lampe 3 salon allumée} // acte le changement du potentiomètre
  LightStatus=HIGH;
}

void Switch_OFF()//les codes des blocs prises ci-dessous doivent être fournis!
  {
  mySwitch.send("000001010101000000010100"); //65556 lampe 1 salon coupée
  mySwitch.send("000001010100010000010100"); //65556 lampe 2 salon coupée
  mySwitch.send("000001010101010000010100"); //65556 lampe 3 salon coupée
  LightStatus=LOW;
}
