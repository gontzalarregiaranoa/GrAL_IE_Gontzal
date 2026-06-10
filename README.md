# LoRan oinarritutako sentsore-sare malgua

Gradu Amaierako Lana (GrAL) — Ingeniaritza Elektronikoko Gradua, UPV/EHU
**Egilea:** Gontzal Arregi Aranoa

Barnealdeko airearen kalitatea (tenperatura, hezetasuna eta gasak/CO₂)
monitorizatzeko sistema, kostu txikiko **LoRa** sentsore-sare batean
oinarritua, eta hodeian bistaratzeko **IoT** pila batekin (MQTT + Telegraf +
InfluxDB, Docker gainean).

---

## Arkitektura

```
  ┌─────────────────────┐        LoRa 868 MHz        ┌──────────────────────┐
  │   Sentsore-modulua  │  ───────────────────────▶  │   Modulu zentrala    │
  │   (sentsore-nodoa)  │   SF7 · BW125 · CR4/5      │   (atebidea)         │
  │  Arduino MKR WAN    │   ◀───────────────────────  │  Arduino MKR WAN     │
  │  1310               │      (bi norabidekoa)       │  1310 + LCD I2C      │
  │  · DHT11 (T/HR)     │                             └──────────┬───────────┘
  │  · MQ-135 (gasak)   │                                        │ USB (Serie)
  │  · LED semaforoa    │                                        ▼
  │  · Buzzer           │                             ┌──────────────────────┐
  │  · Bateria          │                             │  Python atebidea      │
  └─────────────────────┘                             │  (pyserial → MQTT)    │
                                                       └──────────┬───────────┘
                                                                  │ MQTT
                          ┌───────────────────────────────────────▼──────────┐
                          │  Docker (WSL):  EMQX → Telegraf → InfluxDB        │
                          │                                  → Chronograf      │
                          └───────────────────────────────────────────────────┘
```

## Biltegiaren egitura

| Karpeta / fitxategia | Edukia |
|---|---|
| `ARDUINO/firmware/Programa_del_modulo_sensor/` | Sentsore-nodoaren firmwarea (DHT11/MQ-135 irakurri, semaforoa, LoRa bidalketa) |
| `ARDUINO/firmware/Programa_del_modulo_central/` | Atebidearen firmwarea (LoRa jaso, LCD, botoia) |
| `ARDUINO/firmware/arduinopython.py` | Python atebidea: Serie (USB) → MQTT |
| `ARDUINO/irismen_proba/` | Irismen-probako sketchak: `beacon_nodo`/`gateway_rssi` (`LoRa.h`, SF7) eta `*_radiolib` (RadioLib, SF7 vs SF12 azterketa) |
| `ARDUINO/probak/` | Aurretiazko proba-sketchak (igorlea, hartzailea, MQ-135, pantaila, sentsoreak) |
| `GrAL/` | Memoriaren LaTeX iturburuak (`main.tex`, `Bibliografia.bib`, `Irudiak/`) |
| `datuak/` | Landa-datu gordinak: irismen-proben serie-historikoa (SF7 eta SF12) eta kokapena |

## Hardwarea

- 2× **Arduino MKR WAN 1310** (Murata modulua, **SX1276** irratiarekin)
- **DHT11** sentsorea (tenperatura eta hezetasun erlatiboa)
- **MQ-135** sentsorea (airearen kalitatea / gasak)
- **16×2 LCD** pantaila, **I²C** moduluarekin
- LEDak (semaforoa), buzzer aktiboa, Li-Po bateria, antenak, breadboardak

## Abian jartzea

### 1. Firmwarea (Arduino IDE)
Liburutegi hauek behar dira: `LoRa` (sandeepmistry), `DHT sensor library`
(Adafruit), `LiquidCrystal_I2C`. Kargatu `Programa_del_modulo_sensor` nodoan eta
`Programa_del_modulo_central` atebidean.

### 2. Python atebidea
```bash
conda create -n TFG python=3.11
conda activate TFG
pip install pyserial paho-mqtt
python ARDUINO/firmware/arduinopython.py   # COM ataka egokitu
```

### 3. IoT pila (Docker WSL-en)
```bash
docker-compose up -d      # EMQX, InfluxDB, Telegraf, Chronograf
# Bistaratzea: http://localhost:8888 (Chronograf)
```

## LoRa irismen-proba (laburpena)

Bi landa-proba Zamudion (868 MHz / BW 125 kHz / CR 4/5 / 17 dBm):

- **SF7 (`LoRa.h`):** **~450 m-ko** irismen egonkorra **ikus-lerroarekin (LOS)**;
  lotura erortzen da **NLOS**-era pasatzean (bihurgunea + landaredia) eta LOS
  berreskuratzean indartzen da.
- **SF12 (RadioLib):** **~775 m** (ia bikoitza), RSSI **−140 dBm**-raino eta SNR
  −20 dB-raino, lotura zaratapean mantenduz.

Xehetasunak, mapak eta RSSI/SNR grafikoak memoriako «LoRa irismenaren analisia»
eta «RadioLib liburutegira migrazioa…» ataletan. Datu gordinak `datuak/`-en.

## Lizentzia

© 2026 Gontzal Arregi Aranoa.
