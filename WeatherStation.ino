#include <SPI.h>
#include <Wire.h>
#include <Timer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <DHT.h>

// DHT-22 pressure sensor type and pin definitions
#define DHT_TYPE DHT22   // DHT 22  (AM2302)
#define DHT_PIN 6

//Define the Arduino pin numbers for the TFT screen
#define TFT_CS     10
#define TFT_RST    9
#define TFT_DC     8
#define TFT_SCLK   13
#define TFT_MOSI   11

// Library initializations
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
DHT dht = DHT(DHT_PIN, DHT_TYPE);

// Pin number of the LED
const int ledPin = 2;

// Pin numbers for the refresh button
const int refreshButtonPin = 4;
// Status indication for the buttons - 0 = not pressed, 1 = pressed
int refreshButtonState = 0;
// The previous reading from the input pin
int lastRefreshButtonState = LOW;
// The last time the output pin was toggled
long lastRefreshDebounceTime = 0;
const long debounceDelay = 50;

// Initialize the Timer
Timer timer;
// Timer refresh period in milliSeconds - 1 sec = 1000 milliSec
unsigned long refreshPeriod = 30000; // 30 seconds

// Preferences - Set the test mode
const bool testMode = true;

//The setup function is called once at startup of the sketch
void setup() {
  
  // Initialize the Serial Monitor in test mode
  if (testMode) {
    Serial.begin(19200);
    Serial.println("Weather Station Started");
  }

  // Initialize the LED pin
  pinMode(ledPin, OUTPUT);
  // Initialize the button pins
  pinMode(refreshButtonPin, INPUT);

  // Initialize the TFT screen
  tft.initR(INITR_BLACKTAB);

  // Display the screen scaffolding - title & borders
  displayScreen();

  // Initialize the DHT-22 pressure sensor
  dht.begin();

  // Initialize the BMP-085 pressure sensor
  if (!bmp.begin()) {
    Serial.print("No BMP-085 pressure sensor was detected!");
  } else {    
    retrieveAndDrawSensorData();
  }

  // Refresh the screen according to the predefined refresh period
  timer.every(refreshPeriod, refreshScreen);
}

// The loop function is called in an endless loop
void loop() {

  /////////////////////
  // Button Handling //
  /////////////////////

  // Read the state of the switch into a local variables
  int refreshReading = digitalRead(refreshButtonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing reset the debouncing timer
  if (refreshReading != lastRefreshButtonState) {
    lastRefreshDebounceTime = millis();
  }

  if ((millis() - lastRefreshDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (refreshReading != refreshButtonState) {
      refreshButtonState = refreshReading;

      if (refreshButtonState == HIGH) {
        refreshButtonPressed();
      }
    }
  }

  // Save the reading.  Next time through the loop, it'll be the lastButtonState:
  lastRefreshButtonState = refreshReading;

  //////////////////////
  // Other operations //
  //////////////////////

  // Put the indicator LED into its default status
  digitalWrite(ledPin, LOW);

  // Update the Timer
  timer.update();
}

void refreshButtonPressed() {
  if (testMode) {
    //Indicate with the LED that the button was pressed
    digitalWrite(ledPin, HIGH);
  }
  retrieveAndDrawSensorData();
}

void refreshScreen() {
  if (testMode) {
    //Indicate with the LED that the button was pressed
    digitalWrite(ledPin, HIGH);
  }
  retrieveAndDrawSensorData();
}

void displayScreen() {
  //Clear the screen
  tft.fillScreen(ST7735_BLACK);

  // Clear the screen and set text size
  tft.setTextWrap(false);
  tft.setTextSize(1);

  // Display the header of the menu - the header is the first item
  tft.setCursor(20, 5);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  tft.println("Weather Station");

  // Draw the screen border
  tft.drawRect(0, 0, 128, 160, ST7735_WHITE);

  // Draw separation line
  tft.drawLine(0, 16, tft.width() - 1, 16, ST7735_WHITE);
}

void retrieveAndDrawSensorData() {

  // Retrieve the pressure information from the BMP-085 pressure sensor
  sensors_event_t event;
  bmp.getEvent(&event);

  if (event.pressure) {
    float pressure;
    float temperature;
    float altitude;

    pressure = event.pressure;
    
    // Retrieve the temperature and altitude information
    bmp.getTemperature(&temperature);
    altitude = bmp.pressureToAltitude(pressure, SENSORS_PRESSURE_SEALEVELHPA);

    // Retrieve temperature and pressure information from the DHT-22 sensor
    float dhtTemperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Display sensor data on the Weather Station screen
    drawSensorData(pressure, dhtTemperature, humidity);

    // Print out debug infromation to the Serial Monitor in test mode
    if (testMode) {
      Serial.println("Sensor data:");
      Serial.print("Pressure: ");
      Serial.print(pressure);
      Serial.println(" milliBar");

      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" celsius");

      Serial.print("Altitude: ");
      Serial.print(altitude);
      Serial.println(" meter");
      
      Serial.print("DHT Temperature: ");
      Serial.print(dhtTemperature);
      Serial.println(" celsius");
      
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");
      Serial.println("");
    }
  } else {
    Serial.println("BMP-085 pressure sensor error!");
  }
}

void drawSensorData(float pressure, float temperature, float humidity) {

  // Draw numerical data 
  tft.setTextSize(2);

  tft.setTextColor(ST7735_CYAN, ST7735_BLACK);
  tft.setCursor(25, 30);
  tft.print(pressure);

  tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
  tft.setCursor(35, 70);
  tft.print(humidity);

  tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
  tft.setCursor(35, 110);
  tft.print(temperature);

  // Draw unit data
  tft.setTextSize(1);

  tft.setTextColor(ST7735_CYAN, ST7735_BLACK);
  tft.setCursor(35, 50);
  tft.print(" milliBar");

  tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
  tft.setCursor(30, 90);
  tft.print(" humidity %");

  tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
  tft.setCursor(35, 130);
  tft.print(" celsius");
}
