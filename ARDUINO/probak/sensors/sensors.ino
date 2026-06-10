#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define aire 3
#define CO2 4
#define gas 5
#define pinBuzzer 1

String buttonPress = "button pressed";
bool x = false;
//byte localAddress = 0xBB;  
//byte destination = 0xFF;

int Value, incoming, pktNum = 0, pktNum1, pktNum2, counter1 = 0, counter2 = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();

  //while (!Serial); //Espera hasta que la comunicación serial esté disponible
  delay(500);
  Serial.println("LoRa program starting");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  pinMode(aire,OUTPUT);
  pinMode(CO2,OUTPUT);
  pinMode(gas,OUTPUT);
  pinMode(pinBuzzer,OUTPUT);
  delay(500);
}

void loop() {
  Value = analogRead(1);
  Serial.print("Air-quality: ");
  Serial.print(Value);
  Serial.println(" ppm");

  if (counter2 == 4000){
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    float hic = dht.computeHeatIndex(t, h, false);
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C  Heat index: "));
    Serial.print(hic);
    Serial.println(F("°C "));
    
    Serial.print( "The quality of the air is " );
    Serial.print(Value);
    Serial.println( " ppm" );
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&pktNum, sizeof(pktNum));
    LoRa.write((uint8_t*)&Value, sizeof(Value));
    LoRa.write((uint8_t*)&h, sizeof(h));
    LoRa.write((uint8_t*)&t, sizeof(t));
    LoRa.write((uint8_t*)&hic, sizeof(hic));
    LoRa.endPacket();
    pktNum++;
    counter2 = 0;
    delay(1000);
  }
  else{
    counter2++;
    onReceive(LoRa.parsePacket());
  }
  if (x==true){
    if(Value<=99){
      digitalWrite(aire,LOW);
      digitalWrite(CO2,LOW);
      digitalWrite(gas,LOW);
      delay(50);
    }

    if(Value>=100 && Value<=150){
      digitalWrite(aire,HIGH);
      digitalWrite(CO2,LOW);
      digitalWrite(gas,LOW);
      delay(50);
    } 

    if(Value>=151 && Value<=399){
      digitalWrite(aire,LOW);
      digitalWrite(CO2,HIGH);
      digitalWrite(gas,LOW);
      delay(50);
    }

    if(Value>=400){
      digitalWrite(aire,LOW);
      digitalWrite(CO2,LOW);
      digitalWrite(gas,HIGH);
      delay(50);
    }
  }
  else{
      digitalWrite(aire,LOW);
      digitalWrite(CO2,LOW);
      digitalWrite(gas,LOW);
  }

  if(Value>=900){
    tone(pinBuzzer, 988,10000);
  }
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;

  //int recipient = LoRa.read();

  //while (LoRa.available()) {
  incoming = LoRa.read();
  pktNum1 = LoRa.read();
  //}

  //if (recipient != localAddress && recipient != 0xBB) {
    //Serial.println("This message is not for me.");
    //return;                            
  //}
  Serial.print("Packet Number: ");
  Serial.println(pktNum2);
  if (pktNum1 != pktNum2){
    if (incoming == 0){
      x =! x;
      Serial.println(x);
    }
  }
  pktNum2 = pktNum1;
  delay(1000);
}