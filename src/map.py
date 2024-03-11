import matplotlib.pyplot as plt
import numpy as np
from pymongo.mongo_client import MongoClient
from pymongo.server_api import ServerApi
from pymongo import DESCENDING
import time

"""
Draw the Map 
The supermarket map size is 151*91
including three counters on the left side, 
three columns, each with seven rows of shelves.
The locations of the three target are:
'Apple': (50, 21),
'Orange':(52,21),
'Chips': (110, 11),
'Milk': (80, 71)

"""

def connect():
    uri = "mongodb+srv://yangwyyw:9C3BPOnItsviJvUR@cluster0.nrvqeva.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0"

    # Create a new client and connect to the server
    client = MongoClient(uri, server_api=ServerApi('1'))

    # Send a ping to confirm a successful connection
    try:
        client.admin.command('ping')
        print("Pinged your deployment. You successfully connected to MongoDB!")
    except Exception as e:
        print(e)

    return client

# def locate_target(name):
#     return targets_named.get(name, "Target not found.")

def drawMap(last_location):
    x_range = 151
    y_range = 91
    obs = set()

    for i in range(x_range):
        obs.add((i, 0))
        obs.add((i, y_range - 1))

    for i in range(y_range):
        obs.add((0, i))
        obs.add((x_range - 1, i))

    for i in range(40, 60):
        obs.add((i, 70))
        obs.add((i, 60))
        obs.add((i, 50))
        obs.add((i, 40))
        obs.add((i, 30))
        obs.add((i, 20))
        obs.add((i, 10))

    for i in range(70, 90):
        obs.add((i, 70))
        obs.add((i, 60))
        obs.add((i, 50))
        obs.add((i, 40))
        obs.add((i, 30))
        obs.add((i, 20))
        obs.add((i, 10))

    for i in range(100, 120):
        obs.add((i, 70))
        obs.add((i, 60))
        obs.add((i, 50))
        obs.add((i, 40))
        obs.add((i, 30))
        obs.add((i, 20))
        obs.add((i, 10))

    for i in range(15, 26):
        obs.add((15, i))
        obs.add((25, i))
    for i in range(15, 25): 
        obs.add((i, 15))
        obs.add((i, 25))

    for i in range(35, 46):
        obs.add((15, i))
        obs.add((25, i))
    for i in range(15, 25): 
        obs.add((i, 35))
        obs.add((i, 45))

    for i in range(55, 66):
        obs.add((15, i))
        obs.add((25, i))
    for i in range(15, 25): 
        obs.add((i, 55))
        obs.add((i, 65))

    map_img = np.ones((y_range, x_range))
    for ob in obs:
        map_img[ob[1], ob[0]] = 0

    targets_named = {
        'Apple': (50, 21),
        'Orange':(52,21),
        'Chips': (110, 11),
        'Milk': (80, 71)
    }
    fig, ax = plt.subplots(figsize=(10, 6))
    ax.imshow(map_img, cmap='gray')
    ax.set_title('Supermarket')
    ax.text(15.5 ,20 , 'counter', color='green',fontsize=8.5)
    ax.text(15.5 ,40 , 'counter', color='green',fontsize=8.5)
    ax.text(15.5 ,60 , 'counter', color='green',fontsize=8.5)
    target_dots = {}  
    for name, location in targets_named.items():
        dot, = ax.plot(location[0], location[1], '*', markersize=10, label=name)
        target_dots[name] = dot  
        ax.text(location[0] + 1, location[1] - (-2 if name == 'Orange' else 2), name, color='blue')

    current_dot, = ax.plot(last_location[0], last_location[1], 'bo', markersize=8)       


    plt.savefig(f'./src/location.png')
    # plt.show()


if __name__ == "__main__":
    client = connect()
    db = client.supermarket
    coll = db.location

    with coll.watch() as stream:
        for change in stream:
            last_data = coll.find().sort([('_id', DESCENDING)]).limit(1)
            last_data = [doc for doc in last_data][0]
            last_location = [last_data["x"], last_data["y"]]
            print(last_location)
            drawMap(last_location)