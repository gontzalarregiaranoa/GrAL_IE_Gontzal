# -*- coding: utf-8 -*-
"""
Python atebidea (pasarela): serie-ataka (USB) -> MQTT.

Modulu zentralak serie-atakara idazten dituen lerroak irakurtzen ditu
(formatua: `t;ppm;hezetasuna;tenperatura;sentsazio_termikoa`), JSON
objektu batera bihurtzen ditu eta EMQX broker-era argitaratzen ditu
`sentsoreak/data` gaian. Telegraf-ek gai horretara harpidetuta jasotzen
ditu mezuak eta InfluxDB-n idazten ditu.

Mendekotasunak: pyserial, paho-mqtt >= 2.0 (Conda `TFG` ingurunean).
"""

import serial
import time
import json
import sys
from paho.mqtt import client as mqtt_client
from paho.mqtt.enums import CallbackAPIVersion

# KONFIGURAZIOA
SERIAL_PORT = 'COM3'  # Egiaztatu ataka (COM3, COM4...)
BAUD_RATE = 9600
BROKER = 'localhost'
TOPIC = "sentsoreak/data"


def connect_mqtt():
    # paho-mqtt 2.0: callback-en API bertsioa esplizituki adierazi behar da
    client = mqtt_client.Client(
        client_id="python-gateway",
        callback_api_version=CallbackAPIVersion.VERSION2
    )

    def on_connect(client, userdata, flags, rc, properties):
        if rc == 0:
            print("MQTT broker-era konektatuta")
        else:
            print(f"Konexioak huts egin du, kodea: {rc}")

    client.on_connect = on_connect

    try:
        client.connect(BROKER, 1883)
        return client
    except Exception as e:
        print(f"Errorea Docker-eko broker-era konektatzean: {e}")
        sys.exit(1)


# PROGRAMAREN HASIERA
client = connect_mqtt()
client.loop_start()

def balio_zuzenak(p):
    """Neurketak sentsoreen tarte fisikoaren barruan daudela egiaztatzen du."""
    return (0 <= p["ppm"] <= 1023          # MQ-135: ADC 10 bit
            and 0 <= p["hum"] <= 100       # hezetasun erlatiboa (%)
            and -20 <= p["temp"] <= 60     # DHT11
            and -30 <= p["sens"] <= 70)    # sentsazio termikoa


try:
    print(f"Arduinora konektatzen {SERIAL_PORT} atakan...")
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)
    print("Sistema prest. Datuen zain...")

    while True:
        if ser.in_waiting > 0:
            try:
                line = ser.readline().decode('utf-8').strip()

                if ";" in line:
                    parts = line.split(";")
                    if len(parts) >= 5:
                        payload = {
                            "ts": int(parts[0]),
                            "ppm": int(parts[1]),
                            "hum": float(parts[2]),
                            "temp": float(parts[3]),
                            "sens": float(parts[4])
                        }

                        # Tarte fisikotik kanpoko balioak baztertu (pakete hondatuak)
                        if not balio_zuzenak(payload):
                            print(f"Baztertuta (balio ezinezkoak): {line}")
                            continue

                        # DHT11-ak huts egiten duenean, sentsore-moduluak h=0 eta t=0
                        # bidaltzen ditu: ez dira neurketak, beraz MQ-135a baino ez argitaratu
                        if payload["hum"] == 0 and payload["temp"] == 0:
                            print(f"DHT11 irakurketak huts egin du; PPM soilik: {line}")
                            for k in ("hum", "temp", "sens"):
                                del payload[k]

                        json_msg = json.dumps(payload)
                        client.publish(TOPIC, json_msg)  # QoS 0 (lehenetsia)
                        print(f"Bidalita: {json_msg}")

            except Exception:
                # Lerro osatugabea edo ezin bihurtua: baztertu eta jarraitu
                pass

except Exception as e:
    print(f"Errorea: {e}")
except KeyboardInterrupt:
    print("Programa ixten")
    client.loop_stop()
