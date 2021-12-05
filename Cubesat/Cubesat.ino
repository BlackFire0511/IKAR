#define SD_CS_PIN 8
#define SIZE_MASS 50
#define PIN_MQ5  A0
#define ONE_WIRE_BUS 10
#define VL6180X_ADDRESS 0x29

//Импорт библиотек
#include <SPI.h> // Для работы с интерфейсом SPI
#include <SD.h> //Для работы с картой памяти
#include <LiquidCrystal.h> //Для работы с дисплеем
#include <OneWire.h> //Шина OneWire
#include <Wire.h> //Шина I2C
#include <SparkFun_VL6180X.h> //Работа с датчиком освещённости
#include <DallasTemperature.h> //Работа с датчиком температуры
#include <TroykaDHT.h> //Работа с датчиком температуры и влажности
#include <TroykaMQ.h> //Работа с датчиком газов
#include <TroykaGPS.h> //Работа с модулем GPS
#include <SoftwareSerial.h> //Работа с виртуальным Serial

String dataString = "";
String str = "";
char str1[20];
int light = 0;
int TempIn = 0;
int TempOut = 0;
int humidity = 0;
int lpg = 0;
int methane = 0;
int c_alt = 0;
float lati = 0.0;
float longi = 0.0;
float alti = 0.0;
uint32_t timer = 0;

MQ5 mq5(PIN_MQ5);
DHT dht(9, DHT21);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);
VL6180x ts(VL6180X_ADDRESS);
SoftwareSerial mySerial(6, 7);
LiquidCrystal lcd(11, 12, 5, 4, 3, 2);
#define GPS_SERIAL mySerial
GPS gps(GPS_SERIAL);

void setup() {
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  SD.begin(SD_CS_PIN);
  lcd.begin(16, 2);
  mq5.calibrate();
  sensor.begin();
  sensor.setResolution(12);
  ts.VL6180xInit();
  ts.VL6180xDefautSettings();
  while (!Serial) {
    GPS_SERIAL.begin(115200);
    GPS_SERIAL.write("$PMTK251,9600*17\r\n");
    GPS_SERIAL.end();
    GPS_SERIAL.begin(9600);
  }
}

void saveSD() {
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  dataFile.println(dataString);
  dataFile.close();
}

void loop() {
  if (gps.available()) {
    gps.readParsing();
    if (gps.getState() == GPS_OK) {
      lati = gps.getLatitudeBase10();
      longi = gps.getLongitudeBase10();
      alti = gps.getAltitude();
    }
  }
  lpg = mq5.readLPG();
  methane = mq5.readMethane();
  if (dht.getState() == GPS_OK) {
    TempOut = dht.getTemperatureC();
    humidity = dht.getHumidity();
  }
  sensor.requestTemperatures();
  TempIn = sensor.getTempCByIndex(0);
  light = ts.getAmbientLight(GAIN_1);
  if (alti - c_alt == 1 && alti > 25) {
    c_alt = alti;
    dataString = "Altitude: " + String(alti) + ", " + "Latitude: " + String(lati) + ", " + "Longitude: " + String(longi) + ", " + "TempIn: " + String(TempIn) + ", " + "TempOut: " + String(TempOut) + ", " + "Humidity: " + String(humidity) + ", " + "Light: " + String(light) + ", " + "LPG: " + String(lpg) + ", " + "Methane: " + String(methane) + ";" + '\n';
    saveSD();
  } else if (alti - c_alt == -1 && alti > 10) {
    c_alt = alti;
    dataString = "Altitude: " + String(alti) + ", " + "Latitude: " + String(lati) + ", " + "Longitude: " + String(longi) + ", " + "TempIn: " + String(TempIn) + ", " + "TempOut: " + String(TempOut) + ", " + "Humidity: " + String(humidity) + ", " + "Light: " + String(light) + ", " + "LPG: " + String(lpg) + ", " + "Methane: " + String(methane) + ";" + '\n';
    saveSD();
  }
  if (millis() - timer == 500) {
    lcd.clear();
    str = "Hum: " + String(humidity) + "% TOut" + String(TempOut);
    str.toCharArray(str1, 20);
    lcd.setCursor(0, 0); lcd.print("Hello");
    str = "Altitude: " + String(alti) + "m " + "TIn" + String(TempIn);
    str.toCharArray(str1, 20);
    lcd.setCursor(0, 1); lcd.print("Hello");
  }

  if (methane >= 2 || LPG >= 85) {
    analogWrite(A1, 1);
    analogWrite(A2, 1);
  } else if (humidity > 70) {
    analogWrite(A1, 1);
    analogWrite(A2, 0);
  } else if (temp > 10) {
    analogWrite(A1, 0);
    analogWrite(A2, 1);
  } else {
    analogWrite(A1, 0);
    analogWrite(A2, 0);
  }
}
