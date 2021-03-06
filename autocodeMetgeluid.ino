#include <Arduino_AVRSTL.h>
#include <SoftwareSerial.h>
using namespace std;
#define MP3_RX 12 // to TX
#define MP3_TX 9 // to RX

static int8_t select_SD_card[] = {0x7e, 0x03, 0X35, 0x01, 0xef};                   //Kies de sd card voor de speaker
static int8_t stopgeluid[] = {0x7e, 0x02, 0x0e, 0xef};                             //Stop het geluid dat nu wordt afgespeeld
static int8_t volume[] = {0x7e, 0x03, 0x31, 0x14, 0xef};                           //Volume regelen, 4de bit vanaf links bepaald hoe hard
static int8_t sirene[] = {0x7e, 0x04, 0x41, 0x00, 0x02, 0xef};                     //Sirene geluid
static int8_t overwinningsmuziekje[] = {0x7e, 0x04, 0x41, 0x00, 0x03, 0xef};       //Kies nummer om af te spelen, 5de bit vanaf links bepaalt welk mp3 bestand
SoftwareSerial MP3(MP3_RX, MP3_TX);

int TriggerPin1 = 2;                                                               // defineeren van de pinnen voor de afstandssensoren
int EchoPin1 = 13;
int EchoPin2 = 4;
int EchoPin4 = 7;
int EchoPin5 = 8;

int fanPin = 3;                                                                    // defineeren van de pin voor de ventilator

int motorForwardPin = 5;                                                           // defineeren van de pinnen voor de motoren
int motorReversePin = 6;
int motorForwardPin2 = 10;
int motorReversePin2 = 11;


const int treshold = 600;                                                          // sensor fire detection treshold
const int tresholdDichtbij = 2700;                                                 // sensor fire dichtbij treshold --waarde van lamp: 4980--

int afstand_te_meten_voor = 25;                                                    // cm 5, is linksvoor, cm4 is rechtsvoor, cm3 is de Voorkant sensor, cm2 is de rechterkant en cm1 is de linkerkant
int afstand_te_meten_zijkant = 30;


void setup() {
  Serial.begin(9600);
  pinMode(TriggerPin1, OUTPUT);
  pinMode(EchoPin1, INPUT);
  pinMode(EchoPin2, INPUT);

  pinMode(motorForwardPin, OUTPUT);
  pinMode(motorReversePin, OUTPUT);
  pinMode(motorForwardPin2, OUTPUT);
  pinMode(motorReversePin2, OUTPUT);

  pinMode(fanPin, OUTPUT);
  analogWrite(fanPin,0);
  MP3.begin(9600);
  MP3command(select_SD_card, 5);                                    //Stuur commands naar mp3 speler, laatste getal geeft door hoeveel bytes er gestuurd worden.
  MP3command(volume, 5);
}

void MP3command(int8_t command[], int len){
  Serial.print("\nMP3 Command => ");
  for(int i=0;i<len;i++){ MP3.write(command[i]); Serial.print(command[i], HEX); }
  delay(1000);
}

void rechtsaf(){
    analogWrite(motorForwardPin, 0);
    analogWrite(motorReversePin, 50);
    analogWrite(motorForwardPin2, 50);
    analogWrite(motorReversePin2, 0);
}


void linksaf(){
    analogWrite(motorForwardPin, 50);
    analogWrite(motorReversePin, 0);
    analogWrite(motorForwardPin2, 0);
    analogWrite(motorReversePin2, 50); 
}


void rechtdoor(){
    analogWrite(motorForwardPin, 90);
    analogWrite(motorReversePin, 0);
    analogWrite(motorForwardPin2, 73);
    analogWrite(motorReversePin2, 0);
}


void achteruit(){
    analogWrite(motorForwardPin, 0);
    analogWrite(motorReversePin, 130);
    analogWrite(motorForwardPin2, 0);
    analogWrite(motorReversePin2, 130);
}


void stoppen(){
    analogWrite(motorForwardPin, 0);
    analogWrite(motorReversePin, 0);
    analogWrite(motorForwardPin2, 0);
    analogWrite(motorReversePin2, 0);
}


void overwinnings_dansje(){
  MP3command(overwinningsmuziekje, 6);
  rechtsaf();
  delay(400);
  linksaf();
  delay(400);
  rechtsaf();
  delay(400);
  linksaf();
  delay(400);
  rechtsaf();
  delay(400);
  linksaf();
  delay(400);
  rechtsaf();
  delay(400);
  linksaf();
  delay(400);
  rechtsaf();
  delay(3600);
  linksaf();
  delay(3600);
  MP3command(stopgeluid, 4);
  stoppen();
  //ledjes hier nog laten knipperen? :)
}


