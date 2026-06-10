#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

int button = 2;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- IGORLEAREN EGITURA BERA ---
struct DataPacket {
  int packetId;
  int ppm;
  float humidity;
  float temperature;
  float heatIndex;
};

DataPacket received;
int lastId = -1; // Datuak ez errepikatzeko

void setup() {
  Serial.begin(9600);

  if (!LoRa.begin(868E6)) {
    while (1);
  }

  pinMode(button, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Datuen zain...");
}

void loop() {
  // Botoiaren logika (komandoa sentsoreari bidali)
  if (digitalRead(button) == 0) {
    LoRa.beginPacket();
    LoRa.write(0); // 0 sinple bat bidaltzen dugu
    LoRa.endPacket();
    delay(500); // Errebote-aurkakoa
  }

  // Harrera
  int packetSize = LoRa.parsePacket();
  if (packetSize == sizeof(DataPacket)) { // Tamaina ZEHATZA bada bakarrik irakurtzen dugu!

    // Bloke osoa kolpe batean irakurri egituran
    LoRa.readBytes((uint8_t*)&received, sizeof(received));

    // Pakete berria bada, prozesatu egiten dugu
    if (received.packetId != lastId) {
      lastId = received.packetId;
      processData();
    }
  }
}

void processData() {
  unsigned long currentTime = millis() / 1000;

  // Serie irteera Python-erako
  Serial.print(currentTime);
  Serial.print(";");
  Serial.print(received.ppm);
  Serial.print(";");
  Serial.print(received.humidity);
  Serial.print(";");
  Serial.print(received.temperature);
  Serial.print(";");
  Serial.println(received.heatIndex);

  // LCD irteera
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("PPM:"); lcd.print(received.ppm);

  lcd.setCursor(8,0);
  lcd.print("H:"); lcd.print(received.humidity, 1);

  lcd.setCursor(0,1);
  lcd.print("T:"); lcd.print(received.temperature, 1);

  lcd.setCursor(8,1);
  lcd.print("S:"); lcd.print(received.heatIndex, 1);
}
