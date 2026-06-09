#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

int button = 2;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- MISMA ESTRUCTURA QUE EN EL EMISOR ---
struct DataPacket {
  int id_paquete;
  int ppm;
  float humedad;
  float temperatura;
  float sensacion;
};

DataPacket datoRecibido;
int ultimoID = -1; // Para no repetir datos

void setup() {
  Serial.begin(9600);
  
  if (!LoRa.begin(868E6)) {
    while (1);
  }
  
  pinMode(button, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Esperando datos...");
}

void loop() {
  // Lógica del botón (enviar comando al sensor)
  if (digitalRead(button) == 0) {
    LoRa.beginPacket();
    LoRa.write(0); // Enviamos un 0 simple
    LoRa.endPacket();
    delay(500); // Anti-rebote
  }
  
  // Recepción
  int packetSize = LoRa.parsePacket();
  if (packetSize == sizeof(DataPacket)) { // ¡Solo leemos si el tamaño es EXACTO!
    
    // Leemos todo el bloque de golpe en la estructura
    LoRa.readBytes((uint8_t*)&datoRecibido, sizeof(datoRecibido));

    // Si es un paquete nuevo, lo procesamos
    if (datoRecibido.id_paquete != ultimoID) {
      ultimoID = datoRecibido.id_paquete;
      procesarDatos();
    }
  }
}

void procesarDatos() {
  unsigned long tiempo_actual = millis() / 1000;

  // Salida Serie para Python
  Serial.print(tiempo_actual);
  Serial.print(";");
  Serial.print(datoRecibido.ppm);
  Serial.print(";");
  Serial.print(datoRecibido.humedad);
  Serial.print(";");
  Serial.print(datoRecibido.temperatura);
  Serial.print(";");
  Serial.println(datoRecibido.sensacion);

  // Salida LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("PPM:"); lcd.print(datoRecibido.ppm);
  
  lcd.setCursor(8,0);
  lcd.print("H:"); lcd.print(datoRecibido.humedad, 1);

  lcd.setCursor(0,1);
  lcd.print("T:"); lcd.print(datoRecibido.temperatura, 1);
  
  lcd.setCursor(8,1);
  lcd.print("S:"); lcd.print(datoRecibido.sensacion, 1);
}