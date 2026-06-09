# -*- coding: utf-8 -*-
"""
Created on Wed May 29 19:11:03 2024

@author: gontz
"""


#PUERTOKO DATUAK PYTHONEN JASO

import serial
import time
from paho.mqtt import client as mqtt_client
import random

#Variables del Broker
broker = '172.22.52.133'   #ip del broker eth0
port = 1883
topic = "python/mqtt"
client_id = f'python-mqtt-{random.randint(0, 1000)}'
# username = 'emqx'
# password = 'public'


#Función para conectarse al MQTT
def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
    # For paho-mqtt 2.0.0, you need to add the properties parameter.
    # def on_connect(client, userdata, flags, rc, properties):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)
    # Set Connecting Client ID
    client = mqtt_client.Client(client_id)

    # For paho-mqtt 2.0.0, you need to set callback_api_version.
    # client = mqtt_client.Client(client_id=client_id, callback_api_version=mqtt_client.CallbackAPIVersion.VERSION2)

    # client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

#Funnción publisher para mandar mensajes del sensor al broker
def publish(client,line):
    msg_count = 1
    while True:
        time.sleep(1)
        
        result = client.publish(topic, line)
        # result: [0, 1]
        status = result[0]
        if status == 0:
            print(f"Send `{line}` to topic `{topic}`")
        else:
            print(f"Failed to send message to topic {topic}")
        msg_count += 1
        if msg_count > 5:
            break


# Configuración del puerto serial
portserial = 'COM3'  # Cambia esto según el puerto que estés usando
baudrate = 9600

try:
    ser = serial.Serial(portserial, baudrate)
    print(f"Conectado al puerto {port}")
except serial.SerialException as e:
    print(f"No se pudo abrir el puerto serial: {e}")
    #exit()

time.sleep(2)  # Esperar un poco para asegurar que la conexión serial esté estable

client=connect_mqtt()
client.loop_start()


# Lectura de datos
try:
    while True:
        if ser.in_waiting > 0:
            lines = ser.readline().decode('utf-8').rstrip()
            print(f"Datos recibidos: {lines}")
            # Split the message into a list of strings
            line = lines.split(';')
            # Convert each string value to a float
            #line = [float(value) for value in string_values]
            

            #inlfux db format
            measurement = "arduinomezua"
            tags = {
                "tag1": line[1],
                "tag2": line[2],
                "tag3": line[3],
                "tag4": line[4]
            }
            fields = {
                "field1": "PPM",
                "field2": "H",
                "field3": "T",
                "field4": "S"
            }
            timestamp = int(time.time())  # Use a proper timestamp here if needed

            #tag_set = ",".join([f"{k}={v}" for k, v in tags.items()])
            #field_set = ",".join([f'{k}="{v}"' for k, v in fields.items()])
            #field_set = ",".join([f'{k}="{v}" {k}="{v}"' for k, v in fields.items()])
            
            #line_protocol = f"{measurement},{tag_set} {field_set} {timestamp}"
            
            
            line_protocol="arduinomezua,tag1=sensor1 T=" + str(line[3]) + ",H=" + str(line[2])
                
            print(line_protocol)
            #msg = f"arduino,tag=3 field=40 {timestamp}"
            #publish(client, line_protocol)
            result=client.publish(topic, line_protocol)
            status = result[0]
            if status == 0:
                print(f"Send `{line_protocol}` to topic `{topic}`")
            else:
                print(f"Failed to send message to topic {topic}")

except KeyboardInterrupt:
    print("Interrumpido por el usuario")

# Cierre del puerto serial
ser.close()
print("Puerto serial cerrado")
client.loop_stop()
client.disconnect()










