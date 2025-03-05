import mysql.connector
import paho.mqtt.client as mqtt
import json
import datetime

# Configuración de la base de datos MySQL
db = mysql.connector.connect(
    host="79.155.66.139",
    user="buda",
    password="Monlab_2025",
    database="monlab",
    port=5000
)
cursor = db.cursor()

# Parámetros del experimento
ID_EXPERIMENTO = 2
GRADO_INCLINACION = 20
DISTANCIA_SENSORES = 1.0  # Distancia entre los sensores en metros

# Función para manejar mensajes MQTT
def on_message(client, userdata, msg):
    print(f"Mensaje recibido en el topic '{msg.topic}': {msg.payload.decode()}")

    try:
        # Decodificar el mensaje JSON
        data = json.loads(msg.payload.decode())

        # Obtener los tiempos de los sensores
        tiempo1 = data.get("tiempo_sensor1")  # Timestamp UNIX
        tiempo2 = data.get("tiempo_sensor2")  # Timestamp UNIX

        # Validar que los valores no sean None
        if tiempo1 is None or tiempo2 is None:
            print("Error: Alguno de los valores recibidos es 'None'")
            return

        # Convertir timestamps UNIX a objetos datetime
        tiempo1_dt = datetime.datetime.fromtimestamp(int(tiempo1))
        tiempo2_dt = datetime.datetime.fromtimestamp(int(tiempo2))

        # Calcular el tiempo transcurrido en segundos
        tiempo_transcurrido = (tiempo2_dt - tiempo1_dt).total_seconds()

        # Calcular la velocidad (m/s)
        if tiempo_transcurrido > 0:
            velocidad = DISTANCIA_SENSORES / tiempo_transcurrido
            resultado = f"{velocidad:.2f} m/s"
        else:
            print("Error: El tiempo transcurrido es cero o negativo")
            return

        # Insertar en la base de datos
        query = """
            INSERT INTO DATOS_EXPERIMENTOS (id_experimento, tiempo1, tiempo2, resultado)
            VALUES (%s, %s, %s, %s)
        """
        cursor.execute(query, (ID_EXPERIMENTO, tiempo1_dt, tiempo2_dt, resultado))
        db.commit()

        print(f"Dato almacenado en la base de datos: tiempo1={tiempo1_dt}, tiempo2={tiempo2_dt}, velocidad={resultado}")

    except json.JSONDecodeError:
        print("Error: El mensaje no es un JSON válido.")
    except Exception as e:
        print("Error al procesar el mensaje:", e)

# Configuración de MQTT
broker = "localhost"
topic = "sensores/velocidad"

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_message = on_message

# Conectar al broker MQTT
client.connect(broker, 1883, 60)
client.subscribe(topic)

print(f"Escuchando mensajes en el topic '{topic}'...")
client.loop_forever()
