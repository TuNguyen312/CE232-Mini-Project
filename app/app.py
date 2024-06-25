from flask import Flask, render_template, jsonify
from flask_mqtt import Mqtt
import json
import datetime
import models
import threading
import time

app = Flask(__name__, static_url_path="/static")

app.config["MQTT_BROKER_URL"] = "mqtt.flespi.io"
app.config["MQTT_BROKER_PORT"] = 1883
app.config["MQTT_USERNAME"] = (
    "dkZ2qmV2msFaAda1k2J0PSSqYjRH08KY9bmvwLaIdSWpqA0a9vScYP7WNHAo3cDu"
)
app.config["MQTT_PASSWORD"] = ""
app.config["MQTT_REFRESH_TIME"] = 1.0  # refresh time in seconds


mqtt = Mqtt(app)

# sensor_datas = None
models.sensor_datas_collection.create_index("uid", unique=True)
sensor_data = None
buffer_lock = threading.Lock()
uid = None


def convert_objectid_to_str(data):
    """
    Convert ObjectId to string in MongoDB documents.
    """
    if isinstance(data, list):
        for item in data:
            item["_id"] = str(item["_id"])
    elif isinstance(data, dict):
        data["_id"] = str(data["_id"])
    return data


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/get_data")
def get_data():
    sensor_datas = list(
        models.sensor_datas_collection.find().sort("time", -1).limit(10)
    )
    sensor_datas = convert_objectid_to_str(sensor_datas)
    return jsonify(sensor_datas)


@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    mqtt.subscribe("Data/")


@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    global uid, sensor_data
    data = json.loads(message.payload.decode())
    with buffer_lock:
        if uid != int(data["uid"]):
            uid = int(data["uid"])
            time = str(datetime.datetime.now()).split(".")[0]
            sensor_data = data
            sensor_data["time"] = time


def insert_data_periodically():
    while True:
        global sensor_data
        time.sleep(10)  # Chờ 10 giây
        with buffer_lock:
            if sensor_data:
                # Chèn tất cả dữ liệu từ buffer vào MongoDB
                try:
                    models.sensor_datas_collection.insert_one(sensor_data)
                    sensor_data.clear()
                    print(f"Inserted document.")
                except Exception as e:
                    print(f"Error inserting document: {e}")


insertion_thread = threading.Thread(target=insert_data_periodically)
insertion_thread.daemon = True  # Đảm bảo thread dừng khi chương trình dừng
insertion_thread.start()

if __name__ == "__main__":
    app.run(port=5000, debug=True)
