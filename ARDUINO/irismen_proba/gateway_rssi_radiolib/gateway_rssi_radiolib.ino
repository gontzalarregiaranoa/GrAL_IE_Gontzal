/* ============================================================
 *  LoRa IRISMEN-PROBA  -  ATEBIDEA (hartzailea + RSSI)  RadioLib-ekin
 *  ------------------------------------------------------------
 *  Atebidearen RadioLib bertsioa. Beacon-aren paketeak jasotzen ditu
 *  eta, bakoitzeko:
 *    - RSSI (dBm) eta SNR (dB) neurtzen ditu -> radio.getRSSI() / getSNR()
 *    - Serie bidez CSV formatuan inprimatzen du: paketea;rssi;snr
 *    - LCD pantailan erakusten du landan irakurtzeko.
 *
 *  USB bidez konektatuta doa ordenagailura (elikadura + Serial Monitor).
 *
 *  >>> MKR WAN 1310: ikus beacon-eko oharrak (SPI1 200 kHz-tan + Murata
 *      moduluaren reset-a radio.begin() baino lehen). <<<
 *
 *  Beharrezko liburutegia: "RadioLib" (jgromes).
 *  FREQ, SF, BW, CR, SYNCW beacon-aren BERDINAK izan behar dute.
 * ============================================================ */

#include <RadioLib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- MKR WAN 1310-eko SX1276 barnekoa SPI1 bidez ---
SX1276 radio = new Module(LORA_IRQ_DUMB, LORA_IRQ, RADIOLIB_NC, RADIOLIB_NC,
                          SPI1, SPISettings(200000, MSBFIRST, SPI_MODE0));

LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Irrati-parametroak (beacon-aren BERDINAK) ---
#define FREQ    868.0
#define SF      12       // 7 (A fasea) edo 12 (B fasea). Beacon-aren berdina.
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

  int state = radio.begin(FREQ, BW, SF, CR, SYNCW, TXP, PREAMB);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("ERROR: RadioLib begin huts egin du, kodea ");
    Serial.println(state);
    while (true);
  }

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("LoRa zain...");

  Serial.println("paketea;rssi_dBm;snr_dB");   // CSV goiburua
}

void loop() {
  String packet;
  int state = radio.receive(packet);   // blokeatzailea, paketea edo timeout arte

  if (state == RADIOLIB_ERR_NONE) {
    float rssi = radio.getRSSI();   // dBm (0-tik zenbat eta hurbilago, hobeto)
    float snr  = radio.getSNR();    // dB

    // --- Serie CSV-an: paketea;rssi;snr ---
    Serial.print(packet);    Serial.print(";");
    Serial.print(rssi, 0);   Serial.print(";");
    Serial.println(snr, 1);

    // --- LCD: irakurketa handia landarako ---
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RSSI:"); lcd.print(rssi, 0); lcd.print("dBm");
    lcd.setCursor(0, 1);
    lcd.print("SNR:");  lcd.print(snr, 1);  lcd.print(" P"); lcd.print(packet);

  } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
    // Paketea jaso da baina hondatuta (sentsibilitatearen mugan gaude)
    Serial.println("CRC_ERROR;;");
  }
  // RADIOLIB_ERR_RX_TIMEOUT: ez da ezer iritsi leihoan -> entzuten jarraitu
}
