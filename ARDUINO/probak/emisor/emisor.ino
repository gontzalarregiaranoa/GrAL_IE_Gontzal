#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// --- HARDWAREAREN KONFIGURAZIOA ---
#define DHTPIN 2        // DHT konektatzen den pin digitala (jarri 3 3. pina erabiltzen baduzu)
#define DHTTYPE DHT11   // Aldatu DHT22-ra zure sentsorea zuria bada
DHT dht(DHTPIN, DHTTYPE);

const int mq135Pin = A1; // Gaserako pin analogikoa (MKR-ak A1 erabiltzen du)
#define ledGreen 3       // LED berdea
#define ledYellow 4      // LED horia
#define ledRed 5         // LED gorria
#define buzzerPin 1      // Buzzerra

// --- DATU-EGITURA (DATA PACKET) ---
struct DataPacket {
  int packetId;
  int ppm;
  float humidity;
  float temperature;
  float heatIndex;
};

DataPacket packet;
int pktCounter = 0;
int counter = 0;
bool trafficLightOn = false;

void setup() {
  Serial.begin(9600);
  dht.begin(); // Garrantzitsua: sentsorea abiarazi

  if (!LoRa.begin(868E6)) {
    Serial.println("Errorea LoRa abiaraztean");
    while (1);
  }

  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // --- BIDALKETA LOGIKA (~6 segundotik behin gutxi gorabehera) ---
  if (counter >= 6000) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Sentsoreak huts egiten badu, 0.0 bidaltzen dugu erroreak ekiditeko
    if (isnan(h) || isnan(t)) {
      h = 0.0;
      t = 0.0;
      Serial.println("DHT irakurtzean hutsa (begiratu kableak)");
    }

    // Paketea bete
    packet.packetId = pktCounter++;
    packet.ppm = analogRead(mq135Pin);
    packet.humidity = h;
    packet.temperature = t;
    packet.heatIndex = dht.computeHeatIndex(t, h, false);

    // --- DEBUG OSOA ---
    Serial.print("Bidaltzen -> PPM: "); Serial.print(packet.ppm);
    Serial.print(" | Hum: "); Serial.print(packet.humidity);
    Serial.print("% | Temp: "); Serial.print(packet.temperature);
    Serial.print("C | Sens: "); Serial.println(packet.heatIndex);

    // Egitura osoa LoRa bidez bidali
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&packet, sizeof(packet));
    LoRa.endPacket();

    counter = 0;
    delay(1000);
  } else {
    counter++;
    // Zentralaren komandoak entzun (botoia)
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      int incoming = LoRa.read();
      if (incoming == 0) {
         trafficLightOn = !trafficLightOn;
      }
    }
  }

  // --- LOGIKA LOKALA (LEDAK ETA BUZZERRA) ---
  int currentReading = analogRead(mq135Pin);

  if (trafficLightOn) {
    if (currentReading <= 200) { digitalWrite(ledGreen, LOW); digitalWrite(ledYellow, LOW); digitalWrite(ledRed, LOW); }
    else if (currentReading <= 280) { digitalWrite(ledGreen, HIGH); digitalWrite(ledYellow, LOW); digitalWrite(ledRed, LOW); }
    else if (currentReading <= 400) { digitalWrite(ledGreen, LOW); digitalWrite(ledYellow, HIGH); digitalWrite(ledRed, LOW); }
    else { digitalWrite(ledGreen, LOW); digitalWrite(ledYellow, LOW); digitalWrite(ledRed, HIGH); }
  } else {
    digitalWrite(ledGreen, LOW); digitalWrite(ledYellow, LOW); digitalWrite(ledRed, LOW);
  }

  if (currentReading >= 700) tone(buzzerPin, 988);
  else noTone(buzzerPin);
}
