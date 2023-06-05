import serial
import paho.mqtt.client as mqtt
import json
import ssl, socket

# Función que se llama cuando el cliente se conecta al broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado al broker MQTT")
    else:
        print(f"Error de conexión con código {rc}")

# Función que se llama cuando un mensaje ha sido publicado
def on_publish(client, userdata, mid):
    print("Mensaje publicado")

line = []
data = dict()
broker = "iot.eie.ucr.ac.cr"
port = 1883
topic = "v1/devices/me/telemetry"
token="a39n3tsc8jzbqdorkp65"  # Reemplaza esto por tu token de acceso del dispositivo.

button = {"enabled": False}

ser = serial.Serial('/dev/ttyACM0', 115200)

# Configura el cliente MQTT
client = mqtt.Client()

# Define las funciones de callback
client.on_connect = on_connect
client.on_publish = on_publish

# Configura las credenciales
client.username_pw_set(token)  # Sólo pasamos el token como nombre de usuario.

# Conéctate al servidor MQTT
client.connect(broker, port, 60)

# Comienza el loop MQTT para procesar los callbacks y mantener la conexión
client.loop_start()

while True:
    # Lee una línea del puerto serie
    line = ser.readline()
    text = line.decode('utf-8').strip()
    print(text)
    substrings = text.split(',')
    data = {"voltaje": substrings[0] + " V",
            "Eje X: ": substrings[1],
            "Eje Y:": substrings[2],
            "Eje Z:": substrings[3],
            "porcentaje de carga" : substrings[4] + "%"
            }
    json_data = json.dumps(data)

    # Publica los datos en un topic MQTT
    client.publish(topic, payload=json_data, qos=1)
