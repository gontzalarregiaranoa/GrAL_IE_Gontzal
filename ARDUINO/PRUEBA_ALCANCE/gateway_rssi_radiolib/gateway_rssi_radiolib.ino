/* ============================================================
 *  PRUEBA DE ALCANCE LoRa  -  GATEWAY (receptor + RSSI)  con RadioLib
 *  ------------------------------------------------------------
 *  Version RadioLib del gateway. Recibe los paquetes del beacon y,
 *  por cada uno:
 *    - mide RSSI (dBm) y SNR (dB)  -> radio.getRSSI() / getSNR()
 *    - lo imprime por Serial en CSV:  paquete;rssi;snr
 *    - lo muestra en la pantalla LCD para leerlo en el campo.
 *
 *  Va conectado por USB al portatil (alimentacion + Serial Monitor).
 *
 *  >>> MKR WAN 1310: ver notas en el beacon (SPI1 a 200 kHz + reset
 *      del modulo Murata antes de radio.begin()). <<<
 *
 *  Libreria necesaria: "RadioLib" (jgromes).
 *  FREQ, SF, BW, CR, SYNCW deben ser IDENTICOS al beacon.
 * ============================================================ */

#include <RadioLib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- SX1276 interno del MKR WAN 1310 por SPI1 ---
SX1276 radio = new Module(LORA_IRQ_DUMB, LORA_IRQ, RADIOLIB_NC, RADIOLIB_NC,
                          SPI1, SPISettings(200000, MSBFIRST, SPI_MODE0));

LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Parametros de radio (IDENTICOS al beacon) ---
#define FREQ    868.0
#define SF      12       // 7 (Fase A) o 12 (Fase B). Igual que el beacon.
#define BW      125.0
#define CR      5
#define SYNCW   0x12
#define TXP     17
#define PREAMB  8

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

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Esperando LoRa..");

  Serial.println("paquete;rssi_dBm;snr_dB");   // cabecera CSV
}

void loop() {
  String paquete;
  int estado = radio.receive(paquete);   // bloqueante hasta paquete o timeout

  if (estado == RADIOLIB_ERR_NONE) {
    float rssi = radio.getRSSI();   // dBm (cuanto mas cerca de 0, mejor)
    float snr  = radio.getSNR();    // dB

    // --- Serial en CSV: paquete;rssi;snr ---
    Serial.print(paquete);   Serial.print(";");
    Serial.print(rssi, 0);   Serial.print(";");
    Serial.println(snr, 1);

    // --- LCD: lectura grande para el campo ---
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RSSI:"); lcd.print(rssi, 0); lcd.print("dBm");
    lcd.setCursor(0, 1);
    lcd.print("SNR:");  lcd.print(snr, 1);  lcd.print(" P"); lcd.print(paquete);

  } else if (estado == RADIOLIB_ERR_CRC_MISMATCH) {
    // Paquete recibido pero corrupto (estamos en el limite de sensibilidad)
    Serial.println("CRC_ERROR;;");
  }
  // RADIOLIB_ERR_RX_TIMEOUT: no llego nada en la ventana -> seguir escuchando
}
