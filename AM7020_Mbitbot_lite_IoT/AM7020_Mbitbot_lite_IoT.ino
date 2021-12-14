/*
 * Generated using BlocklyDuino:
 *
 * https://github.com/MediaTek-Labs/BlocklyDuino-for-LinkIt
 *
 * Date: Mon, 13 Dec 2021 09:26:15 GMT
 */
/*  部份程式由吉哥積木產生  */
/*  https://sites.google.com/jes.mlc.edu.tw/ljj/linkit7697  */
#include <Adafruit_NeoPixel.h>

#include "Wire.h"
#include "U8g2lib.h"
#include "TinyGsmClientSIM7020.h"
#include <Am7020PubSubClient.h>
#include <SoftwareSerial.h>

unsigned long timer = 0;

unsigned long oled_timer = 0;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(2,4,NEO_GRB + NEO_KHZ800);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
typedef TinyGsmSim7020 TinyGsm;
typedef TinyGsmSim7020::GsmClientSim7020 TinyGsmClient;

TinyGsm       modem(Serial1, 5);

void nbConnect(void)
{
    Serial.println(F("Initializing modem..."));
    while (!modem.init() || !modem.nbiotConnect("iot4ga2", 28)) {
        Serial.print(F("."));
    }

    Serial.print(F("Waiting for network..."));
    while (!modem.waitForNetwork()) {
        Serial.print(F("."));
    }
    Serial.println(F(" success"));
}

const char* broker = "io.adafruit.com";
const int port = 1883;
const char* mqtt_id = "icshop_20211213";
const char* mqtt_username = "Zack_Huang";
const char* mqtt_password = "aio_Jwdo12DnsBtJx1GWg4jlrBBGRvHN";

String topic_buff;

String msg_buff;

TinyGsmClient tcpClient(modem);
Am7020PubSubClient  mqttClient(broker, port, tcpClient);

void mqttConnect(void)
{
    Serial.print(F("Connecting to "));
    Serial.print(broker);
    Serial.print(F("..."));

    while (!mqttClient.connect(mqtt_id, mqtt_username, mqtt_password)) {
        Serial.print(F(" fail"));
    }
    Serial.println(F(" success"));
}

SoftwareSerial pms5003tSerial(3, 0);
long pmat10_ = 0;
long pmat25_ = 0;
long pmat100_ = 0;
long Temp_ = 0;
long Humid_ = 0;
char buf[50];

void retrievepm25(){
  int count = 0;
  unsigned char c;
  unsigned char high;
  while (pms5003tSerial.available()) {
     c = pms5003tSerial.read();
     if((count==0 && c!=0x42) || (count==1 && c!=0x4d)){
       break;
     }
     if(count > 27){
       break;
     }
      else if(count == 10 || count == 12 || count == 14 || count == 24 || count == 26) {
       high = c;
     }
     else if(count == 11){
       pmat10_ = 256*high + c;
     }
     else if(count == 13){
       pmat25_ = 256*high + c;
     }
     else if(count == 15){
       pmat100_ = 256*high + c;
     }
      else if(count == 25){
          Temp_ = (256*high + c)/10;
     }
     else if(count == 27){
                  Humid_ = (256*high + c)/10;
    }       count++;
  }  while(pms5003tSerial.available()) pms5003tSerial.read();
}

void mqttCallback(char *topic, byte *payload, unsigned int len)
{
    topic_buff = String(topic);
    msg_buff = "";

    for (int ii = 0;ii < (int)len;ii++) {
        msg_buff += (char)(*(payload + ii));
    }
    Serial.print(F("Message arrived ["));
    Serial.print(topic_buff);
    Serial.print(F("]: "));
    Serial.println(msg_buff);

  tone(14, 255, 300);
  if (topic_buff == "Zack_Huang/feeds/icshop.rgb-l") {
    setWS2812LEDHEX(1, msg_buff);
    pixels.show();
    pixels.show();
  } else if (topic_buff == "Zack_Huang/feeds/icshop.rgb-r") {
    setWS2812LEDHEX(0, msg_buff);
    pixels.show();
    pixels.show();
  }

}