vector<int> metenVoor(){
  int voorkantRechts = meten(EchoPin4);
  delay(80);
  int voorkantLinks = meten(EchoPin5);
  return {voorkantRechts,voorkantLinks};
  }

  
vector<int> metenZijkant(){
  int zijkantRechts = meten(EchoPin2);
  delay(80);
  int zijkantLinks= meten(EchoPin1);
  return {zijkantRechts,zijkantLinks};
  }


int meten(int echopinnummer){                                                                //Berekent de afstand
  digitalWrite(TriggerPin1, LOW);
  delayMicroseconds(5);
  digitalWrite(TriggerPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TriggerPin1, LOW);                                                            // Afstandssensor een sonar laten sturen
  
  int tijdgemeten = pulseIn(echopinnummer, HIGH);                                            // Meet hoelang de sonar deed om weer terug te komen bij de sensor
  int afstand = (tijdgemeten/2) / 29.1;                                                      // Afstand berekenen is reistijd/2 * snelheid van geluid (je kan ook / 29.1 doen) in centimeters.
  return afstand;}


char blokkade_checker(int afstandRechts, int afstandLinks, int meetafstandvoor,
int meetafstandzijkant, int afstandRechtsVoor, int afstandLinksVoor){                        // Checkt als er een blokkade voor is or er links of rechts ruimte vrij is.
  if(afstandRechtsVoor < meetafstandvoor || afstandLinksVoor < meetafstandvoor){             // Checkt of er meer dan 20 cm vrij is aan de voorkant
     
    if(afstandRechts >= meetafstandzijkant && afstandLinks >= meetafstandzijkant ||
    afstandRechts >= meetafstandzijkant && afstandLinks < meetafstandzijkant){
      return 1;}                                                                             // Als er aan links en rechts genoeg ruimte is OF alleen rechts genoeg ruimte, geeft hij 1 terug
      
    else if(afstandRechts < meetafstandzijkant && afstandLinks >= meetafstandzijkant){
      return 2;}                                                                             // Als er alleen links ruimte is geeft hij 2 terug
    else{
      return 3;
    }
  }
  else{
    return 0;}                                                                               // Geeft 0 terug als er genoeg ruimte aan de voorkant is en dus geen blokkade
}


void richting(int richting_getal, int afstand_te_meten){                                     // Stuurt naar de monitor welke richting de auto op moet(tijdelijke functie)
  if(richting_getal == 0){
    Serial.print("Ga rechtdoor\n");                                                          // Als de eerder gekozen richting 0 was kan de auto rechtdoor. 
    rechtdoor();
  }
    
  else if(richting_getal == 1){
    Serial.print("Ga naar rechts\n");                                                        // Als de eerder gekozen richting 1 was kan/moet de auto rechtsaf
    stoppen();
    delay(100);
    achteruit();
    delay(300);
    rechtsaf();
    delay(400);
  }
  else if(richting_getal == 2){
    Serial.print("Ga naar links\n");                                                         // Als de eerder gekozen richting 2 was dan kan de auto alleen linksaf
    stoppen();
    delay(100);
    achteruit();
    delay(300);
    linksaf();
    delay(400);
  }
  else if(richting_getal == 3){
    Serial.print("Ga achteruit\n");
    achteruit();
    delay(1000);
    int nieuwe_links = meten(EchoPin1);
    int nieuwe_rechts = meten(EchoPin2);

    if(nieuwe_rechts >= afstand_te_meten){
      rechtsaf();
      delay(400);
    }
    else if(nieuwe_rechts < afstand_te_meten && nieuwe_links >= afstand_te_meten){
      linksaf();
      delay(400);
    }
    else if(nieuwe_rechts < afstand_te_meten && nieuwe_links < afstand_te_meten){
      richting(richting_getal, afstand_te_meten);
    }
  }
}



vector<int> vlameSensorReadings(){
  int sensorReading1 = analogRead(A5);
  int sensorReading2 = analogRead(A4);
  int sensorReading3 = analogRead(A3);
  int sensorReading4 = analogRead(A2);
  int sensorReading5 = analogRead(A1);
  return {sensorReading1,sensorReading2,sensorReading3,sensorReading4,sensorReading5};
  }


