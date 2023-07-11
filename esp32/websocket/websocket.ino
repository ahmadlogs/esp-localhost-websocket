#include <WiFi.h>
#include <ArduinoWebsockets.h> //https://github.com/gilmaimon/ArduinoWebsockets/tree/master
#include <DHT.h>
using namespace websockets;
WebsocketsClient socket;
const char* websocketServer = "ws://IP_ADDRESS:PORT/";
boolean connected = false;

//enter the ssid and password of the network where the websocket server is connected
const char* ssid = "ENTER_SSID"; 
const char* password = "ENTER_PASSWORD";

#define DHTPIN 19
#define DHTTYPE DHT11
DHT dht11(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;

const int ledPin = 2;
bool ledState = false;
const int buttonPin = 0;

void setup() {
  Serial.begin(115200);
  dht11.begin();
  connectWiFi();
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  connectToWebSocket();

  socket.onMessage(handleMessage);
  socket.onEvent(handleEvent);
}


void loop() {
  if(!connected) { 
    Serial.println("Connecting to WebSocket server");
    connectToWebSocket();
  }
  socket.poll();

  String dhtData = Load_DHT11_Data();
  socket.send(dhtData + ":dht:localhost:esp");
  delay(1000);

  if (digitalRead(buttonPin) == LOW) { 
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    socket.send(String(ledState) + ":led:localhost:esp");
    delay(200);
  }
}

void handleMessage(WebsocketsMessage message) { 
  Serial.println(message.data());
  String data = message.data();

  String status = parseData(data, 1);
  String sensor = parseData(data, 2);

  if(sensor == "led") { 
    ledState = status.toInt();
    digitalWrite(ledPin, ledState);
  }
}

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

void connectToWebSocket() { 
  connected = socket.connect(websocketServer);
  if (connected) { 
    Serial.println("Connected");
  }
  else { 
    Serial.println("Connection failed.");
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
  Serial.println("Temperature: " + String(temperature) + " °C");
  Serial.println("Humidity: " + String(humidity) + " %%");

  return String(temperature) + "," + String(humidity);
}

String parseData(String data, int index) {
  String result = "";
  int currentIndex = 1;
  int start = 0;
  int end = data.indexOf(":");

  while (end != -1 && currentIndex <= index) {
    if (currentIndex == index) {
      result = data.substring(start, end);
      break;
    }

    start = end + 1;
    end = data.indexOf(":", start);
    currentIndex++;
  }

  return result;
}
