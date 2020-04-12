//////////////////////////////////////////////////////////////////////////////////////////////////////////

//Wifi Setup

#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"
#include <stdio.h>
#include <SPI.h>

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;

// Initialize the Wifi client library
WiFiSSLClient client;

unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 30L * 1000L; // delay between updates, in milliseconds

//////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Adafruit_NeoPixel.h>

//DEFINE MATRIX 32x32
#define WIDTH 32
#define HEIGHT 32
#define PIN 6
#define NUMPIXELS 1024

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//color of the dots ({red, green, blue} max value 255 each]
int colorOfPattern[3] = {25, 0, 25};
//color of background
int colorOfBackground[3] = {0, 0, 0};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup for proximity sensor
// #include <SPI.h>
//
// int trigPin = 12;
// int echoPin = 13;
//
// // function prototype to define default timeout value
// static unsigned int newPulseIn(const byte pin, const byte state, const unsigned long timeout = 1000000L);
// // using a macro to avoid function call overhead
// #define WAIT_FOR_PIN_STATE(state) \
// while (digitalRead(pin) != (state)) { \
//   if (micros() - timestamp > timeout) { \
//     return 0; \
//   } \
// }
// static unsigned int newPulseIn(const byte pin, const byte state, const unsigned long timeout) {
//   unsigned long timestamp = micros();
//   WAIT_FOR_PIN_STATE(!state);
//   WAIT_FOR_PIN_STATE(state);
//   timestamp = micros();
//   WAIT_FOR_PIN_STATE(!state);
//   return micros() - timestamp;
// }
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Button Setup

const int buttonPin = 2; //Pause Button
int buttonState;
int lastButtonState = LOW;

const int buttonPin10 = 10; //Reset array Button
int buttonState10;
int lastButtonState10 = LOW;

const int buttonPin11 = 11; // Request Array from Processing Button
int buttonState11;
int lastButtonState11 = LOW;

const int buttonPin12 = 12; //State Changer Button
int buttonState12;
int lastButtonState12 = LOW;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

//////////////////////////////////////////////////////////////////////////////////////////////////////////

//LED Setup

const int ledPin3 = 3; // RED Led Pin
int ledState3 = LOW;
const int ledPin5 = 5; // Green Led Pin
int ledState5 = LOW;
const int ledPin9 = 9; // Yellow Led Pin
int ledState9 = LOW;


//LED RGB setup

int Red_PIN = 14;
int Green_PIN = 15;
int Blue_PIN = 16;

//brightness of the led

// float bright;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

//2d array

boolean array1[WIDTH][HEIGHT] ;
boolean cellbuffer[WIDTH][HEIGHT];

//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Probability

float Probability = 45;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Variables for timer

boolean pause = false;
int interval = 200;
int lastRecordedTime = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

//Variable for State
//implementing 3 states
//state 0 will Play game of life
//state 1 will display weather information on Led from OpenWeatherMap via Wifi
//state 2 will be an interactive LED - using processing and a webcam -

int gstate = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  //Start Serial
  Serial.begin(115200);

  //Attempt to Connect to wifi
  connect2Wifi();

  //Starting Dot Matrix
  pixels.begin();
  pixels.clear();
  pixels.show();
  initializeCells();
}

void loop() {

  button_changeState();
  stateLedColor();

  switch (gstate) {

    case 0:
      if (millis() - lastConnectionTime > postingInterval)
      {
        httpRequest();
      }
      break;

    case 1:
      pauseGame();
      pauseButton();
      resetButton();
      break;

    case 2:
      pixels.clear();
      for (int i = 0; i < WIDTH * HEIGHT; i++) {
        pixels.setPixelColor(i, pixels.Color(5, 0, 5));
      }
      pixels.show();
      break;


  }
}

void initializeCells() {
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      float state = random (100);
      if (state > Probability) {
        state = 0;
      }
      else {
        state = 1;
      }
      array1[i][j] = int(state); // Save state of each cell
    }
  }
}

void GameofLife_ram() {
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      cellbuffer[i][j] = array1[i][j];
    }
  }
  // Visit each cell:
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      int neighbours = 0; //  count the neighbours
      for (int ii = i - 1; ii <= i + 1; ii++) {
        for (int jj = j - 1; jj <= j + 1; jj++) {
          if (((ii >= 0) && (ii < WIDTH)) && ((jj >= 0) && (jj < HEIGHT))) { //check if out of bounds
            if (!((ii == i) && (jj == j))) { // Make sure to to check against self
              if (cellbuffer[ii][jj] == 1) {
                neighbours ++; // Check alive neighbours and count them
              }
            } // End of if
          } // End of if
        } // End of jj loop
      } //End of ii loop

      // We've checked the neigbours - apply rules
      if (cellbuffer[i][j] == 1) { // The cell is alive: kill it if
        if (neighbours < 2 || neighbours > 3) {
          array1[i][j] = 0; // Die unless it has 2 or 3 neighbours
        }
      }
      else { // The cell is dead: make it live if necessary
        if (neighbours == 3 ) {
          array1[i][j] = 1; // Only if it has 3 neighbours
        }
      } // End of if
    } // End of j loop
  } //End of i loop
} //End of function

