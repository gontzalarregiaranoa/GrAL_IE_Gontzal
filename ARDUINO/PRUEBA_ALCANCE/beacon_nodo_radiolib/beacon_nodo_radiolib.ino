/* ============================================================
 *  PRUEBA DE ALCANCE LoRa  -  BEACON (transmisor)  con RadioLib
 *  ------------------------------------------------------------
 *  Version RadioLib del beacon, para el estudio del Spreading
 *  Factor (SF7 vs SF12) y para honrar la sugerencia del profesor
 *  de integrar la libreria RadioLib (https://github.com/jgromes/RadioLib).
 *
 *  Que hace: emite un paquete cada INTERVALO ms con un contador
 *  incremental (0,1,2,...). El gateway usa ese numero para contar
 *  los paquetes que se pierden a cada distancia.
 *
 *  >>> IMPORTANTE (MKR WAN 1310) <<<
 *  El SX1276 esta DENTRO del modulo Murata. Para hablar con el
 *  directamente hay que: (1) usar SPI1 a 200 kHz (a 2 MHz NO va),
 *  (2) poner el modulo en modo "passthrough" con la secuencia de
 *  reset de mkrwan_reset() ANTES de radio.begin().
 *
 *  Libreria necesaria: "RadioLib" (jgromes) desde el Library Manager.
 *
 *  FASE A: SF = 7   (reproduce la prueba original)
 *  FASE B: SF = 12  (mayor sensibilidad -> mayor alcance)
 *  Cambia SF aqui Y en el gateway; deben ser identicos.
 * ============================================================ */

#include <RadioLib.h>

// --- SX1276 interno del MKR WAN 1310 (Murata) por SPI1 ---
//   CS  = LORA_IRQ_DUMB (28)   DIO0 = LORA_IRQ (31)
//   RST = se hace a mano (RADIOLIB_NC)   DIO1 = RADIOLIB_NC
SX1276 radio = new Module(LORA_IRQ_DUMB, LORA_IRQ, RADIOLIB_NC, RADIOLIB_NC,
                          SPI1, SPISettings(200000, MSBFIRST, SPI_MODE0));

// --- Parametros de radio (IDENTICOS en beacon y gateway) ---
#define FREQ    868.0    // MHz (banda Europa)
#define SF      12       // 7 (Fase A) o 12 (Fase B)
#define BW      125.0    // kHz
#define CR      5        // coding rate 4/5
#define SYNCW   0x12     // sync word (red privada)
#define TXP     17       // potencia TX (dBm)
#define PREAMB  8        // longitud de preambulo

// A SF12 un paquete tarda ~1 s en el aire: deja intervalo amplio
// (ademas, respeta mejor el duty cycle del 1% de la banda 868).
#if SF >= 11
  #define INTERVALO 5000
#else
  #define INTERVALO 2000
#endif

long contador = 0;

// Secuencia de reset del modulo Murata del MKR WAN 1310/1300
void mkrwan_reset() {
  pinMode(LORA_IRQ_DUMB, OUTPUT);
  digitalWrite(LORA_IRQ_DUMB, LOW);
  pinMode(LORA_BOOT0, OUTPUT);
  digitalWrite(LORA_BOOT0, LOW);
  pinMode(LORA_RESET, OUTPUT);
  digitalWrite(LORA_RESET, HIGH); delay(200);
  digitalWrite(LORA_RESET, LOW);  delay(200);
  digitalWrite(LORA_RESET, HIGH); delay(50);
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  mkrwan_reset();
  SPI1.begin();

  int estado = radio.begin(FREQ, BW, SF, CR, SYNCW, TXP, PREAMB);
  if (estado != RADIOLIB_ERR_NONE) {
    Serial.print("ERROR: RadioLib begin fallo, codigo ");
    Serial.println(estado);
    while (true);
  }

  Serial.print("BEACON RadioLib OK -> SF="); Serial.print(SF);
  Serial.print(" BW="); Serial.print(BW);
  Serial.print(" CR=4/"); Serial.print(CR);
  Serial.print(" TX="); Serial.print(TXP); Serial.println("dBm");
}

void loop() {
  int estado = radio.transmit(String(contador));

  if (estado == RADIOLIB_ERR_NONE) {
    Serial.print("Enviado paquete #");
    Serial.println(contador);
  } else {
    Serial.print("TX error: ");
    Serial.println(estado);
  }

  contador++;
  delay(INTERVALO);
}
