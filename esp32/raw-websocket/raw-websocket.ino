/*************************************************************************************************
 *  Created By: Tauseef Ahmad
 *  Created On: 11 July, 2023
 *  
 *  YouTube Video: 
 *  My Channel: https://www.youtube.com/@AhmadLogs
 ***********************************************************************************************/


#include <ArduinoJson.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>

#include <DHT.h>
#define DHTPIN 19
#define DHTTYPE DHT11 
DHT dht11(DHTPIN, DHTTYPE);

using namespace websockets;

WebsocketsClient socket;

//enter the ssid and password of the network where the websocket server is connected
const char* ssid = "ENTER_SSID"; 
const char* password = "ENTER_PASSWORD";

const char* websocketServer = "ws://IP_ADDRESS:PORT/";
boolean connected = false;

const int ledPin = 2;
bool ledState = false;

const int buttonPin = 27;

float temperature = 0;
float humidity = 0;

void setup() {
  
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  
  dht11.begin();
  
  connectWiFi();

  connectToWebSocket();

  // Set the event handler function
  socket.onEvent(handleEvent);

  // Set the message handler function
  socket.onMessage(handleMessage);
}

void loop() {
  socket.poll();

  if (digitalRead(buttonPin) == LOW) {
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    delay(200);  // Delay to debounce the button
    socket.send(String(ledState) + ":led:localhost:esp");
  }

  String dhtData = Load_DHT11_Data();
  //String dhtData = String(temperature) + "," + String(humidity);
  socket.send(dhtData + ":dht:localhost:esp");
  delay(2000);
  
  if(!connected) {
    Serial.println("Attempting to reconnect to WebSocket server...");
    connectToWebSocket();
  }
}


void connectWiFi() {
  WiFi.mode(WIFI_OFF);
  delay(1000);
  //This line hides the viewing of ESP as wifi hotspot
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    
  Serial.print("connected to : "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
}


// Function to connect to the WebSocket server
void connectToWebSocket() {
  connected = socket.connect(websocketServer);

  if (connected) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
  }
}

void reconnectWebsocket() {
  static unsigned long lastReconnectAttempt = 0;
  const unsigned long reconnectInterval = 5000; // 5 seconds

  unsigned long currentMillis = millis();
  if (!connected && (currentMillis - lastReconnectAttempt >= reconnectInterval)) {
    Serial.println("Attempting to reconnect to WebSocket server...");
    connectToWebSocket();
    lastReconnectAttempt = currentMillis;
  }
}

// Function to handle WebSocket messages
void handleMessage(WebsocketsMessage message) {
  Serial.println(message.data());

  // Create a JSON document to store the parsed data
  StaticJsonDocument<200> jsonDoc;

  // Parse the JSON data
  DeserializationError error = deserializeJson(jsonDoc, message.data());

  // Check if parsing succeeded
  if (error) {
    Serial.print("Parsing JSON failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Access the values in the JSON document
  String msg = jsonDoc["msg"];
  String sensor  = jsonDoc["sensor"];
  String receiver= jsonDoc["receiver"];
  String sender  = jsonDoc["sender"];

  Serial.printf("msg      = %s\n", msg);
  Serial.printf("sensor   = %s\n", sensor);
  Serial.printf("receiver = %s\n", receiver);
  Serial.printf("sender   = %s\n", sender);

  if(receiver == "esp"){
    if(sensor == "led"){
      Serial.printf("LED controlled\n");
      ledState = msg.toInt();
      digitalWrite(ledPin, ledState);
    }
  }
}

// Function to handle WebSocket events
void handleEvent(WebsocketsEvent event, WSInterfaceString data) {
  switch (event) {
    case WebsocketsEvent::ConnectionOpened:
      Serial.println("WebSocket connection established");
      socket.send("Hello, server!");
      connected = true;
      break;
    case WebsocketsEvent::ConnectionClosed:
      Serial.println("WebSocket connection closed");
      connected = false;
      break;
    case WebsocketsEvent::GotPing:
      Serial.println("Received ping");
      break;
    case WebsocketsEvent::GotPong:
      Serial.println("Received pong");
      break;
    default:
      break;
  }
}

String Load_DHT11_Data() {
  //-----------------------------------------------------------
  temperature = dht11.readTemperature(); //Celsius
  humidity = dht11.readHumidity();
  //-----------------------------------------------------------
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    temperature = 0;
    humidity = 0;
  }
  //-----------------------------------------------------------
  Serial.printf("Temperature: %d °C\n", temperature);
  Serial.printf("Humidity: %d %%\n", humidity);

  return String(temperature) + "," + String(humidity);
}
