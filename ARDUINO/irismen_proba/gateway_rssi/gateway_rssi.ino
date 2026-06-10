/* ============================================================
 *  LoRa IRISMEN-PROBA  -  ATEBIDEAREN SKETCHA (hartzailea + RSSI)
 *  ------------------------------------------------------------
 *  Sketch hau irismen-probarako BAKARRIK da. EZ du ordezkatzen
 *  benetako firmwarea (Programa_del_modulo_central).
 *
 *  Zer egiten du: beacon-aren paketeak jaso eta, bakoitzeko,
 *    - RSSI (potentzia, dBm) eta SNR (seinale/zarata erlazioa, dB) neurtu
 *    - Serie bidez CSV formatuan inprimatu: paketea;rssi;snr
 *    - LCD pantailan erakutsi, landan ordenagailura begiratu gabe
 *      irakurtzeko.
 *
 *  Modulu hau USB bidez konektatuta doa ordenagailura (elikadura +
 *  Serial Monitor). LCD-ak begi kolpe batean ematen du irakurketa.
 *
 *  GARRANTZITSUA: FREQ, SF, BW eta CR beacon-aren BERDINAK izan behar dute.
 * ============================================================ */

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// === Irrati-parametroak (nodoaren beacon-arekin bat etorri behar dute) ===
const long FREQ = 868E6;
const int  SF   = 7;       // 2. fasea: 12-ra aldatu hemen eta beacon-ean
const long BW   = 125E3;
const int  CR   = 5;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  delay(1000);

  if (!LoRa.begin(FREQ)) {
    Serial.println("ERROR: ezin izan da LoRa abiarazi");
    while (1);
  }
  LoRa.setSpreadingFactor(SF);
  LoRa.setSignalBandwidth(BW);
  LoRa.setCodingRate4(CR);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("LoRa zain...");

  // CSV goiburua (gero kopiatu/itsatsi eta grafikatzeko baliagarria)
  Serial.println("paketea;rssi_dBm;snr_dB");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Beacon-ak bidaltzen duen pakete-zenbakia irakurtzen dugu
    String packet = "";
    while (LoRa.available()) {
      packet += (char)LoRa.read();
    }

    int   rssi = LoRa.packetRssi();   // dBm (0-tik zenbat eta hurbilago, hobeto)
    float snr  = LoRa.packetSnr();    // dB

    // --- Serie CSV-an: paketea;rssi;snr ---
    Serial.print(packet);
    Serial.print(";");
    Serial.print(rssi);
    Serial.print(";");
    Serial.println(snr, 1);

    // --- LCD: irakurketa handia landarako ---
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RSSI:");
    lcd.print(rssi);
    lcd.print("dBm");

    lcd.setCursor(0, 1);
    lcd.print("SNR:");
    lcd.print(snr, 1);
    lcd.print(" P");
    lcd.print(packet);
  }
}
