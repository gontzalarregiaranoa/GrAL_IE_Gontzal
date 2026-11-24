#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Agindu-kodeak (sentsore-moduluaren programarekin eta Python atebidearekin bat)
#define CMD_TOGGLE 0   // semaforoaren egoera aldatu (botoia)
#define CMD_ON     1   // semaforoa piztu
#define CMD_OFF    2   // semaforoa itzali

int button = 2;
int buttonState;
int value;
float humidity;
int pktNum = 0;
int pktNum1, pktNum2;
float temperature;
float heatIndex;

// Serie-atakatik (Python atebidetik) jasotako aginduen bufferra: "CMD:<kodea>\n"
char cmdBuf[16];
byte cmdLen = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);

  if (!LoRa.begin(868E6)) {
    while (1);
  }
  // CRCa gaitu: errorez jasotako paketeak baztertzen dira
  LoRa.enableCrc();

  delay(1000);
  pinMode(button, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.clear();
}

// Agindu bat LoRa bidez sentsore-modulura bidali: [kodea, agindu-zenbakia]
void sendCommand(uint8_t code) {
  LoRa.beginPacket();
  LoRa.write(code);
  LoRa.write(pktNum);
  LoRa.endPacket();
  pktNum++;
}

// Serie-atakako karaktereak blokeatu gabe irakurri; lerro osoa iristean, interpretatu
void readSerialCommands() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      cmdBuf[cmdLen] = '\0';
      if (strncmp(cmdBuf, "CMD:", 4) == 0) {
        int code = atoi(cmdBuf + 4);
        if (code >= CMD_TOGGLE && code <= CMD_OFF) {
          sendCommand(code);
        }
      }
      cmdLen = 0;
    } else if (c != '\r' && cmdLen < sizeof(cmdBuf) - 1) {
      cmdBuf[cmdLen++] = c;
    }
  }
}

void loop() {
  buttonState = digitalRead(button);

  // Botoia sakatuta (INPUT_PULLUP -> 0): semaforoaren egoera aldatu
  if(buttonState == 0){
    sendCommand(CMD_TOGGLE);
    delay(500);
  }

  // Web/MQTT bidezko aginduak (Python atebidea -> serie-ataka)
  readSerialCommands();

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    onReceive(packetSize);
  }
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;

  // Sentsore-moduluaren datu-paketeak bakarrik (20 byte); bestela, baztertu
  if (packetSize < 2 * sizeof(int) + 3 * sizeof(float)) return;

  LoRa.readBytes((uint8_t*)&pktNum1, sizeof(pktNum1));
  LoRa.readBytes((uint8_t*)&value, sizeof(value));
  LoRa.readBytes((uint8_t*)&humidity, sizeof(humidity));
  LoRa.readBytes((uint8_t*)&temperature, sizeof(temperature));
  LoRa.readBytes((uint8_t*)&heatIndex, sizeof(heatIndex));

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
}
