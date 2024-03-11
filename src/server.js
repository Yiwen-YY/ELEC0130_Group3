const express = require('express');
const http = require('http');
const WebSocket = require('websocket').server;
const cors = require('cors');
const { MongoClient } = require('mongodb');

const app = express();
const server = http.createServer(app);
const wsServer = new WebSocket({ httpServer: server });

const PORT = process.env.PORT || 4000;
const uri = 'mongodb+srv://yangwyyw:9C3BPOnItsviJvUR@cluster0.nrvqeva.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0';
const dbName = 'supermarket';

let client;

async function connectToMongoDB() {
    try {
        client = await MongoClient.connect(uri, { useUnifiedTopology: true });
        console.log('Connected to MongoDB Atlas');
    } catch (err) {
        console.error('Error connecting to MongoDB Atlas:', err);
        process.exit(1);
    }
}

connectToMongoDB();

app.use(cors());

wsServer.on('request', function (request) {
    const connection = request.accept(null, request.origin);

    connection.on('message', async function (message) {
        if (message.type === 'utf8') {
            const data = JSON.parse(message.utf8Data);
            if (data.collection === 'items' || data.collection === 'trolley' || data.collection === "button" || data.collection === "location") {
                try {
                    const database = client.db(dbName);
                    const collection = database.collection(data.collection);
                    const tableData = await collection.find({}).toArray();
                    connection.send(JSON.stringify({ data: tableData }));
                } catch (err) {
                    console.error('Error fetching data from MongoDB:', err);
                    connection.send(JSON.stringify({ error: 'Internal Server Error' }));
                }
            }
        }
    });
});

app.use(express.static('public'));

server.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});
