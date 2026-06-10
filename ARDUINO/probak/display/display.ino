#include <SPI.h>
#include <LoRa.h>
  //LCD-rako
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

byte localAddress = 0xBB;  
byte destination = 0xFF;
int button = 2;
int buttonState;
int value;
float humedad;
int pn = 0;
int pnt1, pnt2;
float temperatura;
float senstermica;

  //LCD-rako
LiquidCrystal_I2C lcd (0x27, 2, 1, 0, 4, 5, 6, 7);  //kobertsiorako 

void setup() {

  Serial.begin(9600);
  Serial.println("LoRa message service");
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  delay(1000);
  
  pinMode(button, INPUT_PULLUP);

      //LCD-rako
  lcd.setBacklightPin(3, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.begin(16, 2);
  lcd.clear();

}

void loop() {
  buttonState = digitalRead(button);
  if(buttonState == 0){
  LoRa.beginPacket();
  LoRa.write(buttonState);
  LoRa.write(pn);
  LoRa.endPacket();
  delay(500);
  pn++;
  //Serial.print(buttonState);
  //PacketNum++;
  //Serial.println(".,.");
  //count=0;
  }


  //count++;

  
  onReceive(LoRa.parsePacket());
  

    //Serial.println("Peter: " + message); //name seen in the Serial Monitor
    //LoRa.beginPacket();
    //LoRa.write(120);              
    //LoRa.endPacket();



}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  //while (LoRa.available()) { 

  //LoRa.beginPacket();
  //LoRa.print("Neurketak hasi:");
  //LoRa.endPacket();
    //int packetSize = LoRa.parsePacket();

    if (packetSize >= 2 * sizeof(int) + 3 * sizeof(float)) {
      LoRa.readBytes((uint8_t*)&pnt1, sizeof(pnt1));
      LoRa.readBytes((uint8_t*)&value, sizeof(value));
      LoRa.readBytes((uint8_t*)&humedad, sizeof(humedad));
      LoRa.readBytes((uint8_t*)&temperatura, sizeof(temperatura));
      LoRa.readBytes((uint8_t*)&senstermica, sizeof(senstermica));
    }
    Serial.println(pnt1);
    if(pnt1==pnt2){
      return;
    }
    pnt2=pnt1;

    if(value<=55){
      Serial.print("Aire normal y el valor es de ");
      Serial.println(value);
      Serial.print("Humedad ");
      Serial.println(humedad);
      Serial.print("Temperatura ");
      Serial.println(temperatura);
      Serial.print("Sensación térmica ");
      Serial.println(senstermica);

      lcd.setCursor(0,0);
      lcd.print("N:");
      lcd.print(value);
      lcd.setCursor(8,0);
      lcd.print("H:");
      lcd.print(humedad);
      lcd.setCursor(0,1);
      lcd.print("T:");
      lcd.print(temperatura);
      lcd.setCursor(8,1);
      lcd.print("S:");
      lcd.print(senstermica);


    delay(500);
    }


    if(value>=56 && value<=65){
      Serial.print("Aire con un poco de CO2 y el valor es de ");
      Serial.println(value);
      Serial.print("Humedad ");
      Serial.println(humedad);
      Serial.print("Temperatura ");
      Serial.println(temperatura);
      Serial.print("Sensación térmica ");
      Serial.println(senstermica);

      lcd.setCursor(0,0);
      lcd.print("N:");
      lcd.print(value);
      lcd.setCursor(8,0);
      lcd.print("H:");
      lcd.print(humedad);
      lcd.setCursor(0,1);
      lcd.print("T:");
      lcd.print(temperatura);
      lcd.setCursor(8,1);
      lcd.print("S:");
      lcd.print(senstermica);

    delay(500);
    } 

    if(value>=66 && value<=350){
      Serial.print("Dioxido de carbono y el valor es de ");
      Serial.println(value);
      Serial.print("Humedad ");
      Serial.println(humedad);
      Serial.print("Temperatura ");
      Serial.println(temperatura);
      Serial.print("Sensación térmica ");
      Serial.println(senstermica);

      lcd.setCursor(0,0);
      lcd.print("N:");
      lcd.print(value);
      lcd.setCursor(8,0);
      lcd.print("H:");
      lcd.print(humedad);
      lcd.setCursor(0,1);
      lcd.print("T:");
      lcd.print(temperatura);
      lcd.setCursor(8,1);
      lcd.print("S:");
      lcd.print(senstermica);

    delay(500);
    }

    if(value>=351){
      Serial.print("Propano y Butano y el valor es de ");
      Serial.println(value);
      Serial.print("Humedad ");
      Serial.println(humedad);
      Serial.print("Temperatura ");
      Serial.println(temperatura);
      Serial.print("Sensación térmica ");
      Serial.println(senstermica);

      lcd.setCursor(0,0);
      lcd.print("N:");
      lcd.print(value);
      lcd.setCursor(8,0);
      lcd.print("H:");
      lcd.print(humedad);
      lcd.setCursor(0,1);
      lcd.print("T:");
      lcd.print(temperatura);
      lcd.setCursor(8,1);
      lcd.print("S:");
      lcd.print(senstermica);

    delay(500);
    }
  //}

  
}