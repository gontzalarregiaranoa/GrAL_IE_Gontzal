#include <SPI.h>
#include <LoRa.h>
#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

int counter = 0;
int Value;

int ledGreen = 3;
int ledYellow = 4;
int ledRed = 5;
int buzzerPin = 1;

void setup() {
  Serial.begin(9600);
  dht.begin();

  while (!Serial);
  Serial.println("LoRa Sender");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  pinMode(buzzerPin,OUTPUT);
  delay(1000);
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  float hic = dht.computeHeatIndex(t, h, false); // Heat Index = sentsazio termikoa

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.println(F("°C "));

  Value = analogRead(1);
  Serial.print( "The quality of the air is " );
  Serial.print(Value);
  Serial.println( " ppm" );

  if(Value<=55){
    digitalWrite(ledGreen,LOW);
    digitalWrite(ledYellow,LOW);
    digitalWrite(ledRed,LOW);
    delay(50);
  }

  if(Value>=56 && Value<=65){
    digitalWrite(ledGreen,HIGH);
    digitalWrite(ledYellow,LOW);
    digitalWrite(ledRed,LOW);
    delay(50);
  }

  if(Value>=66 && Value<=350){
    digitalWrite(ledGreen,LOW);
    digitalWrite(ledYellow,HIGH);
    digitalWrite(ledRed,LOW);
    delay(50);
  }

    if(Value>=351){
    digitalWrite(ledGreen,LOW);
    digitalWrite(ledYellow,LOW);
    digitalWrite(ledRed,HIGH);
    delay(50);
  }

  if(Value>=900){
    tone(buzzerPin, 988, 100);
  }

  delay(500);
  digitalWrite(ledGreen,LOW);
  digitalWrite(ledYellow,LOW);
  digitalWrite(ledRed,LOW);

  LoRa.beginPacket();
  LoRa.write((uint8_t*)&Value, sizeof(Value));
  LoRa.write((uint8_t*)&h, sizeof(h));
  LoRa.write((uint8_t*)&t, sizeof(t));
  LoRa.write((uint8_t*)&hic, sizeof(hic));
  LoRa.endPacket();

  counter++;
  Serial.print("Sending packet: ");
  Serial.println(counter);

Serial.println(" ");
delay(500);
}
