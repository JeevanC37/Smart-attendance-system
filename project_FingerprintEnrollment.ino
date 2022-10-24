// Libraries:
#include <Wire.h>
/* Built-In Library */

#include <Adafruit_GFX.h>
/* Install: "Adafruit GFX Library" by Adafruit */

#include <Adafruit_SSD1306.h>
/* Install: "Adafruit SSD1306" by Adafruit */

#include <Adafruit_Fingerprint.h>
/* Install: "Adafruit Fingerprint Sensor Library by Adafruit" */

// Pin Numbers:
#define FP_Rx_Pin 14  //Connect to Tx of Fingerprint Sensor Module
#define FP_Tx_Pin 12  //Connect to Rx of Fingerprint Sensor Module

// Configuration:
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define I2C_ADDRESS 0x3C  // Common Address: 0x3C / 0x3D

// Software Serial Port:
SoftwareSerial SerialFP(FP_Rx_Pin, FP_Tx_Pin);

// Objects - Using sensor without password:
Adafruit_Fingerprint objFP = Adafruit_Fingerprint(&SerialFP);

// Object: OLED
// The (-1) parameter means that your OLED display doesn’t have a RESET pin.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Variables:
unsigned int id;

void setup() {
  /* Begin serial communication with Arduino and Arduino IDE (Serial Monitor) */
  Serial.begin(9600);
  Serial.println("Fingerprint Enrollment.");

  /* Begin serial communication with Arduino and Fingerprint sensor */
  objFP.begin(57600);

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
  display.println("Enrolling");
  display.display();
  
  if (objFP.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (true){
      yield();
    }
  }

  objFP.getTemplateCount();
  Serial.print("Sensor contains ");
  Serial.print(objFP.templateCount);
  Serial.println(" templates");
}

void loop() {
  Serial.println("Ready to enroll a fingerprint.");
  Serial.println("Please type the ID number (from 1 to 127) you want to save the fingerprint.");
  id = readNumber();
  Serial.print("Enrolling ID #");
  Serial.println(id);
  if (doFingerprintEnroll()) {
    Serial.print("Fingerprint Enrollment Successful on ID #");
    Serial.println(id);
  } else {
    Serial.println("Fingerprint Enrollment Failed!");
    Serial.println("Please try again!");
  }
  Serial.println();
  delay(4000);
}

unsigned int readNumber() {
  unsigned int num = 0;
  while (num < 1 || num > 127) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

bool doFingerprintEnroll() {
  uint8_t n;
  Serial.print("\nWaiting for valid finger to enroll as #"); Serial.println(id);
  do {
    n = objFP.getImage();
  } while (n == FINGERPRINT_NOFINGER);
  if (n == FINGERPRINT_OK) {
    Serial.println("First Image Taken Successfully.");
  } else {
    printFPErrorMessage(n);
    return false;
  }

  n = objFP.image2Tz(1);
  if (n == FINGERPRINT_OK) {
    Serial.println("First Image Converted Successfully.");
  } else {
    printFPErrorMessage(n);
    return false;
  }

  Serial.println("Please remove your finger from the sensor.");
  while (objFP.getImage() != FINGERPRINT_NOFINGER);

  Serial.println("\nPlace same finger again.");
  do {
    n = objFP.getImage();
  } while (n == FINGERPRINT_NOFINGER);
  if (n == FINGERPRINT_OK) {
    Serial.println("Second Image Taken Successfully.");
  } else {
    printFPErrorMessage(n);
    return false;
  }

  n = objFP.image2Tz(2);
  if (n == FINGERPRINT_OK) {
    Serial.println("Second Image Converted Successfully.");
  } else {
    printFPErrorMessage(n);
    return false;
  }

  Serial.print("\nCreating model for #");  Serial.println(id);
  n = objFP.createModel();
  if (n == FINGERPRINT_OK) {
    Serial.println("Prints matched! Model Created Successfully.");
  } else {
    printFPErrorMessage(n);
    return false;
  }

  Serial.print("\nStoring model into #");  Serial.println(id);
  n = objFP.storeModel(id);
  if (n == FINGERPRINT_OK) {
    Serial.println("Fingerprint Stored Successfully.");
  } else {
    printFPErrorMessage(n);
    return false;
  }

  Serial.println("Please remove your finger from the sensor.");
  while (objFP.getImage() != FINGERPRINT_NOFINGER);
  return true;
}

void printFPErrorMessage(uint8_t n) {
  switch (n) {
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("ERROR: Communication Error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("ERROR: Imaging Error");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("ERROR: Image Too Messy");
      break;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("ERROR: Could not find fingerprint features");
      break;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("ERROR: Could not find fingerprint features");
      break;
    case FINGERPRINT_ENROLLMISMATCH:
      Serial.println("Fingerprints did not match");
      break;
    case FINGERPRINT_BADLOCATION:
      Serial.println("Could not store in that location");
      break;
    case FINGERPRINT_FLASHERR:
      Serial.println("Error writing to flash");
      break;
    default:
      Serial.println("ERROR: Unknown Error");
      break;
  }
}
