const ws = new WebSocket('ws://localhost:4000');

let itemData;
let trolleyData;
let buttonData;
let locationData;

ws.onmessage = function (event) {
    const data = JSON.parse(event.data);
    // console.log(data.data);

    if (document.getElementById('collection2Button').classList.contains('active')) {
        displayTable(trolleyData);
    }

    obj = data.data[0]
    if (obj && obj.stock_quantity) {
        itemData = data.data;
        console.log(itemData);
        if (document.getElementById('collection1Button').classList.contains('active')) {
            const text = document.getElementById('total-price');
            text.style.display = "none";
            displayTable(itemData);
        }
    } else if (obj && obj.amount) {
        trolleyData = data.data;
        if (document.getElementById('collection2Button').classList.contains('active')) {
            displayTable(trolleyData);
            showTotalPrice(trolleyData);
        }
    } else if (obj && obj.activate) {
        buttonData = data.data;
        // console.log("button");
        // console.log(buttonData);
        // console.log(buttonData[0]["activate"]);
        length = buttonData.length
        if (buttonData[length - 1]["activate"] == "1") {
            document.getElementById('staffButton').classList.add('active')
        } else {
            document.getElementById('staffButton').classList.remove('active');
        }
    } else if (obj && obj.x) {
        locationData = data.data;
        if (document.getElementById('collection3Button').classList.contains('active')) {
            const text = document.getElementById('total-price');
            text.style.display = "none";
            console.log("map");
            showLocation();
        }
    }

    dataDic = {};
    dataDic["items"] = itemData;
    console.log("do");
    console.log(itemData);
    dataDic["trolley"] = trolleyData;
    dataDic["button"] = buttonData;
    dataDic["location"] = locationData;
    document.getElementById("downloadButton").removeEventListener("click", handleDownload);
    document.getElementById("downloadButton").addEventListener("click", handleDownload);
};

document.getElementById('collection1Button').addEventListener('click', function () {
    this.classList.add('active');
    document.getElementById('collection2Button').classList.remove('active');
    document.getElementById('collection3Button').classList.remove('active');
});

document.getElementById('collection2Button').addEventListener('click', function () {
    this.classList.add('active');
    document.getElementById('collection1Button').classList.remove('active');
    document.getElementById('collection3Button').classList.remove('active');
});

document.getElementById('collection3Button').addEventListener('click', function () {
    this.classList.add('active');
    document.getElementById('collection1Button').classList.remove('active');
    document.getElementById('collection2Button').classList.remove('active');
});

function sendData() {
    ws.send(JSON.stringify({ collection: 'items' }));
    ws.send(JSON.stringify({ collection: 'trolley' }));
    ws.send(JSON.stringify({ collection: 'button' }));
    ws.send(JSON.stringify({ collection: 'location' }));
}

setInterval(sendData, 1000);


function displayTable(data) {
    const tableContainer = document.getElementById('tableContainer');
    tableContainer.innerHTML = '';

    for (let i = 0; i < data.length; i++) {
        delete data[i]._id
    }

    const table = document.createElement('table');
    const thead = document.createElement('thead');
    const tbody = document.createElement('tbody');

    const headerRow = document.createElement('tr');
    for (const key in data[0]) {
        const th = document.createElement('th');
        th.textContent = key;
        headerRow.appendChild(th);
    }
    thead.appendChild(headerRow);

    data.forEach(row => {
        const tr = document.createElement('tr');
        for (const key in row) {
            const td = document.createElement('td');
            td.textContent = row[key];
            tr.appendChild(td);
        }
        tbody.appendChild(tr);
    });

    table.appendChild(thead);
    table.appendChild(tbody);
    tableContainer.appendChild(table);
}

// function handleDownload(jsonData) {
//     const csv = convertToCSV(jsonData);
//     const blob = new Blob([csv], { type: 'text/csv' });
//     const url = window.URL.createObjectURL(blob);
//     const link = document.createElement('a');
//     link.href = url;
//     link.download = 'data.csv';
//     document.body.appendChild(link);
//     link.click();
//     document.body.removeChild(link);
// }

function convertToCSV(jsonData) {
    const separator = ',';
    const fields = Object.keys(jsonData[0]);
    let csv = fields.join(separator) + '\n';
    jsonData.forEach((item) => {
        const values = fields.map((field) => item[field]);
        csv += values.join(separator) + '\n';
    });
    return csv;
}

// function downloadData() {
//     // 向服务器发送请求获取 supermarket 数据库中所有 collection 的数据
//     fetch('http://localhost:4000/database-data?database=supermarket')
//         .then(response => response.json())
//         .then(data => {
//             // 对每个 collection 的数据进行处理，分别生成并下载对应的 CSV 文件
//             data.forEach(collection => {
//                 const csvContent = convertToCSV(collection.data);
//                 downloadCSV(csvContent, `${collection.name}.csv`);
//             });
//         })
//         .catch(error => {
//             console.error('Error:', error);
//         });
// }
function handleDownload() {
    downloadData(dataDic)
}
function downloadData(dataDic) {
    for (let key in dataDic) {
        downloadCSV(dataDic[key], key)
    }
}

function downloadCSV(jsonData, filename) {
    const csv = convertToCSV(jsonData);
    const csvData = new Blob([csv], { type: 'text/csv;charset=GB2312;' }); // 使用 GB2312 编码格式
    const link = document.createElement('a');
    link.href = window.URL.createObjectURL(csvData);
    link.download = filename;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
}


function selectMenuItem(menuItem) {
    var allMenuItems = document.querySelectorAll('.btn-secondary');
    allMenuItems.forEach(function (item) {
        item.classList.remove('focus');
    });

    menuItem.classList.add('focus');
}

function showLocation() {

    const tableContainer = document.getElementById('tableContainer');
    tableContainer.innerHTML = '';

    const image = document.createElement('img');

    image.src = './location.png';
    image.alt = 'Placeholder Image';

    tableContainer.appendChild(image);
    console.log("pic");
}

function showTotalPrice(data) {
    const text = document.getElementById('total-price');
    text.innerHTML = '';
    totalPrice = 0;
    for (var i = 0; i < data.length; i++) {
        price = data[i]["price"];
        price = price.substring(1);
        price = parseFloat(price);
        amount = data[i]["amount"];
        amount = parseFloat(amount);
        totalPrice = totalPrice + price * amount;
    }

    text.style.display = "block";
    const newText = document.createTextNode('Total: ￡' + totalPrice);
    text.appendChild(newText);
}