int vlameSensorReadingsTotaal(){
  int sensorReading1 = analogRead(A5);
  int sensorReading2 = analogRead(A4);
  int sensorReading3 = analogRead(A3);
  int sensorReading4 = analogRead(A2);
  int sensorReading5 = analogRead(A1);
  return sensorReading1+sensorReading2+sensorReading3+sensorReading4+sensorReading5;
  }


void flameCheck(){
  vector<int> sensorReading = vlameSensorReadings();
  if (sensorReading[0]>treshold || sensorReading[1]>treshold || sensorReading[2]>treshold
  || sensorReading[3]>treshold || sensorReading[4]>treshold){
    flameLocaliser();
    }
  }


void vlamUitblazen(){
  int sensorReadingsTotaal = vlameSensorReadingsTotaal();
  analogWrite(fanPin,255);
  while (sensorReadingsTotaal > treshold){
    delay(3000);
    sensorReadingsTotaal = vlameSensorReadingsTotaal();
    }
  analogWrite(fanPin,0);
  }


void vlamGevonden(){
  int sensorReadingsTotaal = vlameSensorReadingsTotaal();
  Serial.println("***Vlam gevonden***");
  stoppen();
  while(sensorReadingsTotaal > treshold){
    vlamUitblazen();
    delay(5000);
    sensorReadingsTotaal = vlameSensorReadingsTotaal();
    }
  MP3command(stopgeluid, 4);
  achteruit();
  delay(500);
  stoppen();
  overwinnings_dansje();
  }


void flameLocaliser(){
  MP3command(sirene, 6);
  vector<int> afstandVoor = metenVoor();
  delay(80);
  vector<int> afstandZijkant = metenZijkant();

  int totaalSensorReadings = vlameSensorReadingsTotaal();
  vector<int> sensorReading = vlameSensorReadings();
  
  while (25 < afstandVoor[0] && 25 < afstandVoor[1] || totaalSensorReadings < tresholdDichtbij) {
    
    totaalSensorReadings = vlameSensorReadingsTotaal();
    sensorReading = vlameSensorReadings();
    
    if (sensorReading[0]>sensorReading[1] && sensorReading[0]>sensorReading[2] &&
    sensorReading[0]>sensorReading[3] && sensorReading[0]>sensorReading[4]){
      rechtsaf();
      delay(400);
      }
    else if (sensorReading[1]>sensorReading[0] && sensorReading[1]>sensorReading[2] &&
    sensorReading[1]>sensorReading[3] && sensorReading[1]>sensorReading[4]){
      rechtsaf();
      delay(200);
      }
    else if (sensorReading[2]>sensorReading[0] && sensorReading[2]>sensorReading[1] &&
    sensorReading[2]>sensorReading[3] && sensorReading[2]>sensorReading[4]){
      Serial.println("midden");
      rechtdoor();
      if (25 < afstandVoor[0] || 25 < afstandVoor[1]){
        afstandVoor = metenVoor();
        delay(80);
        afstandZijkant = metenZijkant();
        
        int reactie_keuze = blokkade_checker(metenZijkant[0], metenZijkant[1], 25, 30,
        metenVoor[0], metenVoor[1]);
        richting(reactie_keuze, 30);        
        
        break;
        }
      }
    else if (sensorReading[3]>sensorReading[0] && sensorReading[3]>sensorReading[1] &&
    sensorReading[3]>sensorReading[2] && sensorReading[3]>sensorReading[4]){
      linksaf();
      delay(200);
      }
    else if (sensorReading[4]>sensorReading[0] && sensorReading[4]>sensorReading[1] &&
    sensorReading[4]>sensorReading[2] && sensorReading[4]>sensorReading[3]){
      linksaf();
      delay(400);
      }
    rechtdoor();
    delay(100);  
      
    afstandVoor = metenVoor();
    }

  if (totaalSensorReadings > tresholdDichtbij){
    vlamGevonden();
    }
  }

  


void loop() {
  vector<int> afstandVoor = metenVoor();
  delay(80);
  vector<int> afstandZijkant = metenZijkant();
 
  int reactie_keuze = blokkade_checker(afstandZijkant[0], afstandZijkant[1],
  afstand_te_meten_voor, afstand_te_meten_zijkant,afstandVoor[0], afstandVoor[1]);
  richting(reactie_keuze, afstand_te_meten_zijkant);
  
  delay(80);

  flameCheck();
 
}
