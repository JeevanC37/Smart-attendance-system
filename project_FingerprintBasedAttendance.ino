// Libraries:
/* Add JSON Package and Install: esp8266 by ESP8266 Community */
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <Wire.h>
/* Built-In Library */

#include <Adafruit_GFX.h>
/* Install: "Adafruit GFX Library" by Adafruit */

#include <Adafruit_SSD1306.h>
/* Install: "Adafruit SSD1306" by Adafruit */

#include <Adafruit_Fingerprint.h>
/* Install: "Adafruit Fingerprint Sensor Library by Adafruit" */

// Pin Numbers:
#define FP_Rx_Pin 14 //Connect to Tx of Fingerprint Sensor Module
#define FP_Tx_Pin 12 //Connect to Rx of Fingerprint Sensor Module

// Configuration:
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define I2C_ADDRESS 0x3C  // Common Address: 0x3C / 0x3D

// Software Serial Port:
SoftwareSerial SerialFP(FP_Rx_Pin, FP_Tx_Pin);

// Object: Fingerprint sensor
Adafruit_Fingerprint objFP = Adafruit_Fingerprint(&SerialFP);

// Object: OLED
// The (-1) parameter means that your OLED display doesn’t have a RESET pin.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Objects: Wi-Fi
WiFiClientSecure client;

// Your Wi-Fi credentials:
const char ssid[] = "fl_4k";  // Name of your network (Hotspot or Router name)
const char pass[] = "9731994494"; // Corresponding Password

// Host and HTTPS Port:
const char host[] = "script.google.com";
const int httpsPort = 443;

// Google App Script ID:
String GAS_ID = "AKfycbyjLFsmesSv1XzrfH7AMsqIuqifWBsJCXFirqyV0tuf0hgqFWSoBY0KLXkacWX_O6a8";
const char fingerprint[] = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";

bool isFaculty = false;

void setup() {
  /* Begin serial communication with Arduino and Arduino IDE (Serial Monitor) */
  Serial.begin(9600);
  Serial.println("Fingerprint Identification");

  if (!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true) {  // Don't proceed, loop forever
      yield();
    }
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(10, 26);  // (ColIdx, RowIdx)
  display.println("Connecting");
  display.println("to Wi-Fi..");
  display.display();

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setInsecure();

  display.clearDisplay();
  display.setCursor(10, 26);
  display.println("Connected");
  display.display();

  /* Begin serial communication with Arduino and Fingerprint sensor */
  objFP.begin(57600);

  if (objFP.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (true) {  // Don't proceed, loop forever
      yield();
    }
  }

  objFP.getTemplateCount();
  Serial.print("Sensor contains ");
  Serial.print(objFP.templateCount);
  Serial.println(" templates");
}

void loop() {
  Serial.println("\nWaiting for valid fingerprint...");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Scan");
  display.println("your");
  display.println("finger");
  display.display();

  if (isValidFingerprintDetected()) {
    Serial.print("Fingerprint Found on ID #");
    Serial.print(objFP.fingerID);
    Serial.print(" with confidence of ");
    Serial.println(objFP.confidence);

    display.clearDisplay();
    display.setCursor(0, 0);

    if (!isFaculty && objFP.fingerID == 1) {
      display.println("Faculty");
      display.println("Sign-In");
      display.display();
      isFaculty = true;
      UploadToGoogleSheet(String(objFP.fingerID));
    } else if (!isFaculty && objFP.fingerID != 1) {
      display.println("Invalid");
      display.println("Faculty");
      display.println("ID");
      display.display();
    } else if (isFaculty && objFP.fingerID == 1) {
      display.println("Faculty");
      display.println("Sign-Out");
      display.display();
      isFaculty = false;
      UploadToGoogleSheet(String(objFP.fingerID));
    } else if (isFaculty && objFP.fingerID != 1) {
      display.println("Student");
      display.println(objFP.fingerID);
      display.display();
      UploadToGoogleSheet(String(objFP.fingerID));
    }
    delay(2000);
  } else {
    Serial.println("Reading Fingerprint has Failed!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Reading");
    display.println("Failed");
    display.display();
    delay(2000);
  }
  while (objFP.getImage() != FINGERPRINT_NOFINGER); /* Loop until the finger is on the sensor.*/
}

bool isValidFingerprintDetected() {
  while (objFP.getImage() != FINGERPRINT_OK); /* Loop until the image is taken. */
  if (objFP.image2Tz() != FINGERPRINT_OK) return false;
  if (objFP.fingerFastSearch() != FINGERPRINT_OK)  return false;
  return true;
}


void UploadToGoogleSheet(String fpId) {
  Serial.print("Connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    client.stop();
    return;
  }



  String url = "/macros/s/" + GAS_ID + "/exec?Value1=" + fpId;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  Serial.println("Request sent.");

  while (client.connected()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
    }
  }
  client.stop();
  Serial.println("\n[Disconnected]");
}
