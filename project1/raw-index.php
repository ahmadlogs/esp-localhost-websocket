<?php

/* 
ws://192.168.0.100/test
ws://localhost:8080
*/

//REPLACE WITH YOUR COMPUTER IP ADDRESS WHERE THE WEBSOCKET SERVER IS RUNNING
define('WEBSOCKET_URL', 'ws://localhost');


$servername = "localhost";
$username = "root";
$password = "";
$dbname = "sensor_db";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

$sql = "SELECT temperature FROM dht11 ORDER BY id DESC LIMIT 1";
$result = $conn->query($sql);

if ($result->num_rows > 0) {
    $row = $result->fetch_assoc();
    $temperature = $row["temperature"];
} else {
    $temperature = "N/A";
}

$conn->close();
?>

<!DOCTYPE html>
<html>
<head>
    <title>Temperature Monitor</title>
    <script>
        var socket = new WebSocket("<?php echo WEBSOCKET_URL; ?>"); 

        socket.onmessage = function(event) {
            console.log(event.data);
			
			var jsonData = event.data;
            var data = JSON.parse(jsonData);
			
			if(data.receiver == "localhost" || data.sender == "localhost" ){
				if(data.sensor == "led"){
					console.log(data.msg);
					var button = document.getElementById("ledButton");
					button.innerHTML = data.msg == "1" ? "ON" : "OFF";
				}
				else if(data.sensor == "dht"){
					console.log(data.msg);
					var parts = data.msg.split(",");
					var temperature = parts[0];
					var himidity = parts[1];
					document.getElementById("temperature").innerHTML = temperature + " °C";
					//storeTemperature(data);
				}
			}
        };

        function toggleLED() {
            var button = document.getElementById("ledButton");
            var status = button.innerHTML === "OFF" ? "1" : "0";
            socket.send(status + ":led:esp:localhost");
        }

        /*function storeTemperature(temperature) {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "store_temperature.php", true);
            xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
            xhr.onreadystatechange = function() {
                if (xhr.readyState === 4 && xhr.status === 200) {
                    console.log("Temperature stored successfully.");
                }
            };
            xhr.send("temperature=" + temperature);
        }*/
    </script>
</head>
<body>
    <h1>Temperature: <span id="temperature"><?php echo $temperature; ?> °C</span></h1>
    <button id="ledButton" onclick="toggleLED()">OFF</button>
</body>
</html>
