#include <WiFi.h> // Include WIFi Library for ESP32
#include <WebServer.h> // Include WebSwever Library for ESP32
#include <ArduinoJson.h> // Include ArduinoJson Library
#include <Adafruit_Sensor.h>
//#include <Adafruit_BME280.h>
#include <WebSocketsServer.h>  // Include WebSockets Library by Markus Sattler
#include <Adafruit_MPU6050.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>




//Adafruit_BME280 bme; // I2C


Adafruit_MPU6050 mpu;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
 
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);




const char* ssid     = "OnePlusSid";
const char* password = "esp32ece328";


int interval = 1000; // virtual delay
unsigned long previousMillis = 0; // Tracks the time since last event fired


const char web[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Arduino and ESP32 Websocket</title>
  <meta name='viewport' content='width=device-width, initial-scale=1.0' />
  <meta charset='UTF-8'>
  <style>
    body {
      background-color: #E6D8D5;
      text-align: center;
    }
  </style>
</head>
<body>
  <h1>accel_x: <span id='accel_x'>-</span></h1>
  <h1>gyro_y: <span id='gyro_y'>-</span></h1>
  <h1>LED 1: <span id='message1'>-</span></h1><button type='button' id='BTN_1'>
    <h1>ON</h1>
  </button><button type='button' id='BTN_2'>
    <h1>OFF</h1>
  </button>
  <h1>LED 2: <span id='message2'>-</span></h1><button type='button' id='BTN_3'>
    <h1>ON</h1>
  </button><button type='button' id='BTN_4'>
    <h1>OFF</h1>
  </button>
</body>
<script>
  var Socket;
  document.getElementById('BTN_1').addEventListener('click', button_1_pressed);
  document.getElementById('BTN_2').addEventListener('click', button_2_pressed);
  document.getElementById('BTN_3').addEventListener('click', button_3_pressed);
  document.getElementById('BTN_4').addEventListener('click', button_4_pressed);
  function init() {
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    Socket.onmessage = function(event) {
      processCommand(event);
    };
  }
  function processCommand(event) {
    var obj = JSON.parse(event.data);
    document.getElementById('message1').innerHTML = obj.PIN_Status1;
    document.getElementById('message2').innerHTML = obj.PIN_Status2;


    document.getElementById('accel_x').innerHTML = obj.accel_x;
    document.getElementById('gyro_y').innerHTML = obj.gyro_y;
    console.log(obj.PIN_Status1);
    console.log(obj.PIN_Status2);
    console.log(obj.accel_x);
    console.log(obj.gyro_y);
  }
  function button_1_pressed() {
    Socket.send('1');
  }
  function button_2_pressed() {
    Socket.send('0');
  }
  function button_3_pressed() {
    Socket.send('2');
  }
  function button_4_pressed() {
    Socket.send('3');
  }
  window.onload = function(event) {
    init();
  }
</script>
</html>)rawliteral";




String jsonString; // Temporary storage for the JSON String
String pin_status1 = ""; // Holds the status of the pin
String pin_status2 = ""; // Holds the status of the pin
float accel_x; // holds the acceleration value
float gyro_y;// holds the orientation in x-axis value


WebServer server(80);  // create instance for web server on port "80"
WebSocketsServer webSocket = WebSocketsServer(81);  //create instance for webSocket server on port"81"


