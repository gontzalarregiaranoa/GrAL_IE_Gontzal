#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// --- CONFIGURACIÓN SENSORES ---
#define DHTPIN 2       // Pin donde está el DHT11
#define DHTTYPE DHT11  // Tipo de sensor
DHT dht(DHTPIN, DHTTYPE);

// --- CONFIGURACIÓN PINES ---
#define aire 3       // LED Verde
#define CO2 4        // LED Amarillo
#define gas 5        // LED Rojo
#define pinBuzzer 1  // Zumbador

// Variable para la lectura del MQ-135 (Pin Analógico A1)
// IMPORTANTE: En las MKR se debe usar A1, no 1.
const int pinMQ135 = A1; 

String buttonPress = "button pressed";
bool x = false; // Estado del semáforo (activado/desactivado remotamente)

int Value;      // Valor del aire
int incoming;
int pktNum = 0;
int pktNum1, pktNum2;
int counter = 0;  

void setup() {
  Serial.begin(9600);
  dht.begin(); // Iniciamos el sensor de temperatura

  delay(500);
  Serial.println("LoRa program starting");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  pinMode(aire, OUTPUT);
  pinMode(CO2, OUTPUT);
  pinMode(gas, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);
  
  // Inicializamos los LEDs apagados
  digitalWrite(aire, LOW);
  digitalWrite(CO2, LOW);
  digitalWrite(gas, LOW);
  
  delay(500);
}

void loop() {

  // --- LOGICA DE LECTURA Y ENVIO (Cada X ciclos) ---
  // NOTA: Dependiendo de si 'x' es true o false, el bucle va a diferente velocidad.
  // Si notas que envía datos demasiado rápido o lento, ajusta el 6000.
  if (counter >= 6000){
    
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    // Comprobamos si el DHT ha fallado
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      // No hacemos return para que al menos envíe el dato del gas
      h = 0.0;
      t = 0.0;
    }
    
    // CORRECCIÓN: Leemos del pin A1 explícitamente
    Value = analogRead(pinMQ135);
    
    Serial.print("Air-quality: ");
    Serial.print(Value);
    Serial.println(" ppm");
    
    // Calcular sensación térmica (Heat Index)
    float hic = dht.computeHeatIndex(t, h, false);
    
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("C  Heat index: "));
    Serial.println(hic);
    
    // --- ENVIO LORA ---
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&pktNum, sizeof(pktNum)); // 1. Número paquete
    LoRa.write((uint8_t*)&Value, sizeof(Value));   // 2. Calidad Aire (int)
    LoRa.write((uint8_t*)&h, sizeof(h));           // 3. Humedad (float)
    LoRa.write((uint8_t*)&t, sizeof(t));           // 4. Temperatura (float)
    LoRa.write((uint8_t*)&hic, sizeof(hic));       // 5. Sensación (float)
    LoRa.endPacket();
    
    pktNum++;
    counter = 0;
    delay(1000); // Espera 1 segundo tras enviar
  }
  else{
    counter++;
    // Miramos si hay mensajes del módulo central (para encender/apagar LEDs)
    int packetSize = LoRa.parsePacket();
    if (packetSize) { 
        onReceive(packetSize); 
    }
  }
  
  // --- LOGICA DEL SEMAFORO ---
  if (x == true){
    // Actualizamos el valor por si acaso ha cambiado rápido
    // (Opcional: puedes quitar este analogRead si quieres usar solo el de arriba)
    Value = analogRead(pinMQ135); 

    if(Value <= 200){
      digitalWrite(aire, LOW); // Quizás querías HIGH (Verde) aquí? Según tu lógica original era LOW.
      digitalWrite(CO2, LOW);
      digitalWrite(gas, LOW);
    }
    else if(Value >= 201 && Value <= 280){
      digitalWrite(aire, HIGH); // Verde
      digitalWrite(CO2, LOW);
      digitalWrite(gas, LOW);
    } 
    else if(Value >= 281 && Value <= 400){
      digitalWrite(aire, LOW);
      digitalWrite(CO2, HIGH); // Amarillo
      digitalWrite(gas, LOW);
    }
    else if(Value >= 401){
      digitalWrite(aire, LOW);
      digitalWrite(CO2, LOW);
      digitalWrite(gas, HIGH); // Rojo
    }
    
    delay(50); // Pequeño retardo para estabilidad
  }
  else{
      // Todo apagado si x es false
      digitalWrite(aire, LOW);
      digitalWrite(CO2, LOW);
      digitalWrite(gas, LOW);
  }

  // --- LOGICA DEL BUZZER ---
  // Si el aire es muy malo (>700), pita independientemente de 'x'
  // IMPORTANTE: analogRead aquí de nuevo para tener el dato fresco
  if(analogRead(pinMQ135) >= 700){
    tone(pinBuzzer, 988, 1000); // Pita 1 segundo, no 10 segundos (era mucho)
  } else {
    noTone(pinBuzzer); // Aseguramos que se calla
  }
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;

  // Leemos lo que manda el central
  incoming = LoRa.read(); // Estado del botón
  pktNum1 = LoRa.read();  // ID del paquete

  // Si es un paquete nuevo, cambiamos el estado de 'x'
  if (pktNum1 != pktNum2){
    // El central manda 0 cuando aprieta el botón (INPUT_PULLUP)
    if (incoming == 0){
      x = !x; // Invertimos estado (True <-> False)
      Serial.print("Semaforo cambiado a: ");
      Serial.println(x);
    }
  }
  pktNum2 = pktNum1;
}