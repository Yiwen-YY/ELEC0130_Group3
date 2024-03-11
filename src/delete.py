def connect():
    from pymongo.mongo_client import MongoClient
    from pymongo.server_api import ServerApi

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

def deleteItem(client, name, amount):
    db = client.supermarket
    coll = db.trolley
    item = coll.find_one({ "name": name })
    stock_quantity = int(item["amount"])

    if (stock_quantity > 1):
        coll.update_one({ "name": name }, { "$set": { 'amount': stock_quantity - amount } })
    if (stock_quantity == 1):
        coll.delete_one(item) 
    print(f"Delete {amount} {name} successfully!")

if __name__ == "__main__":
    deleteItem(connect(), "Apples", 1)