void setWS2812LEDHEX(byte idx, String hex_color) {
long number = strtol( &hex_color[1], NULL, 16);
pixels.setPixelColor(idx, pixels.Color(number >> 16, number >> 8 & 0xFF, number & 0xFF));
}

void setup()
{
  pixels.begin();

  pixels.setBrightness(30);

  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);

  Serial1.begin(115200);

  randomSeed(analogRead(16));

  topic_buff.reserve(100);

  msg_buff.reserve(100);

  mqttClient.setCallback(mqttCallback);

  mqttClient.setKeepAlive(270);

  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 0);
    u8g2.print(String("AM7020 Init ...                   ").c_str());

    u8g2.sendBuffer();
  } while (u8g2.nextPage());
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 0);
    u8g2.print(String("NBIoT Connecting...          ").c_str());

    u8g2.sendBuffer();
  } while (u8g2.nextPage());
  nbConnect();
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 0);
    u8g2.print(String("MQTT Connecting...          ").c_str());

    u8g2.sendBuffer();
  } while (u8g2.nextPage());
  mqttConnect();
  mqttClient.subscribe(String("Zack_Huang/feeds/icshop.rgb-l").c_str());
  mqttClient.subscribe(String("Zack_Huang/feeds/icshop.rgb-r").c_str());
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 0);
    u8g2.print(String("MQTT Connected !            ").c_str());

    u8g2.sendBuffer();
  } while (u8g2.nextPage());
  timer = millis() + 60000;
  pms5003tSerial.begin(9600);

}


void loop()
{
  retrievepm25();
  if (!modem.isNetworkConnected()) {
    u8g2.firstPage();
    do {
      u8g2.setCursor(0, 0);
      u8g2.print(String("NBIoT Connecting...          ").c_str());

      u8g2.sendBuffer();
    } while (u8g2.nextPage());
    nbConnect();
  }
  if (!mqttClient.connected()) {
    u8g2.firstPage();
    do {
      u8g2.setCursor(0, 0);
      u8g2.print(String("MQTT Connecting...          ").c_str());

      u8g2.sendBuffer();
    } while (u8g2.nextPage());
    mqttConnect();
    u8g2.firstPage();
    do {
      u8g2.setCursor(0, 0);
      u8g2.print(String("MQTT Connected !            ").c_str());

      u8g2.sendBuffer();
    } while (u8g2.nextPage());
    mqttClient.subscribe(String("Zack_Huang/feeds/icshop.rgb-l").c_str());
    mqttClient.subscribe(String("Zack_Huang/feeds/icshop.rgb-r").c_str());
  }
  mqttClient.loop();
  if (millis() >= timer) {
    timer = millis() + 60000;
    mqttClient.publish(String("Zack_Huang/feeds/icshop.pm1").c_str(), String(pmat10_).c_str());
    mqttClient.publish(String("Zack_Huang/feeds/icshop.pm2-5").c_str(), String(pmat25_).c_str());
    mqttClient.publish(String("Zack_Huang/feeds/icshop.pm10").c_str(), String(pmat100_).c_str());
  }
  if (millis() >= oled_timer) {
    oled_timer = millis() + 150;
    u8g2.firstPage();
    do {
      u8g2.setCursor(0, 0);
      u8g2.print(String("MQTT Connected !            ").c_str());
      u8g2.drawLine(0, 11, 128, 11);
      u8g2.setCursor(0, 20);
      u8g2.print(String(String("PM1.0: ")+String(pmat10_)).c_str());
      u8g2.setCursor(0, 35);
      u8g2.print(String(String("PM2.5: ")+String(pmat25_)).c_str());
      u8g2.setCursor(0, 50);
      u8g2.print(String(String("PM10 : ")+String(pmat100_)).c_str());

      u8g2.sendBuffer();
    } while (u8g2.nextPage());
  }
}