/* ============================================================
 *  LoRa IRISMEN-PROBA  -  NODOAREN SKETCHA (igorlea / beacon)
 *  ------------------------------------------------------------
 *  Sketch hau irismen-probarako BAKARRIK da. EZ du ordezkatzen
 *  benetako firmwarea (Programa_del_modulo_sensor).
 *
 *  Zer egiten du: 2 segundotik behin pakete bat igortzen du,
 *  gorantz doan pakete-zenbaki batekin (0, 1, 2, ...). Atebideak
 *  zenbaki hori erabiliko du distantzia bakoitzean zenbat pakete
 *  galtzen diren jakiteko.
 *
 *  GARRANTZITSUA: irrati-parametroek (FREQ, SF, BW, CR) atebidearen
 *  BERDINAK izan behar dute, bestela ez dira komunikatuko.
 *
 *  2. Faserako (SF7 vs SF12 azterketa): aldatu SF = 7 SF = 12-ra
 *  HEMEN eta atebidean, eta errepikatu neurketak.
 * ============================================================ */

#include <SPI.h>
#include <LoRa.h>

// === Irrati-parametroak (atebidearekin bat etorri behar dute) ===
const long FREQ = 868E6;   // 868 MHz maiztasuna (Europako banda)
const int  SF   = 7;       // Spreading Factor (7..12). 2. fasea: probatu 12
const long BW   = 125E3;   // 125 kHz banda-zabalera
const int  CR   = 5;       // Coding rate -> 4/5
const int  TXP  = 17;      // Igorpen-potentzia (dBm). Benetako sistemaren berdina

const unsigned long INTERVAL = 2000;  // ms paketeen artean
long counter = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("Nodoaren BEACON - LoRa irismen-proba");

  if (!LoRa.begin(FREQ)) {
    Serial.println("ERROR: ezin izan da LoRa abiarazi");
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
  // Pakete-zenbakia bakarrik bidaltzen dugu testu gisa
  LoRa.beginPacket();
  LoRa.print(counter);
  LoRa.endPacket();

  Serial.print("Bidalitako paketea #");
  Serial.println(counter);

  counter++;
  delay(INTERVAL);
}
