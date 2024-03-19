#include <Arduino.h>
//Macros
#define LEDAN digitalWrite(13, HIGH)
#define LEDAUS digitalWrite(13, LOW)
#define PumpeAN digitalWrite(A0, HIGH)
#define PumpeAUS digitalWrite(A0, LOW)
#define readSensor (uint16_t)analogRead(A7)

//Konstanten
const uint32_t Stunde = (3600UL * 1000UL);
const uint32_t Tag = (Stunde * 24UL);
const uint32_t BewaesserungTimeout = (15UL * 1000UL); //15s Pumpe An
const uint32_t PumpenTimeout = (Stunde * 3UL);        //Mindestzeit Pumpe aus bis zur nächsten Bewässerung
//const uint16_t feuchteMin = 470;           //so feucht darf der Boden maximal sein
//const uint16_t feuchteMax = 495;           //so trocken darf der Boden maximal sein
const uint16_t feuchteCalibMin = 380;        //unterer Wert fuer Kalibrierung (nass)
const uint16_t feuchteCalibMax = 580;        //oberer Wert fuer Kalibrierung (trocken)
const uint16_t feuchteNormMin = 475;
const uint16_t feuchteNormMax = 520;


const uint16_t feuchteMin = 40;          //so feucht darf der Boden maximal sein
const uint16_t feuchteMax = 50;           //so trocken darf der Boden maximal sein


//Variablen
uint16_t sensorWert = 0;
bool startup = true;

uint32_t pumpenTimer = 0;
//Prototypen
void printValues();
void bewaessern();

void setup() {
  pinMode(A0, OUTPUT);
  pinMode(A7, INPUT);
  PumpeAUS;
  Serial.begin(115200);
  //ersten Wert entsorgen
  sensorWert = readSensor;
  sensorWert = 0;
  pumpenTimer = millis();
}

void loop() {
  bewaessern();
}

void bewaessern()
{
  static uint32_t printTime = 0;
  const uint32_t PrintTimeout = 1000;
  static uint32_t BewaesserungTime = 0;
  uint16_t sensorTemp = 0;
  static bool printed = false;

  //8 mal messen und Mittelwert bilden
  for(uint8_t i = 0; i < 8; i++) {
    sensorTemp += readSensor;
  }
  sensorWert = sensorTemp >> 3;
  //Sensorwerte normalisieren (0-100)
  sensorWert = map(sensorWert, feuchteCalibMin, feuchteCalibMax, 100, 0);
  //Bewaesserung nur, wenn Feuchtigkeit im korrekten Bereich ist und die Mindestdauer zwischen zwei Bewaesserungen abgelaufen ist
  if(((sensorWert < feuchteMin) || (sensorWert < feuchteMax)) && ((millis() - pumpenTimer) > PumpenTimeout))
  {
    PumpeAN;
    LEDAN;
    Serial.println("Pumpe AN");
    BewaesserungTime = millis();
    pumpenTimer = millis();
    printed = false;
  }
  //Timer fuer Einschaltzeit Pumpe
  if((millis() - BewaesserungTime) > BewaesserungTimeout)
  {
    PumpeAUS;
    LEDAUS;
    if(!printed)
    {
      Serial.println("Pumpe AUS");
    }
    printed = true;
  }
  //Werte nur 1 mal pro sekunde ausgeben
  if((millis() - printTime) > PrintTimeout)
  {
    printTime = millis();
    printValues();  
  }
}
//Werte ausgeben
void printValues()
{
  Serial.print("\n\n\n\n\n\n\n\n\n\n\n\n\n");
  Serial.println("####################################################################\n");
  Serial.println("Sensorwert: " + String(sensorWert));
  Serial.print("Restzeit bis Pumpe wieder bewaessern darf: ");
  //uint32_t stunden = ((millis() - pumpenTimer) / 1000 / 3600) > 0;
  uint32_t minuten;
  if((PumpenTimeout < (millis() - pumpenTimer)))
  {
    minuten = 0;
  } else
  {
    minuten = ((PumpenTimeout - (millis() - pumpenTimer)) / 1000UL / 60UL);
  }
  //uint32_t sekunden = ((millis() - pumpenTimer) / 1000) > 0;
  Serial.println(String(minuten) + "min");
  Serial.println("\n####################################################################\n\n\n");
}
/*
Initialer Pumpenwert bei trockener Erde ca. 550 (stabile 5V)

*/