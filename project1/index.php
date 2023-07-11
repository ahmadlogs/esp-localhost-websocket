<html>
	<head>
		<title>Project1</title>
	</head>
<body>
	<h4>Temperature: <span id="temperature">0 °C</span></h4>
	<h4>Humidity: <span id="humidity">0 </span></h4>
	<button id="ledButton" onclick="toggle()">OFF</button>

	<script>
		//REPLACE WITH YOUR COMPUTER IP ADDRESS WHERE THE WEBSOCKET SERVER IS RUNNING
		var socket = new WebSocket('ws://localhost:81');
		
		socket.onmessage = function(event) { 
			console.log(event.data);
			const data = event.data.split(":");
			
			const msg		= data[0] || "";
			const sensor	= data[1] || "";
			
			if(sensor == "led"){ 
				var button = document.getElementById("ledButton");
				button.innerHTML = msg == "1" ? "ON" : "OFF";
			}
			else if(sensor == "dht"){ 
				var parts = msg.split(",");
				var temperature = parts[0];
				var himidity = parts[1];
				
				document.getElementById("temperature").innerHTML = temperature + " °C";
				document.getElementById("humidity").innerHTML = himidity + " %";
			}
		};
		
		function toggle() { 
			var button = document.getElementById("ledButton");
			var status = button.innerHTML === "OFF" ? "1" : "0";
			socket.send(status + ":led:esp:localhost");
		}
	</script>

</body>
</html>