void statePlot() {
  int alive;
  //int bright1 = proxibrightness();
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {

      if (j & 0x1) { //is odd
        if (array1[i][j] == 1) {
          pixels.setPixelColor(((j * WIDTH) + (WIDTH - 1 - i)), pixels.Color(colorOfPattern[0], colorOfPattern[1], colorOfPattern[2]));
          alive++;
        }
        else {
          pixels.setPixelColor(((j * WIDTH) + (WIDTH - 1 - i)), pixels.Color(colorOfBackground[0], colorOfBackground[0], colorOfBackground[0]));
        }
      }
      else {
        if (array1[i][j] == 1) {
          pixels.setPixelColor(i + (j * WIDTH) , pixels.Color(colorOfPattern[0], colorOfPattern[1], colorOfPattern[2]));
          alive++;
        }
        else {
          pixels.setPixelColor(i + (j * WIDTH), pixels.Color(colorOfBackground[0], colorOfBackground[0], colorOfBackground[0]));
        }
      }
    }
  }
  pixels.show();
  if (alive <= int(NUMPIXELS * 0.038)) {
    initializeCells();
  }
}

void pauseButton() {
  pinMode(ledPin5, OUTPUT);

  int reading = digitalRead(buttonPin);
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle Pause if the new button state is HIGH
      if (buttonState == HIGH) {
        pause = !pause;
      }
    }
  }
  lastButtonState = reading;

  if (pause == false) {
    ledState5 = HIGH;
    digitalWrite(ledPin5, ledState5);
  } else if (pause == true) {
    ledState5 = LOW;
    digitalWrite(ledPin5, ledState5);
  }
}

void resetButton() {

  int reading = digitalRead(buttonPin10);
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState10) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState10) {
      buttonState10 = reading;
      // only toggle Reset if the new button state is HIGH
      if (buttonState10 == HIGH) {
        initializeCells();
      }
    }
  }
  lastButtonState10 = reading;
}

void button_requestArrayFromSerial() {

  int reading = digitalRead(buttonPin11);
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState11) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState11) {
      buttonState11 = reading;
      // Send request to Processing if the new button state is HIGH
      if (buttonState11 == HIGH) {
        //start request
      }
    }
  }
  lastButtonState11 = reading;
}

void button_changeState() {

  int reading = digitalRead(buttonPin12);

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState12) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState12) {
      buttonState12 = reading;

      // only toggle if the new button state is HIGH
      if (buttonState12 == HIGH) {
        pixels.clear();
        gstate = gstate + 1;
      }
    }
  }
  lastButtonState12 = reading;
}

void stateLedColor() {  //RGB LED color depending on gstate.
  pinMode(Red_PIN, OUTPUT);
  pinMode(Green_PIN, OUTPUT);
  pinMode(Blue_PIN, OUTPUT);

  if (gstate == 0) {        //Color Red if gstate=0
    analogWrite(Red_PIN, 255);
    analogWrite(Green_PIN, 0);
    analogWrite(Blue_PIN, 0);
  } else if (gstate == 1) { //Color Green if gstate=1
    analogWrite(Red_PIN, 0);
    analogWrite(Green_PIN, 255);
    analogWrite(Blue_PIN, 0);
  } else if (gstate == 2) { //Color Blue if gstate=2
    analogWrite(Red_PIN, 0);
    analogWrite(Green_PIN, 0);
    analogWrite(Blue_PIN, 255);
  } else if (gstate >= 3) {  //Reset if gstate=0 to maintain only 3 switch cases
    gstate = 0;
  }
}

void pauseGame() {
  if (millis() - lastRecordedTime > interval) {
    if (!pause) {
      GameofLife_ram();
      statePlot();
      lastRecordedTime = millis();
    }
  }
}

