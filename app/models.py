from pymongo import MongoClient

client = MongoClient("localhost", 27017)

db = client.flask_db

sensor_datas_collection = db.sensor_datas
