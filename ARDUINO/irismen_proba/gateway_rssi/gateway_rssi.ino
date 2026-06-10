/* ============================================================
 *  PRUEBA DE ALCANCE LoRa  -  SKETCH DEL GATEWAY (receptor + RSSI)
 *  ------------------------------------------------------------
 *  Este sketch es SOLO para la prueba de alcance. NO sustituye a
 *  tu firmware real (Programa_del_modulo_central).
 *
 *  Que hace: recibe los paquetes del beacon y, por cada uno,
 *    - mide RSSI (potencia, dBm) y SNR (relacion senal/ruido, dB)
 *    - lo imprime por Serial en formato CSV: paquete;rssi;snr
 *    - lo muestra en la pantalla LCD para que lo leas en el campo
 *      sin tener que mirar el portatil.
 *
 *  Este modulo va conectado por USB al portatil (alimentacion +
 *  Serial Monitor). El LCD te da la lectura de un vistazo.
 *
 *  IMPORTANTE: FREQ, SF, BW y CR deben ser IDENTICOS al beacon.
 * ============================================================ */

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// === Parametros de radio (deben coincidir con el beacon del nodo) ===
const long FREQ = 868E6;
const int  SF   = 7;       // Fase 2: cambiar a 12 aqui y en el beacon
const long BW   = 125E3;
const int  CR   = 5;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  delay(1000);

  if (!LoRa.begin(FREQ)) {
    Serial.println("ERROR: no se pudo iniciar LoRa");
    while (1);
  }
  LoRa.setSpreadingFactor(SF);
  LoRa.setSignalBandwidth(BW);
  LoRa.setCodingRate4(CR);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Esperando LoRa..");

  // Cabecera CSV (util para luego copiar/pegar y graficar)
  Serial.println("paquete;rssi_dBm;snr_dB");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Leemos el numero de paquete que manda el beacon
    String paquete = "";
    while (LoRa.available()) {
      paquete += (char)LoRa.read();
    }

    int   rssi = LoRa.packetRssi();   // dBm (cuanto mas cerca de 0, mejor)
    float snr  = LoRa.packetSnr();    // dB

    // --- Serial en CSV: paquete;rssi;snr ---
    Serial.print(paquete);
    Serial.print(";");
    Serial.print(rssi);
    Serial.print(";");
    Serial.println(snr, 1);

    // --- LCD: lectura grande para el campo ---
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RSSI:");
    lcd.print(rssi);
    lcd.print("dBm");

    lcd.setCursor(0, 1);
    lcd.print("SNR:");
    lcd.print(snr, 1);
    lcd.print(" P");
    lcd.print(paquete);
  }
}