void connect2Wifi() {
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin9, OUTPUT);
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0")
  {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED)
  {
    digitalWrite(ledPin3,HIGH);
    digitalWrite(ledPin9, LOW);
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the status:
  digitalWrite(ledPin3,LOW);
  digitalWrite(ledPin9,HIGH);
  printWifiStatus();

}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void httpRequest() {
  // if there's a successful connection:
  if (client.connect("api.openweathermap.org", 443))
  {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET /data/2.5/weather?q=spain,madrid&appid=4483727fd8bd14c96f505ab796963203 HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("Connection: close");
    client.println();

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
    if (strcmp(status + 9, "200 OK") != 0)
    {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    }

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders))
    {
      Serial.println(F("Invalid response"));
      return;
    }
    // Allocate the JSON document
    // Use arduinojson.org/v6/assistant to compute the capacity.
    const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(0) + 2 * JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 2 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(14) + 280;
    DynamicJsonDocument doc(capacity);

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, client);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    int weatherId = doc["weather"][0]["id"].as<int>();
    float weatherTemperature = doc["main"]["temp"].as<float>();
    int weatherHumidity = doc["main"]["humidity"].as<int>();

    //Disconnect
    client.stop();

    Serial.println(F("Response:"));
    Serial.print("Weather: ");
    Serial.println(weatherId);
    Serial.print("Temperature: ");
    Serial.println(weatherTemperature);
    Serial.print("Humidity: ");
    Serial.println(weatherHumidity);
    Serial.println();

    if (weatherId == 800)   //clear
    {
      for (int i = 0; i < WIDTH * HEIGHT; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 10, 10));
      }
      pixels.show();
    }
    else
    {
      switch (weatherId / 100)
      {
        case 2:     //Thunderstorm
          for (int i = 0; i < WIDTH * HEIGHT; i++) {
            pixels.setPixelColor(i, pixels.Color(25, 0, 25));
          }
          pixels.show();
          break;

        case 3:     //Drizzle
          for (int i = 0; i < WIDTH * HEIGHT; i++) {
            pixels.setPixelColor(i, pixels.Color(0, 0, 10));
          }
          pixels.show();
          break;
        case 5:     //Rain
          for (int i = 0; i < WIDTH * HEIGHT; i++) {
            pixels.setPixelColor(i, pixels.Color(0, 0, 25));
          }
          pixels.show();
          break;

        case 7:     //Sun with clouds
          for (int i = 0; i < WIDTH * HEIGHT; i++) {
            pixels.setPixelColor(i, pixels.Color(20, 20, 0));
          }
          pixels.show();
          break;
        case 8:     //clouds
          for (int i = 0; i < WIDTH * HEIGHT; i++) {
            pixels.setPixelColor(i, pixels.Color(20, 12, 16));
          }
          pixels.show();
          break;

        default:    //Sun with clouds
          for (int i = 0; i < WIDTH * HEIGHT; i++) {
            pixels.setPixelColor(i, pixels.Color(20, 20, 0));
          }
          pixels.show();
          break;
      }
    }
    // note the time that the connection was made:
    lastConnectionTime = millis();
  }
  else
  {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

//void portTest() {       test1
//
//  pinMode(ledPin9, OUTPUT);
//
//  //  RECEIVE DATA FROM PROCESSING CODE TO USE
//  // Wiring/Arduino code:
//
//  while (Serial.available()) { // If data is available to read,
//    serVal += Serial.read();
//  }
//  for (int i = 0; i < cols; i++) {
//    for (int j = 0; j < rows; j++) {
//      int index = serVal.indexOf(","); //We find the next comma
//      array1[i][j] = atol(serVal.substring(0, index).c_str());
//      serVal = serVal.substring(index + 1); //Remove the number from the string
//    }
//  }
////    if (serVal == 'r') {
////      digitalWrite(ledPin9, HIGH);
////      delay(150);
////      digitalWrite(ledPin9, LOW);
////      serVal = "";
////    }
////    else {
////      digitalWrite(ledPin9, LOW);
////    }
//}

//void establishContact() {
//  while (Serial.available() <= 0) {
//    Serial.write('A');   // send a capital A
//    delay(300);
//  }
//}

//int proxibrightness() {
//  long duration, distance;
//  digitalWrite(trigPin, HIGH);
//  delayMicroseconds(10);
//  digitalWrite(trigPin, LOW);
//  duration = newPulseIn(echoPin, HIGH);
//  distance = (duration / 1) / 29.1;
//  duration = newPulseIn(echoPin, LOW);
//  delay(10);
//
////  int average=0;
////  int bright;
////  for (int i = 0; i < 20; i++) {
////    average = average + distance;
////  }
////  average = average / 20;
////  if (average < 0) {
////    average = 0;
////  } else if (average > 400) {
////    average = 400;
////  }
////  if (average < 100) {
////    bright = bright + 2;
////    if (bright > 100) {
////      bright = 100;
////    }
////  }
////  if (average > 100) {
////    bright = bright - 2;
////    if (bright < 0) {
////      bright = 0;
////    }
////  }
//  //mapping reverse range for bright to be at max when close and minimum at far.
//  bright = map(distance,0,400,40,0);
//  return bright;
//}
