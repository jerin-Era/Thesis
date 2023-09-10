#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

WiFiClient wifiClient;

// Replace with your Wi-Fi credentials
const char* ssid = "Jerin";
const char* password = "12121212";

// Replace with the URL you want to make a request to
String url = "http://192.168.4.1/get031?message="; // Example API URL
String fullUrl;

#include <Wire.h>
#include <Adafruit_VL53L0X.h>

Adafruit_VL53L0X lox = Adafruit_VL53L0X();


#include <SoftwareSerial.h>
#include <TinyGPS++.h>

SoftwareSerial gpsSerial(D3, D4); // RX, TX
TinyGPSPlus gps;

#include <SPI.h>
#include <SD.h>



float latitude;
float longitude;
String locationVal;
unsigned long timestamp;
float distance;
String saveData;


int txtFile = 1;
String nextFile;

String fileNameFull;
int fileNameSerial = 0;

void setup() {
  Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");

  // Initialize SD card
  if (!SD.begin(15)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialization");
  delay(100);

  gpsSerial.begin(9600);

  Wire.begin();

  //  if (!lox.begin()) {
  //    Serial.println("Failed to boot VL53L0X");
  //    while (1);
  //  }



  if (!lox.begin()) {
    Serial.println("Failed to boot VL53L0X");
    while (1);
  }


  delay(100);

  int biggestFileNumber = findBiggestFileNumber("/");
  Serial.print("The biggest file number is: ");
  Serial.println(biggestFileNumber);
  //readAllFiles("/");

  delay(100);

  String fileNameString = String(biggestFileNumber + 1) + ".txt";
  const char* fileName = fileNameString.c_str();
  nextFile = fileName;
  Serial.println(nextFile);
}

void loop() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        //Serial.println("GPS is valid");
        // Print latitude, longitude, and altitude
        // Serial.print("Latitude: ");
        latitude = gps.location.lat();
        //Serial.println(latitude);
        // Serial.print("Longitude: ");

        longitude = gps.location.lng();
        //Serial.println(longitude);

        // Get UTC timestamp
        //timestamp = gps.time.value();
        //        Serial.print("UTC Timestamp: ");
        //        Serial.println(timestamp);

        int year = gps.date.year();
        int month = gps.date.month();
        int day = gps.date.day();

        int hour = gps.time.hour()+6;
        int minute = gps.time.minute();
        int second = gps.time.second();


        String Syear = String(year);
        String Smonth = String(month);
        String Sday = String(day);

        String Shour = String(hour);
        String Sminute = String(minute);
        String Ssecond = String(second);

        String timeDate = Sday + "-" + Smonth + "-" + Syear + ", " + Shour + ":" + Sminute + ":" + Ssecond;
        Serial.println(timeDate);

        // Adjust timestamp to Bangladesh Standard Time (BST)
       // unsigned long bstTimestamp = timestamp + (6 * 3600); // Add 6 hours (6 * 3600 seconds)

        //        Serial.print("BD Timestamp: ");
        //        Serial.println(bstTimestamp);

        locationVal = ", lat: " + String(latitude) + ", log: " + String(longitude) + ", time: " + timeDate;
        //Serial.println(locationVal);
        delay(100);

        VL53L0X_RangingMeasurementData_t measure;

        lox.rangingTest(&measure, false);

        if (measure.RangeStatus != 4) {  // 4 = out of range error
          // Serial.print("Distance(mm): ");
          distance = measure.RangeMilliMeter;
          //Serial.println(measure.RangeMilliMeter);
        } else {
          //Serial.println("Out of range");
          distance = 0;
        }
        delay(100);

        saveData = "distance: " + String(distance) + "mm" + locationVal;
        Serial.println(saveData);
        writeToFile("/" + nextFile + "", saveData);


        if (WiFi.status() == WL_CONNECTED) {
          HTTPClient http;  //Object of class HTTPClient
          int fileCheck1 = nextFile.toInt();
          int fileCheck2 = nextFile.toInt();
          if (fileCheck1 == fileCheck2) {
            fileNameSerial = fileNameSerial + 1;
            fileNameFull = nextFile + String(fileNameSerial);
          }
          //String added = nextFile;
          fullUrl = url + fileNameFull;
          Serial.println(fullUrl);
          http.begin(wifiClient, fullUrl);// get the result (the error code)
          int httpCode = http.GET();
          //Check the returning code
          if (httpCode > 0) {
            String response;
            response = http.getString();
            Serial.println(response);
            http.end();   //Close connection
            if (httpCode != 200) {
              // Bad Response Code
              //errorMessage = "Error response (" + String(httpCode) + "): " + response;
              Serial.println("Error");
              return;
            }
            // Close the connection
            http.end();
          }

          // Wait for some time before making the next request
          //delay(5000);
        }


        delay(15000);
      }
    }
  }
}

void writeToFile(String filename, String data) {
  File myFile = SD.open(filename, FILE_WRITE);

  if (myFile) {
    myFile.println(data);
    myFile.close();
    Serial.println("Data written to file!");
  } else {
    Serial.println("Error opening file!");
  }
}

int findBiggestFileNumber(const char* path) {
  File root = SD.open(path);

  int biggestNumber = 0;

  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      // No more files
      break;
    }

    if (!entry.isDirectory() && entry.name()[0] != '.') {
      String fileName = entry.name();
      fileName.remove(fileName.lastIndexOf('.'));
      int fileNumber = fileName.toInt();

      if (fileNumber > biggestNumber) {
        biggestNumber = fileNumber;
      }
    }

    entry.close();
  }

  root.close();

  return biggestNumber;
}
