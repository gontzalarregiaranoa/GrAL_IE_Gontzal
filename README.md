# LoRan oinarritutako sentsore-sare malgua / Red de sensores flexible basada en LoRa

Gradu Amaierako Lana (GrAL) — Ingeniaritza Elektronikoko Gradua, UPV/EHU
**Egilea:** Gontzal Arregi Aranoa

Sistema de monitorización de calidad del aire en interiores (temperatura, humedad y
gases/CO₂) basado en una red de sensores **LoRa** de bajo coste, con visualización en la
nube mediante una pila **IoT** (MQTT + Telegraf + InfluxDB sobre Docker).

---

## Arquitectura

```
  ┌─────────────────────┐        LoRa 868 MHz        ┌──────────────────────┐
  │   Sentsore-modulua  │  ───────────────────────▶  │   Modulu zentrala    │
  │   (nodo sensor)     │   SF7 · BW125 · CR4/5      │   (gateway)          │
  │  Arduino MKR WAN    │   ◀───────────────────────  │  Arduino MKR WAN     │
  │  1310               │      (bidirekzionala)       │  1310 + LCD I2C      │
  │  · DHT11 (T/HR)     │                             └──────────┬───────────┘
  │  · MQ-135 (gasak)   │                                        │ USB (Serial)
  │  · LED semaforoa    │                                        ▼
  │  · Buzzer           │                             ┌──────────────────────┐
  │  · Bateria          │                             │  Python gateway       │
  └─────────────────────┘                             │  (pyserial → MQTT)    │
                                                       └──────────┬───────────┘
                                                                  │ MQTT
                          ┌───────────────────────────────────────▼──────────┐
                          │  Docker (WSL):  EMQX → Telegraf → InfluxDB        │
                          │                                  → Chronograf      │
                          └───────────────────────────────────────────────────┘
```

## Estructura del repositorio

| Carpeta / fichero | Contenido |
|---|---|
| `ARDUINO/firmware/Programa_del_modulo_sensor/` | Firmware del nodo sensor (lee DHT11/MQ-135, semáforo, envío LoRa) |
| `ARDUINO/firmware/Programa_del_modulo_central/` | Firmware del gateway (recibe LoRa, LCD, botón) |
| `ARDUINO/firmware/arduinopython.py` | Pasarela Python: Serial (USB) → MQTT |
| `ARDUINO/irismen_proba/` | Sketches de la prueba de alcance: `beacon_nodo`/`gateway_rssi` (`LoRa.h`, SF7) y `*_radiolib` (RadioLib, estudio SF7 vs SF12) |
| `ARDUINO/probak/` | Sketches de pruebas previas (emisor, receptor, MQ-135, display, sensores) |
| `GrAL/` | Fuentes LaTeX de la memoria (`main.tex`, `Bibliografia.bib`, `Irudiak/`) |
| `datuak/` | Datos crudos de campo: histórico serial de las pruebas de alcance (SF7 y SF12) y ubicación |

## Hardware

- 2× **Arduino MKR WAN 1310** (módulo Murata con radio **SX1276**)
- Sensor **DHT11** (temperatura y humedad relativa)
- Sensor **MQ-135** (calidad del aire / gases)
- Pantalla **LCD 16×2** con módulo **I²C**
- LEDs (semáforo), buzzer activo, batería Li-Po, antenas, breadboards

## Puesta en marcha

### 1. Firmware (Arduino IDE)
Requiere las librerías: `LoRa` (sandeepmistry), `DHT sensor library` (Adafruit),
`LiquidCrystal_I2C`. Cargar `Programa_del_modulo_sensor` en el nodo y
`Programa_del_modulo_central` en el gateway.

### 2. Pasarela Python
```bash
conda create -n TFG python=3.11
conda activate TFG
pip install pyserial paho-mqtt
python ARDUINO/firmware/arduinopython.py   # ajustar el puerto COM
```

### 3. Pila IoT (Docker en WSL)
```bash
docker-compose up -d      # EMQX, InfluxDB, Telegraf, Chronograf
# Visualización: http://localhost:8888 (Chronograf)
```

## Prueba de alcance LoRa (resumen)

Dos pruebas de campo en Zamudio (868 MHz / BW 125 kHz / CR 4/5 / 17 dBm):

- **SF7 (`LoRa.h`):** alcance estable de **~450 m con línea de visión (LOS)**; el enlace
  cae al pasar a **NLOS** (curva + vegetación) y se recupera al restablecer la LOS.
- **SF12 (RadioLib):** **~775 m** (casi el doble), con RSSI hasta **−140 dBm** y SNR hasta
  −20 dB, manteniendo el enlace por debajo del ruido.

Detalle, mapas y gráficas RSSI/SNR en las secciones «LoRa irismenaren analisia» y
«RadioLib liburutegira migrazioa…» de la memoria. Datos crudos en `datuak/`.

## Licencia

© 2026 Gontzal Arregi Aranoa.
