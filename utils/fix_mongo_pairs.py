import pymongo

from pymongo import MongoClient
client = MongoClient()

client = MongoClient('localhost', 27017)
db = client.colposcopy

collection = db.ranking
pairs = collection.find()

phase_name = ["", "", "plain", "green", "hinselmann", "schiller"]

for p in pairs:
    key = p["_id"].split('/')
    p["phase"] = phase_name[int(key[0])]
    collection.save(p)
