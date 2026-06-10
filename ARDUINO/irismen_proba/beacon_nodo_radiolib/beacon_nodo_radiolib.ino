/* ============================================================
 *  LoRa IRISMEN-PROBA  -  BEACON (igorlea)  RadioLib-ekin
 *  ------------------------------------------------------------
 *  Beacon-aren RadioLib bertsioa, Spreading Factor-aren azterketarako
 *  (SF7 vs SF12) eta irakaslearen iradokizunari jarraituz RadioLib
 *  liburutegia integratzeko (https://github.com/jgromes/RadioLib).
 *
 *  Zer egiten du: INTERVAL ms-tik behin pakete bat igortzen du,
 *  kontagailu gorakor batekin (0,1,2,...). Atebideak zenbaki hori
 *  erabiltzen du distantzia bakoitzean galtzen diren paketeak zenbatzeko.
 *
 *  >>> GARRANTZITSUA (MKR WAN 1310) <<<
 *  SX1276 txipa Murata moduluaren BARRUAN dago. Berarekin zuzenean
 *  komunikatzeko: (1) SPI1 erabili 200 kHz-tan (2 MHz-tan EZ doa),
 *  (2) modulua "passthrough" moduan jarri mkrwan_reset() reset-sekuentziarekin
 *  radio.begin() baino LEHEN.
 *
 *  Beharrezko liburutegia: "RadioLib" (jgromes), Library Manager-etik.
 *
 *  A FASEA: SF = 7   (jatorrizko proba errepikatzen du)
 *  B FASEA: SF = 12  (sentsibilitate handiagoa -> irismen handiagoa)
 *  Aldatu SF hemen ETA atebidean; berdinak izan behar dute.
 * ============================================================ */

#include <RadioLib.h>

// --- MKR WAN 1310-eko SX1276 barnekoa (Murata) SPI1 bidez ---
//   CS  = LORA_IRQ_DUMB (28)   DIO0 = LORA_IRQ (31)
//   RST = eskuz egiten da (RADIOLIB_NC)   DIO1 = RADIOLIB_NC
SX1276 radio = new Module(LORA_IRQ_DUMB, LORA_IRQ, RADIOLIB_NC, RADIOLIB_NC,
                          SPI1, SPISettings(200000, MSBFIRST, SPI_MODE0));

// --- Irrati-parametroak (BERDINAK beacon-ean eta atebidean) ---
#define FREQ    868.0    // MHz (Europako banda)
#define SF      12       // 7 (A fasea) edo 12 (B fasea)
#define BW      125.0    // kHz
#define CR      5        // coding rate 4/5
#define SYNCW   0x12     // sync word (sare pribatua)
#define TXP     17       // TX potentzia (dBm)
#define PREAMB  8        // aitzin-seinalearen luzera

// SF12-an pakete batek ~1 s behar du airean: utzi tarte zabala
// (gainera, 868 bandaren %1eko duty cycle-a hobeto errespetatzen du).
#if SF >= 11
  #define INTERVAL 5000
#else
  #define INTERVAL 2000
#endif

long counter = 0;

// MKR WAN 1310/1300-eko Murata moduluaren reset-sekuentzia
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

  Serial.print("BEACON RadioLib OK -> SF="); Serial.print(SF);
  Serial.print(" BW="); Serial.print(BW);
  Serial.print(" CR=4/"); Serial.print(CR);
  Serial.print(" TX="); Serial.print(TXP); Serial.println("dBm");
}

void loop() {
  // RadioLib transmit()-ek String erreferentzia bat eskatzen du (ez-const):
  // erabili izendun aldagaia, ez String(counter) bezalako aldi baterakoa.
  String msg = String(counter);
  int state = radio.transmit(msg);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.print("Bidalitako paketea #");
    Serial.println(counter);
  } else {
    Serial.print("TX error: ");
    Serial.println(state);
  }

  counter++;
  delay(INTERVAL);
}
