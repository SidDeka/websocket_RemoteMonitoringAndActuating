#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>


#include <LiquidCrystal_I2C.h>




// Wifi Credentials
const char* ssid     = "OnePlusSid";
const char* password = "esp32ece328";


const int button1 = 15;
const int button2 = 4;
bool led_state = false;
bool led_state2= false;


WebSocketsClient webSocket; // websocket client class instance
StaticJsonDocument<100> doc; // Allocate a static JSON document


LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows
void setup() {
  // Connect to local WiFi
  WiFi.begin(ssid, password);
  Serial.begin(115200);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Print local IP address


  delay(2000); // wait for 2s
  lcd.init(); // initialize the lcd
  lcd.backlight();
  //address, port, and URL path
  webSocket.begin("192.168.235.220", 81, "/");
  // webSocket event handler
  webSocket.onEvent(webSocketEvent);
  // if connection failed retry every 5s
  webSocket.setReconnectInterval(5000);


  pinMode(button1, INPUT);
  pinMode(button2, INPUT);



}
void loop() {
  webSocket.loop(); // Keep the socket alive
}
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  // Make sure the screen is clear
  //  u8g2.clearBuffer();
  if (type == WStype_TEXT)
  {
    DeserializationError error = deserializeJson(doc, payload); // deserialize incoming Json String
    if (error) { // Print erro msg if incomig String is not JSON formated
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    const String pin_stat = doc["PIN_Status"]; // String variable tha holds LED status
    const float t = doc["Temp"]; // Float variable that holds temperature
    const float h = doc["Hum"]; // Float variable that holds Humidity
    const float accel_x = doc["accel_x"]; //float variable that holds acceleration
    const float gyro_y = doc["gyro_y"]; //float variable that holds orientation y-axis
    // Print the received data for debugging
    Serial.print(String(pin_stat));
    //Serial.print(String(t));
    //Serial.println(String(h));
    Serial.print(String(accel_x));
    Serial.println(String(gyro_y));

    // Send acknowledgement


    if(digitalRead(button1) == HIGH && led_state == true){
      while(digitalRead(button1) == HIGH){
        delay(10);
      }
      led_state = false;
      webSocket.sendTXT("0");
    }else if(digitalRead(button1) == HIGH && led_state == false){
      while(digitalRead(button1) == HIGH){
        delay(10);
      }
      led_state = true;
      webSocket.sendTXT("1");
    }

        if(digitalRead(button2) == HIGH && led_state2 == true){
      while(digitalRead(button2) == HIGH){
        delay(10);
      }
      led_state2 = false;
      webSocket.sendTXT("2");
    }else if(digitalRead(button2) == HIGH && led_state2 == false){
      while(digitalRead(button2) == HIGH){
        delay(10);
      }
      led_state2 = true;
      webSocket.sendTXT("3");
    }
    /* LED: OFF
       TMP: Temperature
       Hum: Humidity
    */
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Accel_x:"); // Print TMP: on the display
    lcd.print(accel_x); // Print temperature value
    lcd.setCursor(0, 1); // Set the cursor position (0, 40)
    lcd.print("Gyro_y: ");// Print HUM: on the display
    lcd.print(gyro_y); // Print Humidity vsalue
  }
}
