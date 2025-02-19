import mysql.connector
import paho.mqtt.client as mqtt
import json
import datetime

# Configuración de la base de datos MySQL
db = mysql.connector.connect(
    host="localhost",
    user="prueba",
    password="Monlau2020",
    database="bd_monlab"
)
cursor = db.cursor()

# ID del experimento (modifícalo según corresponda)
ID_EXPERIMENTO = 1

# Función para manejar mensajes MQTT
def on_message(client, userdata, msg):
    print(f"Mensaje recibido en el topic '{msg.topic}': {msg.payload.decode()}")

    try:
        # Decodificar el mensaje JSON
        data = json.loads(msg.payload.decode())

        # ⚠️ Corregir los nombres de las claves según el JSON recibido
        tiempo1 = data.get("tiempo_sensor1")  # Timestamp UNIX
        tiempo2 = data.get("tiempo_sensor2")  # Timestamp UNIX

        # Validar que los valores no sean None
        if tiempo1 is None or tiempo2 is None:
            print("Error: Alguno de los valores recibidos es 'None'")
            return
        
        # Convertir timestamps UNIX a formato `DATETIME`
        tiempo1_dt = datetime.datetime.fromtimestamp(int(tiempo1)).strftime('%Y-%m-%d %H:%M:%S')
        tiempo2_dt = datetime.datetime.fromtimestamp(int(tiempo2)).strftime('%Y-%m-%d %H:%M:%S')

        # Insertar en la base de datos
        query = """
            INSERT INTO Datos_Experimentos (tiempo1, tiempo2, id_experimento)
            VALUES (%s, %s, %s)
        """
        cursor.execute(query, (tiempo1_dt, tiempo2_dt, ID_EXPERIMENTO))
        db.commit()

        print(f"Dato almacenado en la base de datos: tiempo1={tiempo1_dt}, tiempo2={tiempo2_dt}")

    except json.JSONDecodeError:
        print("Error: El mensaje no es un JSON válido.")
    except Exception as e:
        print("Error al procesar el mensaje:", e)

# Configuración de MQTT
broker = "localhost"
topic = "sensores/tiempo"

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)  # Corregir advertencia de deprecación
client.on_message = on_message

# Conectar al broker MQTT
client.connect(broker, 1883, 60)
client.subscribe(topic)

print(f"Escuchando mensajes en el topic '{topic}'...")
client.loop_forever()
