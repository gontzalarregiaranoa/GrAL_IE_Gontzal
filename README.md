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
| `ARDUINO/Programa_del_modulo_sensor/` | Firmware del nodo sensor (lee DHT11/MQ-135, semáforo, envío LoRa) |
| `ARDUINO/Programa_del_modulo_central/` | Firmware del gateway (recibe LoRa, LCD, botón) |
| `ARDUINO/PRUEBA_ALCANCE/beacon_nodo/` | Sketch de la **prueba de alcance** (beacon, 1 paquete/2 s) |
| `ARDUINO/PRUEBA_ALCANCE/gateway_rssi/` | Sketch de la prueba de alcance (mide RSSI/SNR, CSV + LCD) |
| `ARDUINO/arduinopython.py` | Pasarela Python: Serial (USB) → MQTT |
| `GrAL/` | Fuentes LaTeX de la memoria (`main.tex`, `Bibliografia.bib`, `Irudiak/`) |
| `oro.docx` | Documento de trabajo con los datos crudos de la prueba de alcance |

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
python ARDUINO/arduinopython.py   # ajustar el puerto COM
```

### 3. Pila IoT (Docker en WSL)
```bash
docker-compose up -d      # EMQX, InfluxDB, Telegraf, Chronograf
# Visualización: http://localhost:8888 (Chronograf)
```

## Prueba de alcance LoRa (resumen)

Test de campo con configuración 868 MHz / SF7 / BW 125 kHz / CR 4/5 / 17 dBm.
Alcance estable de **~450 m con línea de visión (LOS)**; el enlace cae al pasar a
**NLOS** (curva + vegetación) y se recupera al restablecer la LOS. Detalle, mapa y
gráficas RSSI/SNR en la sección «LoRa irismenaren analisia» de la memoria.

## Licencia

© 2026 Gontzal Arregi Aranoa.
