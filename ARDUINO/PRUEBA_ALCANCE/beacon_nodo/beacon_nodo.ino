/* ============================================================
 *  PRUEBA DE ALCANCE LoRa  -  SKETCH DEL NODO (transmisor / beacon)
 *  ------------------------------------------------------------
 *  Este sketch es SOLO para la prueba de alcance. NO sustituye a
 *  tu firmware real (Programa_del_modulo_sensor).
 *
 *  Que hace: emite un paquete cada 2 segundos con un numero de
 *  paquete que va subiendo (0, 1, 2, ...). El gateway usara ese
 *  numero para saber cuantos paquetes se pierden a cada distancia.
 *
 *  IMPORTANTE: los parametros de radio (FREQ, SF, BW, CR) tienen
 *  que ser IDENTICOS a los del gateway, o no se comunicaran.
 *
 *  Para la Fase 2 (estudio SF7 vs SF12): cambia SF = 7 por SF = 12
 *  AQUI y en el gateway, y repite las medidas.
 * ============================================================ */

#include <SPI.h>
#include <LoRa.h>

// === Parametros de radio (deben coincidir con el gateway) ===
const long FREQ = 868E6;   // Frecuencia 868 MHz (banda Europa)
const int  SF   = 7;       // Spreading Factor (7..12). Fase 2: probar 12
const long BW   = 125E3;   // Ancho de banda 125 kHz
const int  CR   = 5;       // Coding rate -> 4/5
const int  TXP  = 17;      // Potencia de transmision (dBm). Igual que tu sistema real

const unsigned long INTERVALO = 2000;  // ms entre paquetes
long contador = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("BEACON del nodo - prueba de alcance LoRa");

  if (!LoRa.begin(FREQ)) {
    Serial.println("ERROR: no se pudo iniciar LoRa");
    while (1);
  }

  LoRa.setSpreadingFactor(SF);
  LoRa.setSignalBandwidth(BW);
  LoRa.setCodingRate4(CR);
  LoRa.setTxPower(TXP);

  Serial.print("Config: SF="); Serial.print(SF);
  Serial.print(" BW="); Serial.print(BW);
  Serial.print(" CR=4/"); Serial.print(CR);
  Serial.print(" TX="); Serial.print(TXP); Serial.println("dBm");
}

void loop() {
  // Enviamos solo el numero de paquete como texto
  LoRa.beginPacket();
  LoRa.print(contador);
  LoRa.endPacket();

  Serial.print("Enviado paquete #");
  Serial.println(contador);

  contador++;
  delay(INTERVALO);
}
