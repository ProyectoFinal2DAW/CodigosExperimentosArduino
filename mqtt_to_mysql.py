import mysql.connector
import paho.mqtt.client as mqtt
import json
from datetime import datetime, timedelta, timezone

# Zona horaria (UTC+2 ejemplo España verano)
ZONA_HORARIA = timezone(timedelta(hours=2))

# Conexión MySQL
db = mysql.connector.connect(
    host="monlab.ddns.net",
    user="buda",
    password="Monlab_2025",
    database="monlab",
    port=5000
)
cursor = db.cursor()

ID_EXPERIMENTO = 1
datos_temporales = {}

def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())
        sensor = data.get("sensor")
        tiempo = float(data.get("tiempo"))

        if sensor is None or tiempo is None:
            print("Mensaje incompleto:", data)
            return

        dt = datetime.fromtimestamp(tiempo, tz=timezone.utc).astimezone(ZONA_HORARIA)
        print(f"{sensor} => {dt.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]}")

        datos_temporales[sensor] = dt

        if all(k in datos_temporales for k in ["sensor1", "sensor2", "sensor3", "sensor4"]):
            query = """
                INSERT INTO DATOS_EXPERIMENTOS (id_experimento, tiempo1, tiempo2, tiempo3, tiempo4)
                VALUES (%s, %s, %s, %s, %s)
            """
            cursor.execute(query, (
                ID_EXPERIMENTO,
                datos_temporales["sensor1"],
                datos_temporales["sensor2"],
                datos_temporales["sensor3"],
                datos_temporales["sensor4"]
            ))
            db.commit()
            print("Datos guardados en la base de datos.")
            datos_temporales.clear()

    except Exception as e:
        print("Error al procesar mensaje:", e)

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.subscribe("sensores/tiempo")

print("Escuchando MQTT...")
client.loop_forever()
