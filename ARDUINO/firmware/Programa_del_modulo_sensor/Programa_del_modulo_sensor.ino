#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// --- SENTSOREEN KONFIGURAZIOA ---
#define DHTPIN 2       // DHT11 dagoen pina
#define DHTTYPE DHT11  // Sentsore mota
DHT dht(DHTPIN, DHTTYPE);

// --- PINEN KONFIGURAZIOA ---
#define ledGreen 3    // LED berdea
#define ledYellow 4   // LED horia
#define ledRed 5      // LED gorria
#define buzzerPin 1   // Buzzerra

// MQ-135 irakurketarako aldagaia (A1 pin analogikoa)
// GARRANTZITSUA: MKR plaketan A1 erabili behar da, ez 1.
const int mq135Pin = A1;

String buttonPress = "button pressed";
bool trafficLightOn = false; // Semaforoaren egoera (urrunetik aktibatu/desaktibatu)

int airValue;   // Airearen balioa
int incoming;
int pktNum = 0;
int pktNum1, pktNum2;
int counter = 0;

void setup() {
  Serial.begin(9600);
  dht.begin(); // Tenperatura-sentsorea abiarazi

  delay(500);
  Serial.println("LoRa program starting");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // LEDak itzalita hasieratu
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledYellow, LOW);
  digitalWrite(ledRed, LOW);

  delay(500);
}

void loop() {

  // --- IRAKURKETA ETA BIDALKETA LOGIKA (X ziklotik behin) ---
  // OHARRA: 'trafficLightOn' true edo false den arabera, begizta abiadura
  // desberdinean doa. Datuak azkarregi edo astiroegi bidaltzen baditu, doitu 6000.
  if (counter >= 6000){

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // DHT-ak huts egin duen egiaztatu
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      // Ez dugu return egiten, gutxienez gasaren datua bidal dezan
      h = 0.0;
      t = 0.0;
    }

    // ZUZENKETA: A1 pinetik esplizituki irakurri
    airValue = analogRead(mq135Pin);

    Serial.print("Air-quality: ");
    Serial.print(airValue);
    Serial.println(" ppm");

    // Sentsazio termikoa kalkulatu (Heat Index)
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("C  Heat index: "));
    Serial.println(hic);

    // --- LORA BIDALKETA ---
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&pktNum, sizeof(pktNum));     // 1. Pakete-zenbakia
    LoRa.write((uint8_t*)&airValue, sizeof(airValue)); // 2. Airearen kalitatea (int)
    LoRa.write((uint8_t*)&h, sizeof(h));               // 3. Hezetasuna (float)
    LoRa.write((uint8_t*)&t, sizeof(t));               // 4. Tenperatura (float)
    LoRa.write((uint8_t*)&hic, sizeof(hic));           // 5. Sentsazio termikoa (float)
    LoRa.endPacket();

    pktNum++;
    counter = 0;
    delay(1000); // Bidali ondoren segundo bat itxaron
  }
  else{
    counter++;
    // Modulu zentralaren mezurik dagoen begiratu (LEDak piztu/itzaltzeko)
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        onReceive(packetSize);
    }
  }

  // --- SEMAFOROAREN LOGIKA ---
  if (trafficLightOn == true){
    // Balioa eguneratu, azkar aldatu bada ere
    airValue = analogRead(mq135Pin);

    if(airValue <= 200){
      digitalWrite(ledGreen, LOW);
      digitalWrite(ledYellow, LOW);
      digitalWrite(ledRed, LOW);
    }
    else if(airValue >= 201 && airValue <= 280){
      digitalWrite(ledGreen, HIGH); // Berdea
      digitalWrite(ledYellow, LOW);
      digitalWrite(ledRed, LOW);
    }
    else if(airValue >= 281 && airValue <= 400){
      digitalWrite(ledGreen, LOW);
      digitalWrite(ledYellow, HIGH); // Horia
      digitalWrite(ledRed, LOW);
    }
    else if(airValue >= 401){
      digitalWrite(ledGreen, LOW);
      digitalWrite(ledYellow, LOW);
      digitalWrite(ledRed, HIGH); // Gorria
    }

    delay(50); // Egonkortasunerako atzerapen txikia
  }
  else{
      // Dena itzalita 'trafficLightOn' false bada
      digitalWrite(ledGreen, LOW);
      digitalWrite(ledYellow, LOW);
      digitalWrite(ledRed, LOW);
  }

  // --- BUZZERRAREN LOGIKA ---
  // Airea oso txarra bada (>700), txistu egiten du 'trafficLightOn' edozein delarik
  // GARRANTZITSUA: hemen berriz analogRead, datu freskoa izateko
  if(analogRead(mq135Pin) >= 700){
    tone(buzzerPin, 988, 1000); // Segundo batez txistu egin
  } else {
    noTone(buzzerPin); // Isiltzen dela ziurtatu
  }
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;

  // Modulu zentralak bidaltzen duena irakurri
  incoming = LoRa.read(); // Botoiaren egoera
  pktNum1 = LoRa.read();  // Paketearen IDa

  // Pakete berria bada, 'trafficLightOn' egoera aldatu
  if (pktNum1 != pktNum2){
    // Zentralak 0 bidaltzen du botoia sakatzean (INPUT_PULLUP)
    if (incoming == 0){
      trafficLightOn = !trafficLightOn; // Egoera alderantzikatu (True <-> False)
      Serial.print("Semaforoa aldatuta: ");
      Serial.println(trafficLightOn);
    }
  }
  pktNum2 = pktNum1;
}
