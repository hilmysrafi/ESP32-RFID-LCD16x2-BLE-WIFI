#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);  

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 34

MFRC522 mfrc522(SS_PIN, RST_PIN);

const char* ssid = "House2White"; //Nama WiFi
const char* password = "listrikm4h4l"; //Password WiFi

const char *getMAC_data;
//const char *getdev_name;

String MAC_data[100];
int scanTime = 10; //In seconds
int device_count = 0;
int RSSI_data[100];
//String dev_name[100];

BLEScan* pBLEScan;
BLEAdvertisedDevice device;
String content;

  class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
     void onResult(BLEAdvertisedDevice device) {
     
    }
};

void setup()
{
  Serial.begin(115200); 
  Serial.println("Scanning...");
  while (!Serial);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Nama: ");
  lcd.setCursor(0,1);
  lcd.print("Saldo: ");
  SPI.begin();    
  mfrc522.PCD_Init();   
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting..");
  }
  Serial.print("Wifi connected..");
  Serial.println("");
}

void loop()
{
   if (WiFi.status() == WL_CONNECTED) {
    Scanrfid();
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    device_count = foundDevices.getCount();
    Serial.printf("Devices count: %d\n", device_count);
    Serial.print("Devices found: \n");
    for (uint32_t i = 0; i < device_count; i++)
    {
      BLEAdvertisedDevice device = foundDevices.getDevice(i);
//      if (device.haveName()){
//      getdev_name= device.getName().c_str();
//      }
      RSSI_data[i] = device.getRSSI();
      getMAC_data = BLEUtils::buildHexData(nullptr,(uint8_t *)device.getManufacturerData().data(),device.getManufacturerData().length());
      MAC_data[i]= String(getMAC_data);
//      dev_name[i] = String(getdev_name);
     
      if (MAC_data[i] == "484d4c24985d01dc" )
     {
//      Serial.printf("Device Name = %s ", getdev_name);
//      Serial.println();
      Serial.printf("MAC Address = %s", getMAC_data);
      Serial.println();
      Serial.printf("RSSI = %d ", RSSI_data[i]);
      datable(i);
     }
     } 
    Serial.println();
    Serial.println("Scan done!");
    pBLEScan->clearResults();
    Scanrfid();
  }
  lcd.setCursor(0,0);
  lcd.print("Nama: ");
  lcd.setCursor(0,1);
  lcd.print("Saldo: ");
}

void Scanrfid(){
      if ( ! mfrc522.PICC_IsNewCardPresent())
      {
        return;
      }
      if ( ! mfrc522.PICC_ReadCardSerial())
      {
        return;
      }
      Serial.println();
      Serial.print("UID tag = ");
      content = "";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++)
      {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      content.toUpperCase();
      Serial.println();
      rfidadd();
      rfidread();
  }

void datable(int i){
  HTTPClient http;
      http.begin("http://codemantul.my.id/TugasAkhir/ble.php?manufacturer=" + String(MAC_data[i]) + "&rssi=" + String(RSSI_data[i]));
      int httpCode = http.GET();
      if (httpCode > 0) {
        String payload = http.getString();
        //Serial.print("RESPONE = ");
        //Serial.println(payload);
      }
       http.end();
}
void rfidadd(){
  HTTPClient http;
      http.begin("http://codemantul.my.id/TugasAkhir/rfidadd.php?id=" + String(content));
      int httpCode = http.GET();
      if (httpCode > 0) {
        String payload = http.getString();
        Serial.print("RESPONE = ");
        Serial.println(payload);
      }
       http.end();
}
void rfidread(){
  HTTPClient http;
  http.begin("http://codemantul.my.id/TugasAkhir/rfidread.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST("id=" + content);
  delay(100);
  if (httpResponseCode > 0) {
    String response = http.getString();
    char json[500];
    response.toCharArray(json, 500);
    StaticJsonDocument<200> doc;
    deserializeJson(doc, json);

    const char* status = doc["Detail"]["Status"];
    const char* nama = doc["Detail"]["Data User"]["nama"];
    int saldo = doc["Detail"]["Data User"]["saldo"];
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Nama: ");
    lcd.print(nama);
    lcd.setCursor(0,1);
    lcd.print("Saldo: Rp");
    lcd.print(saldo);
    delay(3000);
    lcd.clear();
    
    Serial.println("=============== HASIL PARSING RESPONSE ==========");
    Serial.print("Status RFID = ");
    Serial.println(status);
    Serial.print("Nama Pengguna = ");
    Serial.println(nama);
    Serial.print("Saldo = Rp ");
    Serial.println(saldo);
    Serial.println("==================================================");
     
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  delay(300);
  http.end();
}
