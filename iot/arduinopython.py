# -*- coding: utf-8 -*-
"""
Python atebidea: serie-ataka (USB) <-> MQTT, bi noranzkoetan.

1) Datuak (modulu zentrala -> MQTT):
   Modulu zentralak serie-atakara idazten dituen lerroak irakurtzen ditu
   (formatua: `t;ppm;hezetasuna;tenperatura;sentsazio_termikoa`), JSON
   objektu batera bihurtzen ditu eta EMQX broker-era argitaratzen ditu
   `sentsoreak/data` gaian. Telegraf-ek gai horretara harpidetuta jasotzen
   ditu mezuak eta InfluxDB-n idazten ditu.

2) Aginduak (MQTT -> modulu zentrala):
   `sentsoreak/aginduak` gaira harpidetzen da. Jasotako agindua
   ("piztu", "itzali" edo "aldatu") serie-atakatik bidaltzen dio modulu
   zentralari (`CMD:<kodea>` lerroa), eta honek LoRa bidez sentsore-modulura.
   Aginduak edozein MQTT bezerotatik bidal daitezke, baita nabigatzailetik ere
   (EMQX dashboard-eko WebSocket bezeroa, ws://localhost:8083/mqtt).

Mendekotasunak: pyserial, paho-mqtt >= 2.0 (Conda `TFG` ingurunean).
"""

import json
import queue
import sys
import time

import serial
from paho.mqtt import client as mqtt_client
from paho.mqtt.enums import CallbackAPIVersion

# KONFIGURAZIOA
SERIAL_PORT = 'COM3'  # Egiaztatu ataka (COM3, COM4...)
BAUD_RATE = 9600
BROKER = 'localhost'
TOPIC = "sentsoreak/data"
AGINDU_TOPIC = "sentsoreak/aginduak"

# Agindu-kodeak (modulu zentralaren eta sentsore-moduluaren programekin bat)
AGINDUAK = {"aldatu": 0, "piztu": 1, "itzali": 2}

# MQTT callback-etik (beste hari batean) serie-atakara idazteko ilara
agindu_ilara = queue.Queue()


def balio_zuzenak(p):
    """Neurketak sentsoreen tarte fisikoaren barruan daudela egiaztatzen du."""
    return (0 <= p["ppm"] <= 1023          # MQ-135: ADC 10 bit
            and 0 <= p["hum"] <= 100       # hezetasun erlatiboa (%)
            and -20 <= p["temp"] <= 60     # DHT11
            and -30 <= p["sens"] <= 70)    # sentsazio termikoa


def lerroa_prozesatu(line):
    """Serie-atakako lerro bat JSON payload bihurtzen du.

    None itzultzen du lerroa baztertu behar bada (formatu okerra edo
    balio ezinezkoak). DHT11-ak huts egin badu (h=0 eta t=0), MQ-135aren
    balioa baino ez da itzultzen.
    """
    if ";" not in line:
        return None
    parts = line.split(";")
    if len(parts) < 5:
        return None
    try:
        payload = {
            "ts": int(parts[0]),
            "ppm": int(parts[1]),
            "hum": float(parts[2]),
            "temp": float(parts[3]),
            "sens": float(parts[4]),
        }
    except ValueError:
        return None

    # Tarte fisikotik kanpoko balioak baztertu (pakete hondatuak)
    if not balio_zuzenak(payload):
        print(f"Baztertuta (balio ezinezkoak): {line}")
        return None

    # DHT11-ak huts egiten duenean, sentsore-moduluak h=0 eta t=0 bidaltzen
    # ditu: ez dira neurketak, beraz MQ-135a baino ez argitaratu
    if payload["hum"] == 0 and payload["temp"] == 0:
        print(f"DHT11 irakurketak huts egin du; PPM soilik: {line}")
        for k in ("hum", "temp", "sens"):
            del payload[k]
    return payload


def agindua_interpretatu(testua):
    """MQTT mezu baten testua agindu-kode bihurtzen du (None, ezezaguna bada)."""
    return AGINDUAK.get(testua.strip().lower())


def connect_mqtt():
    # paho-mqtt 2.0: callback-en API bertsioa esplizituki adierazi behar da
    client = mqtt_client.Client(
        client_id="python-gateway",
        callback_api_version=CallbackAPIVersion.VERSION2
    )

    def on_connect(client, userdata, flags, rc, properties):
        if rc == 0:
            print("MQTT broker-era konektatuta")
            # on_connect barruan harpidetu: berriro konektatzean ere harpidetzen da
            client.subscribe(AGINDU_TOPIC)
        else:
            print(f"Konexioak huts egin du, kodea: {rc}")

    def on_message(client, userdata, msg):
        testua = msg.payload.decode("utf-8", errors="replace")
        kodea = agindua_interpretatu(testua)
        if kodea is None:
            print(f"Agindu ezezaguna ({msg.topic}): {testua!r}")
            return
        agindu_ilara.put(kodea)
        print(f"Agindua jasota: {testua.strip()} -> CMD:{kodea}")

    client.on_connect = on_connect
    client.on_message = on_message

    try:
        client.connect(BROKER, 1883)
        return client
    except Exception as e:
        print(f"Errorea Docker-eko broker-era konektatzean: {e}")
        sys.exit(1)


def main():
    client = connect_mqtt()
    client.loop_start()

    try:
        print(f"Arduinora konektatzen {SERIAL_PORT} atakan...")
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)
        print("Sistema prest. Datuen zain...")

        while True:
            # 1) Zain dauden aginduak modulu zentralera bidali
            while not agindu_ilara.empty():
                kodea = agindu_ilara.get()
                ser.write(f"CMD:{kodea}\n".encode("ascii"))

            # 2) Modulu zentralaren datuak irakurri eta argitaratu
            if ser.in_waiting > 0:
                try:
                    line = ser.readline().decode('utf-8').strip()
                except UnicodeDecodeError:
                    continue  # lerro hondatua: baztertu eta jarraitu
                payload = lerroa_prozesatu(line)
                if payload is not None:
                    json_msg = json.dumps(payload)
                    client.publish(TOPIC, json_msg)  # QoS 0 (lehenetsia)
                    print(f"Bidalita: {json_msg}")
            else:
                time.sleep(0.05)  # CPUa ez erabiltzeko itxaron-begiztan

    except KeyboardInterrupt:
        print("Programa ixten")
    except Exception as e:
        print(f"Errorea: {e}")
    finally:
        client.loop_stop()


if __name__ == "__main__":
    main()
