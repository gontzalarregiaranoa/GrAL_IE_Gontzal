#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// --- CONFIGURACIÓN HARDWARE ---
#define DHTPIN 2        // Pin digital donde conectas el DHT (cambia a 3 si usas el pin 3)
#define DHTTYPE DHT11   // Cambia a DHT22 si tu sensor es blanco
DHT dht(DHTPIN, DHTTYPE);

const int pinMQ135 = A1; // Pin analógico para el gas (MKR usa A1)
#define aire 3           // LED Verde
#define CO2 4            // LED Amarillo
#define gas 5            // LED Rojo
#define pinBuzzer 1      // Zumbador

// --- ESTRUCTURA DE DATOS (DATA PACKET) ---
struct DataPacket {
  int id_paquete;
  int ppm;
  float humedad;
  float temperatura;
  float sensacion;
};

DataPacket miPaquete; 
int pktCounter = 0;
int counter = 0;
bool semaforoActivo = false;

void setup() {
  Serial.begin(9600);
  dht.begin(); // Importante: iniciar sensor
  
  if (!LoRa.begin(868E6)) {
    Serial.println("Error iniciando LoRa");
    while (1);
  }
  
  pinMode(aire, OUTPUT);
  pinMode(CO2, OUTPUT);
  pinMode(gas, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);
}

void loop() {
  // --- LÓGICA DE ENVÍO (Cada ~6 segundos aprox) ---
  if (counter >= 6000) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    // Si el sensor falla, enviamos 0.0 para evitar errores
    if (isnan(h) || isnan(t)) { 
      h = 0.0; 
      t = 0.0; 
      Serial.println("Fallo leyendo DHT (Revisa cables)");
    }
    
    // Rellenamos el paquete
    miPaquete.id_paquete = pktCounter++;
    miPaquete.ppm = analogRead(pinMQ135);
    miPaquete.humedad = h;
    miPaquete.temperatura = t;
    miPaquete.sensacion = dht.computeHeatIndex(t, h, false);

    // --- DEBUG COMPLETO ---
    Serial.print("Enviando -> PPM: "); Serial.print(miPaquete.ppm);
    Serial.print(" | Hum: "); Serial.print(miPaquete.humedad);     
    Serial.print("% | Temp: "); Serial.print(miPaquete.temperatura);
    Serial.print("C | Sens: "); Serial.println(miPaquete.sensacion); 

    // Enviamos la estructura completa por LoRa
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&miPaquete, sizeof(miPaquete));
    LoRa.endPacket();
    
    counter = 0;
    delay(1000);
  } else {
    counter++;
    // Escuchar comandos del central (botón)
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      int incoming = LoRa.read();
      if (incoming == 0) { 
         semaforoActivo = !semaforoActivo;
      }
    }
  }

  // --- LÓGICA LOCAL (LEDS Y BUZZER) ---
  int lecturaActual = analogRead(pinMQ135);
  
  if (semaforoActivo) {
    if (lecturaActual <= 200) { digitalWrite(aire, LOW); digitalWrite(CO2, LOW); digitalWrite(gas, LOW); }
    else if (lecturaActual <= 280) { digitalWrite(aire, HIGH); digitalWrite(CO2, LOW); digitalWrite(gas, LOW); }
    else if (lecturaActual <= 400) { digitalWrite(aire, LOW); digitalWrite(CO2, HIGH); digitalWrite(gas, LOW); }
    else { digitalWrite(aire, LOW); digitalWrite(CO2, LOW); digitalWrite(gas, HIGH); }
  } else {
    digitalWrite(aire, LOW); digitalWrite(CO2, LOW); digitalWrite(gas, LOW);
  }
  
  if (lecturaActual >= 700) tone(pinBuzzer, 988);
  else noTone(pinBuzzer);
}