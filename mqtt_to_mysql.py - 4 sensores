import mysql.connector
import paho.mqtt.client as mqtt
import json
import datetime

# Configuración de la base de datos MySQL
db = mysql.connector.connect(
    host="79.154.117.109",
    user="buda",
    password="Monlab_2025",
    database="monlab",
    port=5000
)
cursor = db.cursor()

ID_EXPERIMENTO = 1

# Función para manejar mensajes MQTT
def on_message(client, userdata, msg):
    print(f"Mensaje recibido en el topic '{msg.topic}': {msg.payload.decode()}")

    try:
        data = json.loads(msg.payload.decode())

        # Obtener valores del JSON
        tiempo1 = data.get("tiempo_sensor1")
        tiempo2 = data.get("tiempo_sensor2")
        tiempo3 = data.get("tiempo_sensor3")
        tiempo4 = data.get("tiempo_sensor4")

        if None in (tiempo1, tiempo2, tiempo3, tiempo4):
            print("Error: Alguno de los valores recibidos es 'None'")
            return

        # Convertir timestamps UNIX a datetime con milisegundos
        def convertir_timestamp(ts):
            return datetime.datetime.fromtimestamp(float(ts))

        tiempo1_dt = convertir_timestamp(tiempo1)
        tiempo2_dt = convertir_timestamp(tiempo2)
        tiempo3_dt = convertir_timestamp(tiempo3)
        tiempo4_dt = convertir_timestamp(tiempo4)

        query = """
            INSERT INTO DATOS_EXPERIMENTOS (id_experimento, tiempo1, tiempo2, tiempo3, tiempo4)
            VALUES (%s, %s, %s, %s, %s)
        """
        cursor.execute(query, (ID_EXPERIMENTO, tiempo1_dt, tiempo2_dt, tiempo3_dt, tiempo4_dt))
        db.commit()

        print(f"Dato almacenado en la base de datos:")
        print(f"  tiempo1 = {tiempo1_dt}")
        print(f"  tiempo2 = {tiempo2_dt}")
        print(f"  tiempo3 = {tiempo3_dt}")
        print(f"  tiempo4 = {tiempo4_dt}")

    except json.JSONDecodeError:
        print("Error: El mensaje no es un JSON válido.")
    except Exception as e:
        print("Error al procesar el mensaje:", e)

# Configuración de MQTT
broker = "localhost"
topic = "sensores/tiempo"

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_message = on_message

client.connect(broker, 1883, 60)
client.subscribe(topic)

print(f"Escuchando mensajes en el topic '{topic}'...")
client.loop_forever()