void setup() {
  // put your setup code here, to run once:
  pinMode(26, OUTPUT); // Set PIN 26 As output(LED Pin)
  pinMode(27,OUTPUT); // Set PIN 27 As output(LED Pin)


  Serial.begin(115200); // Init Serial for Debugging.
  delay(1000);
 
 while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens




  Serial.println("Adafruit MPU6050 test!");




  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");




  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }




  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);



  Serial.println("");
  delay(100);




  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);


 
 
  WiFi.begin(ssid, password); // Connect to Wifi
  while (WiFi.status() != WL_CONNECTED) { // Check if wifi is connected or not
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  // Print the IP address in the serial monitor windows.
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // Initialize a web server on the default IP address. and send the webpage as a response.
  server.on("/", []() {
    server.send(200, "text\html", web);
  });
  server.begin(); // init the server
  webSocket.begin();  // init the Websocketserver
  webSocket.onEvent(webSocketEvent);  // init the webSocketEvent function when a websocket event occurs


  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(15);
  display.clearDisplay();
 
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
 
 
  display.println("accel_x: ");
  display.setCursor(40, 0);
  display.println(accel_x);




  display.println("gyro_y: ");
  display.setCursor(40, 8);
  display.println(gyro_y);


  display.display(); // display OLED content


 
}


void displayUpdate(){
  display.clearDisplay();
 
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
 
 
  display.println("accel_x: ");
  display.setCursor(44, 0);
  display.println(accel_x);




  display.println("gyro_y: ");
  display.setCursor(40, 8);
  display.println(gyro_y);


  display.display(); // display OLED content
}


void update_mpu(){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  accel_x = a.acceleration.x; // Read temperature as Celsius (the default)
  gyro_y = g.gyro.y; // Read temperature as Fahrenheit (isFahrenheit = true
}	






void update_webpage()
{
  StaticJsonDocument<100> doc;
  // create an object
  JsonObject object = doc.to<JsonObject>();
  object["PIN_Status1"] = pin_status1 ;
  object["PIN_Status2"] = pin_status2;
  object["accel_x"] = accel_x ;
  object["gyro_y"] = gyro_y ;
  serializeJson(doc, jsonString); // serialize the object and save teh result to teh string variable.
  //Serial.println( jsonString ); // print the string for debugging.
  webSocket.broadcastTXT(jsonString); // send the JSON object through the websocket
  jsonString = ""; // clear the String.
}


void loop() {


  displayUpdate();
  update_mpu();
  server.handleClient();  // webserver methode that handles all Client
  webSocket.loop(); // websocket server methode that handles all Client
  unsigned long currentMillis = millis(); // call millis  and Get snapshot of time
  if ((unsigned long)(currentMillis - previousMillis) >= interval) { // How much time has passed, accounting for rollover with subtraction!
  previousMillis = currentMillis;   // Use the snapshot to set track time until next event
  update_webpage();
  }
}






// This function gets a call when a WebSocket event occurs
void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED: // enum that read status this is used for debugging.
      Serial.print("WS Type ");
      Serial.print(type);
      Serial.println(": DISCONNECTED");
      break;
    case WStype_CONNECTED:  // Check if a WebSocket client is connected or not
      Serial.print("WS Type ");
      Serial.print(type);
      Serial.println(": CONNECTED");
      if (digitalRead(26) == HIGH) {  //check if pin 22 is high or low
        pin_status1 = "ON";
        update_webpage(); // update the webpage accordingly
      }
      else{                          
        pin_status1 = "OFF"; //check if pin 22 is high or low
        update_webpage();// update the webpage accordingly
      }
      if (digitalRead(27) == HIGH) {  //check if pin 22 is high or low
        pin_status2 = "ON";
        update_webpage(); // update the webpage accordingly
      }else{                          
        pin_status2 = "OFF"; //check if pin 22 is high or low
        update_webpage();// update the webpage accordingly
      }
      break;
    case WStype_TEXT: // check responce from client
      Serial.println(); // the payload variable stores teh status internally
      Serial.println(payload[0]);
      if (payload[0] == '1') {
        pin_status1 = "ON";
        digitalWrite(26, HIGH);
           
      }
      else if (payload[0] == '0') {
        pin_status1 = "OFF";
        digitalWrite(26, LOW);       
     
      }
      else if (payload[0] == '2') {
        pin_status2 = "ON";
        digitalWrite(27, HIGH); 
             
      }
      else if (payload[0] == '3') {
        pin_status2 = "OFF";
        digitalWrite(27, LOW);     
      }
      break;
      }
  }

