#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

int button = 2;
int buttonState;
int value;
float humidity;
int pktNum = 0;
int pktNum1, pktNum2;
float temperature;
float heatIndex;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);

  if (!LoRa.begin(868E6)) {
    while (1);
  }

  delay(1000);
  pinMode(button, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void loop() {
  buttonState = digitalRead(button);

  if(buttonState == 0){
    LoRa.beginPacket();
    LoRa.write(buttonState);
    LoRa.write(pktNum);
    LoRa.endPacket();
    delay(500);
    pktNum++;
  }

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    onReceive(packetSize);
  }
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;

  if (packetSize >= 2 * sizeof(int) + 3 * sizeof(float)) {
      LoRa.readBytes((uint8_t*)&pktNum1, sizeof(pktNum1));
      LoRa.readBytes((uint8_t*)&value, sizeof(value));
      LoRa.readBytes((uint8_t*)&humidity, sizeof(humidity));
      LoRa.readBytes((uint8_t*)&temperature, sizeof(temperature));
      LoRa.readBytes((uint8_t*)&heatIndex, sizeof(heatIndex));
  }

  if(pktNum1 == pktNum2){
    return;
  }
  pktNum2 = pktNum1;

  unsigned long currentTime = millis()/1000;

  Serial.print(currentTime);
  Serial.print(";");
  Serial.print(value);
  Serial.print(";");
  Serial.print(humidity);
  Serial.print(";");
  Serial.print(temperature);
  Serial.print(";");
  Serial.println(heatIndex);

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("PPM:");
  lcd.print(value);

  lcd.setCursor(8,0);
  lcd.print("H:");
  lcd.print(humidity, 1);

  lcd.setCursor(0,1);
  lcd.print("T:");
  lcd.print(temperature, 1);

  lcd.setCursor(8,1);
  lcd.print("S:");
  lcd.print(heatIndex, 1);

  delay(500);
}
