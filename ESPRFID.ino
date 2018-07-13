#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define USE_SERIAL Serial

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_1_PIN        10         // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 2
#define SS_2_PIN        8          // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 1

#define NR_OF_READERS   2

byte ssPins[] = {SS_1_PIN, SS_2_PIN};

ESP8266WiFiMulti WiFiMulti;

MFRC522 mfrc522[NR_OF_READERS];   // Create MFRC522 instance.

void setup() {
    Serial.begin(115200);//belum mencari lebih lanjut lagi seingatku mengikuti baudrate yg lebih rendah tapi kali ini aku pilih ke yg lebih tinggi
    
    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    Serial.println("ini di setup");//cuma untuk debugging
    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFiMulti.addAP("WiFu", "rahasia123");
  
    while (!Serial);

    SPI.begin();

    for (uint8_t reader = 0; reader < NR_OF_READERS; reader++){
      mfrc522[reader].PCD_Init{ssPins[reader], RST_PIN};
    }
}

void loop() {

  for (uint8_t reader = 0; reader< NR_OF_READERS; reader++){
   if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      String SUID = getUID(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      delay(1000);
      
      checkDAtabase(SUID);

      
    }
  } 
}

void checkDatabase(String UID){
     // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");
        String Addr = "192.168.43.77";
        http.begin("http://"+Addr+"/Lavorus/lockDoorValidation.php?"+UID); //HTTP
        Serial.println("ini request GET");
        USE_SERIAL.print("[HTTP] GET...\n");
        
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                Serial.println("ini sudah handle GET");
                String payload = http.getString();
                USE_SERIAL.println(payload);
                if (payload == "Permitted"){
                    Serial.println("Access Denied ");
                   }
                else if (payload == "Blocked"){
                    Serial.println("Access Granted ");
                   }
            }
        } else {
          Serial.println("ahh gagal");
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }

    delay(10000);
}

String getUID(byte *buffer, byte bufferSize){
  String UID_HEX ="";

  for(byte i=0;i<bufferSize;i++){
      String part_UID_HEX = String(buffer[i],HEX);
      UID_HEX = StringAdd(UID_HEX,part_UID_HEX);      
    }
  return UID_HEX;
  }

 String stringAdd(String string1,String string2){
    int len1 = string1.length();
    int len2 = string2.length();

    char char1[len1];
    char char2[len2];

    char ret[len1+len2];

    string1.toCharArray(char1, len1+1);
    string2.toCharArray(char2, len2+1);

    for (int i=0; i <= len1 + 1;i++){
        ret[i] = char1[i];
      }
    for (int i=0; i <= len2 + 1;i++){
        ret[i +len1] = char2[i];
      }
      String retval = ret;
      return retval;
  }

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? "0" : "");
    Serial.print(buffer[i], HEX);
  }
  Serial.println();
}
