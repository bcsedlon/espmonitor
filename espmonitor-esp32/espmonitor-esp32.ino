#include "Arduino.h"

//TODO:
//#include "soc/soc.h"
//#include "soc/rtc_cntl_reg.h"

#include "libraries/WiFiManager/src/WiFiManager.h"

#include <WiFiClientSecure.h>
//#include "libraries/UniversalTelegramBot.h"
#include <UniversalTelegramBot.h>
//#include "libraries/TelegramCertificate.h"
//#define BOTtoken "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
//#define BOTtoken "1810161402:AAHkQ4VhJmWGgcNC9XGtQEgz53ZJ2Zb-SR8"

char botChatId[32];
char lastBotChatId[32];
char botToken[56];
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

//#define DRD_TIMEOUT 10
//#define DRD_ADDRESS 4
//#include "libraries/DoubleResetDetector.h"
//DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

//#define ESP8266
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "libraries/ESP8266HTTPClient.h"
#include <ESP8266HTTPUpdateServer.h>
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
#else
#include <FS.h>
#include <Update.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "libraries/OLED/SSD1306.h"
#include "libraries/OLED/OLEDDisplayUi.h"
WebServer httpServer(80);
#endif

unsigned long commInterval;
unsigned long reconnNum;
int reconnCount = 0;

#define SCREENSAVER 60

unsigned long commMillis;
unsigned long secMillis;
unsigned long screenSaverMillis;
unsigned long ledMillis;
unsigned long secCounter;

WiFiClient espClient;

#ifndef ESP8266
#define DRAWMESSAGE(display, message) (drawMessage(&display, message))
#else
#define DRAWMESSAGE(display, message) ()
#endif

#include <WiFiClient.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

//#define SD_CS_PIN 22
#ifdef SD_CS_PIN
#include <FS.h>
#include "libraries/SD/src/SD.h"
#include <SPI.h>
#endif

#define ONEWIREBUS_PIN 0
#ifdef ONEWIREBUS_PIN
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(ONEWIREBUS_PIN);
DallasTemperature oneWireSensors(&oneWire);
#endif

#define LED0_PIN 16

#define CONFIG_WIFI_PIN 2
int inputPins[] = {36, 39, 34, 35, 32, 33, 25, 26, 27, 14, 12, 13, 23, 22, 21, 19, 18, 17, 15};
//int inputPins[] = {27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27};
//int inputPins[] = {39, 36, 35, 34, 33, 32, 27, 26, 25, 23, 22, 21, 19, 18, 17, 16, 15, 14, 13, 2};

//#define DHT_PIN 12
#ifdef DHT_PIN
//#define DHTTYPE DHT11	// DHT 11
//#define DHTTYPE DHT22	// DHT 22  (AM2302)
//#define DHTTYPE DHT21	// DHT 21 (AM2301)
#include "libraries/DHT.h"
DHT dht(DHT_PIN, 11);
#endif

#ifdef MQTT
#include <PubSubClient.h>
PubSubClient mqttClient(espClient);
std::atomic_flag mqttLock = ATOMIC_FLAG_INIT;
String mqttRootTopic;
int mqttState;
//const char* mqtt_server = "broker.hivemq.com";//"iot.eclipse.org";
//const char* mqtt_server = "iot.eclipse.org";
#define MQTT_CLIENTID   "espmonitor-"
#define ROOT_TOPIC		"espmonitor/"
#define LEVEL_VAL_TOPIC	"/level/val"
#define LEVEL_MAX_TOPIC	"/level/max"
#define LEVEL_MIN_TOPIC	"/level/min"
#define A_TOPIC 	"/A"
#define B_TOPIC 	"/B"
#define C_TOPIC 	"/C"
#define D_TOPIC 	"/D"
char msg[20];
#endif

#ifdef NTP
#include "libraries/NTPClient.h"
#include "libraries/Timezone.h"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone CE(CEST, CET);
#endif

#define DEVICES_NUM	24
#define DEVICES_DIGITAL_NUM 19

#define OUTPUT_BIT 0
#define MANUAL_BIT 1
#define CMD_BIT 2
#define UNACK_BIT 3
#define RUNONCE_BIT 4
#define PREVOUTPUT_BIT 5

#define SAMPLES 16

#ifdef SD_CS_PIN
bool loadFromSdCard(String path) {
	String dataType = "text/plain";
	/*
	if(path.endsWith("/")) path += "index.htm";
	if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
	else if(path.endsWith(".htm")) dataType = "text/html";
	else if(path.endsWith(".css")) dataType = "text/css";
	else if(path.endsWith(".js")) dataType = "application/javascript";
	else if(path.endsWith(".png")) dataType = "image/png";
	else if(path.endsWith(".gif")) dataType = "image/gif";
	else if(path.endsWith(".jpg")) dataType = "image/jpeg";
	else if(path.endsWith(".ico")) dataType = "image/x-icon";
	else if(path.endsWith(".xml")) dataType = "text/xml";
	else if(path.endsWith(".pdf")) dataType = "application/pdf";
	else if(path.endsWith(".zip")) dataType = "application/zip";
	*/
	File dataFile = SD.open(path.c_str());
	//if(dataFile.isDirectory()){
	//	path += "/index.htm";
	//  dataType = "text/html";
	//  dataFile = SD.open(path.c_str());
	//}
	if (!dataFile)
		return false;
	//if (httpServer.hasArg("download")) dataType = "application/octet-stream";
	if (httpServer.streamFile(dataFile, dataType) != dataFile.size()) {
		Serial.println("Sent less data than expected!");
	}
	dataFile.close();

	return true;
}
#endif

float analogRead(int pin, int samples) {
	float r = 0;
	for(int i = 0; i < samples; i++)
		r += analogRead(pin);
	r /= samples;
	return r;
}

struct Device {
	int par1;
	int par2;
	int par3;
	int par4;
	int flags;
	char name[16];
	float val;
	unsigned long millis;
} devices[DEVICES_NUM];

IPAddress deviceIP;

bool isAP;
//bool isErrorConn = true;
bool isCheckIn = false;
int errorConnCounter = 0;

#ifdef SD_CS_PIN
bool isSD, errorSD;
#endif

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#ifndef ESP8266
#define OLED_ADDRESS 0x3c
#define OLED_SDA 5
#define OLED_SCL 4
//#define OLED_RST 16
SSD1306 display(OLED_ADDRESS, OLED_SDA, OLED_SCL);

#ifdef MQTT
char mqttServer[20];
char mqttUser[20];
char mqttPassword[20];
unsigned int mqttID;
String mqttClientId;

void receivedCallback(char* topic, byte* payload, unsigned int length) {
	Serial.print(F("MQTT TOPIC:"));
	Serial.print(topic);
	Serial.print(F(" PAYLOAD:"));
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	Serial.println();
	int i = -1;
	if(strstr(topic, A_TOPIC))
		i = 0;
	if(strstr(topic, B_TOPIC))
		i = 1;
	if(strstr(topic, C_TOPIC))
		i = 2;
	if(strstr(topic, D_TOPIC))
		i = 3;
	if(i > -1) {
		if((char)payload[0]=='0') {
			bitClear(devices[i].flags, OUTPUT_BIT);
			bitSet(devices[i].flags, MANUAL_BIT);
		}
		else if((char)payload[0]=='1') {
			bitSet(devices[i].flags, OUTPUT_BIT);
			bitSet(devices[i].flags, MANUAL_BIT);
		}
		//else if((char)payload[0])=='A') {
		//	bitClear(devices[0].flags, RUNONCE_BIT);
		//  bitClear(devices[0].flags, MANUAL_BIT);
		//}
	}
}

void mqttConnect() {
	int i = 0;
	while (!mqttClient.connected()) {
		yield();
		//Serial.print("MQTT connecting ...");
		if(mqttClient.connect(mqttClientId.c_str(), mqttUser, mqttPassword)) {;
			mqttClient.subscribe(String(ROOT_TOPIC + String(mqttID) + "/cmd/#").c_str());
			mqttState = mqttClient.state();
			break;
		}
		else {
			mqttState = mqttClient.state();
			Serial.print(F("MQTT ERROR:"));
			Serial.println(mqttState);
			delay(1000);
		}
		if(i++ >= 2)
			break;
	}
}
#endif

/*
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
	display->setTextAlignment(TEXT_ALIGN_RIGHT);
	display->setFont(ArialMT_Plain_16);
	//display->drawString(128, 0, String(millis()));

#ifdef NTP
	time_t t = CE.toLocal(timeClient.getEpochTime());
	byte h = (t / 3600) % 24;
	byte m = (t / 60) % 60;
	byte s = t % 60;
	char buff[10];
	sprintf(buff, "%02d:%02d:%02d", h, m, s);
	display->drawString(128, 0, buff);
#endif

	display->setTextAlignment(TEXT_ALIGN_LEFT);
	if(isAP)
			display->drawString(0, 0, "AP");
	else {
		if(isErrorConn)
			display->drawString(0, 0, "OFFLINE");
		else
			display->drawString(0, 0, "ONLINE");
	}
}
*/

void drawFrame0(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	//display->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	if(!isAP) {
		display->drawString(0 + x, 16 + y, WiFi.SSID());
	}
	else {
		display->drawString(0 + x, 16 + y, deviceIP.toString());
	}
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->drawString(0 + x, 32 + y, "ESPMONITOR");
}
/*
void drawFrameA1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->drawString(0 + x, 16 + y, "LEVEL ALARMS");
	if(bitRead(devices[3].flags, OUTPUT_BIT))
		display->drawString(0 + x, 32 + y, "MIN");
	else
		display->drawString(0 + x, 32 + y, "    ");
	if(bitRead(devices[4].flags, OUTPUT_BIT))
		display->drawString(64 + x, 32 + y, "MAX");
	else
		display->drawString(64 + x, 32 + y, "   ");
}

void drawFrameA2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	if(!bitRead(devices[DEV_ALARM_MAX].flags, OUTPUT_BIT)) {
		drawNextFrame(display);
		return;
	}
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->drawString(0 + x, 16 + y, "ALARM");
    display->drawString(0 + x, 32 + y, "LEVEL MAX");
}

void drawFrameA3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	if(!bitRead(devices[DEV_ALARM_MIN].flags, OUTPUT_BIT)) {
		drawNextFrame(display);
		return;
	}
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->drawString(0 + x, 16 + y, "ALARM");
    display->drawString(0 + x, 32 + y, "LEVEL MIN");
}

void drawFrameA4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
#ifdef SD_CS_PIN
	if(!errorSD) {
		drawNextFrame(display);
		return;
	}
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->drawString(0 + x, 16 + y, "ALARM");
    display->drawString(0 + x, 32 + y, "SD CARD ERR");
#else
    drawNextFrame(display);
#endif
}
void drawFrameA5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	if(!isErrorConn) {
		drawNextFrame(display);
		return;
	}
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->drawString(0 + x, 16 + y, "ALARM");
    display->drawString(0 + x, 32 + y, "INTERNET ERR");
}
*/

String deviceToString(struct Device device){
	//return String(bitRead(device.flags, MANUAL_BIT) ? "MAN " : "AUTO ")  + String(bitRead(device.flags, OUTPUT_BIT) ? "ON" : "OFF");
	return String(bitRead(device.flags, OUTPUT_BIT) ? "ALARM" : "OK");
}

/*
void drawFrameD1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->drawString(0 + x, 16 + y,"A: " + String(devices[0].name));
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 32 + y, deviceToString(devices[0]));
}

void drawFrameD2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->drawString(0 + x, 16 + y, "B: " + String(devices[1].name));
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 32 + y, deviceToString(devices[1]));
}

void drawFrameD3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->drawString(0 + x, 16 + y, "C: " + String(devices[2].name));
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 32 + y, deviceToString(devices[2]));
}

void drawFrameD4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->drawString(0 + x, 16 + y, "D: " + String(devices[3].name));
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 32 + y, deviceToString(devices[3]));
}

void drawFrameM1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	display->drawString(0 + x, 16 + y, "TEMPERATURE");
	display->setFont(ArialMT_Plain_16);
	//display->drawString(0 + x, 32 + y, String(temperature));
}
*/

int devNo;
int frameNo = 0;

void drawFrameD(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	//display->drawString(0 + x, 16 + y,"A: " + String(devices[devNo].name));
	display->drawString(0 + x, 16 + y, String(char(devNo + 65)) + ":" + String(devices[devNo].name));
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 32 + y, deviceToString(devices[devNo]));
    display->drawString(64 + x, 32 + y, String(bitRead(devices[devNo].flags, MANUAL_BIT)));
}

void drawFrameM(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	//display->drawString(0 + x, 16 + y,"A: " + String(devices[devNo].name));
	display->drawString(0 + x, 16 + y, String(char(devNo + 65)) + ":" + String(devices[devNo].name));
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 32 + y, deviceToString(devices[devNo]));
    display->drawString(64 + x, 32 + y, String(devices[devNo].val));
}

//FrameCallback frames[] = { drawFrame1, drawFrameA2, drawFrameA3, drawFrameA4, drawFrameA5, drawFrameD1, drawFrameD2, drawFrameD3, drawFrameD4, drawFrameM1};
//int frameCount = 10;
FrameCallback frames[] = { drawFrame0,
		drawFrameD, drawFrameD, drawFrameD, drawFrameD,
		drawFrameD, drawFrameD, drawFrameD, drawFrameD,
		drawFrameD, drawFrameD, drawFrameD, drawFrameD,
		drawFrameD, drawFrameD, drawFrameD, drawFrameD,
		drawFrameD, drawFrameD, drawFrameD, drawFrameM,
		drawFrameM, drawFrameM, drawFrameM, drawFrameM};
int frameCount = 25;

//OverlayCallback overlays[] = { msOverlay };
//int overlaysCount = 1;


int lastFrameNo = 0;
String lastMessage;
void drawDisplay(OLEDDisplay *display) {
	drawDisplay(display, lastFrameNo);
}

void drawDisplay(OLEDDisplay *display, int frame) {
	devNo = frame - 1;
	devNo = min(devNo, DEVICES_NUM);
	lastFrameNo = frame;

	display->clear();

	if(millis() - screenSaverMillis > SCREENSAVER * 1000) {
		display->display();
		return;
	}

	if(isAP)
		display->drawString(0, 0, "AP");
	else
		display->drawString(0, 0, WiFi.localIP().toString());

	(frames[frame])(display, 0, 0, 0);

	//for (uint8_t i = 0; i < overlaysCount; i++){
	//    (overlays[i])(display, 0 );
	// }

	//for(int i = 0; i < 4; i++) {
	//	if(bitRead(devices[i].flags, OUTPUT_BIT)) {
	//		char ch = 65 + i;
	//		display->drawString(12 * i, 48, String(ch));
	//	}
	//}

	//if(isErrorConn) {
	//	display->setTextAlignment(TEXT_ALIGN_RIGHT);
	//	display->drawString(128, 48, "ALM");
	//	display->setTextAlignment(TEXT_ALIGN_LEFT);
	//}

#ifdef SD_CS_PIN
	if(errorSD) {
		display->setTextAlignment(TEXT_ALIGN_RIGHT);
		display->drawString(128, 48, "ALM");
		display->setTextAlignment(TEXT_ALIGN_LEFT);
	}
#endif

	if(lastMessage != "") {
		display->drawString(0, 48, lastMessage);
		lastMessage = "";
	}
	else {
		//if(isErrorConn)
		if(errorConnCounter)
			display->drawString(0, 48, "CONN ERROR");
	}

	display->display();
}

void drawMessage(OLEDDisplay *display, String msg) {
	lastMessage = msg;
	Serial.println(msg);

	drawDisplay(display);
	//display->setFont(ArialMT_Plain_16);
	//display->setTextAlignment(TEXT_ALIGN_LEFT);
	//display->drawString(0, 48, msg);
	//display->display();
}
#endif

const char* host = "espmonitor-esp";
const char* updatePath = "/update";

char* htmlHeader = "<html><head><title>ESPMONITOR</title><meta name=\"viewport\" content=\"width=device-width\"><style type=\"text/css\">button {height:100px;width:100px;font-family:monospace;border-radius:5px;}</style></head><body><h1><a href=/>ESPMONITOR</a></h1>";
char* htmlFooter = "<hr><a href=./save>SAVE INSTRUMENTS!</a><hr><a href=/settings>SYSTEM SETTINGS</a><hr><a href=http://growmat.cz>http://growmat.cz</a></body></html>";
//const char HTTP_STYLE[] PROGMEM  = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

char wwwUsername[16];// = "admin";
char wwwPassword[16];

//#define THINGSSPEAK
#ifdef THINGSSPEAK
char serverName[20];
char writeApiKey[20];
unsigned int talkbackID;
char talkbackApiKey[20];
#endif

HTTPClient http;  //Declare an object of class HTTPClient
int httpCode;
unsigned int httpErrorCounter;

//#define LCD
#ifdef LCD
#include <SPI.h>
#include "libraries/Adafruit_GFX.h"
#include "libraries/Adafruit_PCD8544.h"
const int8_t RST_PIN = 4;
const int8_t CE_PIN = 5;
const int8_t DC_PIN = 12;
//const int8_t DIN_PIN = D7;  // Uncomment for Software SPI
//const int8_t CLK_PIN = D5;  // Uncomment for Software SPI
const int8_t BL_PIN = 16; //D0;
Adafruit_PCD8544 display = Adafruit_PCD8544(DC_PIN, CE_PIN, RST_PIN);
#endif

//#define RFRX_PIN D3
#ifdef RXTX_PIN
#include "libraries/RCSwitch.h"
RCSwitch rcSwitch = RCSwitch();
unsigned int r1Off = 2664494;
unsigned int r1On = 2664495;
unsigned int r2Off = 2664492;
unsigned int r2On = 2664493;
unsigned int r3Off = 2664490;
unsigned int r3On = 2664491;
unsigned int r4Off = 2664486;
unsigned int r4On = 2664487;
unsigned int rAllOff = 2664481;
unsigned int rAllOn = 2664482;
#endif

#define EEPROM_FILANEME_ADDR 124
#define EEPROM_OFFSET 160

String getDeviceForm(int i, struct Device devices[]) {
	Device d = devices[i];

	String s = "<form action=/dev><input type=hidden name=id value=";
	s += i;
	s += "><h2>";
	s += char( i+ 65);
	//s += String(i);
	s += ":";
	s += String(d.name);

		//if(bitRead(devices[i].flags, MANUAL_BIT))
		//	s += " MANUAL";
		//else
		//	s += " AUTO";
	if(bitRead(devices[i].flags, OUTPUT_BIT))
		s += " ALARM ";
	else
		s += " OK ";

	if(i < DEVICES_DIGITAL_NUM) {
		s += String(bitRead(devices[i].flags, MANUAL_BIT));
	}
	else {
		s += String(devices[i].val);
		s += " C";
	}
	s += "</h2>";

	s += "<hr><h3>SETTINGS:</h3>";
	s += "<hr>NAME<br>";
	s += "<input name=name value=\"";
	s += d.name;
	s += "\">";

	if(i < DEVICES_DIGITAL_NUM) {
		s += "<hr>DELAY [SEC]<br><input name=par1 value=";
		s += d.par1;
		//s += "><hr>DELAY OFF [SEC]<br><input name=par2 value=";
		//s += d.par2;
		s += "><hr>INVERSION [-]<br><input name=par3 value=";
		s += d.par3;
		s += ">";
	}
	else {
		s += "<hr>TEMP MAX [C]<br><input name=par1 value=";
		s += d.par1;
		s += "><hr><br>TEMP MIN [C]<br><input name=par2 value=";
		s += d.par2;
		//s += "><hr><br>DELAY [SEC]<br><input name=par3 value=";
		//s += d.par3;
		s += ">";
	}

	s += "<hr><button type=submit name=cmd value=set>SET</button>";
	s += "</form>";
	return s;
}

void handleRoot(){
   if(!httpServer.authenticate(wwwUsername, wwwPassword))
      return httpServer.requestAuthentication();

  String message;
  httpServer.send(200, "text/plain", message);
}

void saveApi() {
	int offset = 8;
	EEPROM.put(offset, wwwUsername);
	offset += sizeof(wwwUsername);
	EEPROM.put(offset, wwwPassword);
	offset += sizeof(wwwPassword);

	EEPROM.put(offset, botToken);
	offset += sizeof(botToken);
	EEPROM.put(offset, botChatId);
	offset += sizeof(botChatId);

	EEPROM.put(offset, commInterval);
	offset += sizeof(commInterval);
	EEPROM.put(offset, reconnNum);
	offset += sizeof(reconnNum);

#ifdef THINGSSPEAK
	EEPROM.put(offset, serverName);
	offset += sizeof(serverName);
	EEPROM.put(offset, writeApiKey);
	offset += sizeof(writeApiKey);
	EEPROM.put(offset, talkbackApiKey);
	offset += sizeof(talkbackApiKey);
	EEPROM.put(offset, talkbackID);
	offset += sizeof(talkbackID);
#endif

#ifdef MQTT
	EEPROM.put(offset, mqttServer);
	offset += sizeof(mqttServer);
	EEPROM.put(offset, mqttUser);
	offset += sizeof(mqttUser);
	EEPROM.put(offset, mqttPassword);
	offset += sizeof(mqttPassword);
	EEPROM.put(offset, mqttID);
	offset += sizeof(mqttID);
	mqttRootTopic = String(ROOT_TOPIC + String(mqttID) + "/val");
	mqttClient.setServer(mqttServer, 1883);
#endif

	EEPROM.put(0, 0);
	EEPROM.commit();
}

void saveInstruments() {
	for(int d=0; d< DEVICES_NUM; d++) {
		EEPROM.put(EEPROM_OFFSET + sizeof(Device) * d, devices[d]);
	}
	EEPROM.put(0, 0);
	EEPROM.commit();
}

void startWiFiAP() {
	  isAP = true;
	  //WiFi.softAP("espmonitor", "espmonitor");
	  WiFi.softAP("", "");
	  Serial.println(F("START AP..."));
	  deviceIP = WiFi.softAPIP();
	  Serial.print(F("AP IP:"));
	  Serial.println(deviceIP);
}

bool connectWiFi() {
	if(WiFi.status() == WL_CONNECTED) {
		isAP = false;
		return true;
	}

	DRAWMESSAGE(display, "WIFI CONN...");

//#ifndef ESP8266
//		drawDisplay(&display, 0);
//#endif

	WiFiManager wifiManager;
	//DRAWMESSAGE(display, wifiManager.getSSID());

	//if (wifiManager.autoConnect()) {
	if (wifiManager.connectWifi("", "") == WL_CONNECTED) {
		isAP = false;
		Serial.print(F("IP:"));
		Serial.println(WiFi.localIP());
		DRAWMESSAGE(display, "WIFI DONE");
		return true;
	}

	DRAWMESSAGE(display, "WIFI FAIL");

#ifdef LED0_PIN
	for(int i = 0; i < 4; i++) {
		digitalWrite(LED0_PIN, true);
		delay(250);
		digitalWrite(LED0_PIN, false);
		delay(250);
	}
#else
	delay(4 * 250 * 125);
#endif

	return false;
}

/////////////////////////////////
// setup
/////////////////////////////////
void setup() {
	//TODO:
	//WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

	Serial.begin(115200);
	Serial.print(F("\n\n"));
	Serial.println(F("ESPMONITOR"));

#ifndef ESP8266
	//analogReadResolution(12);
	display.init();
	display.setFont(ArialMT_Plain_16);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(0, 0, "ESPMONITOR");
	display.display();
#endif

#ifdef SD_CS_PIN
	isSD = true;
	if(!SD.begin(SD_CS_PIN)) {
		isSD = false;
		Serial.println(F("SD ERROR"));
	}
	uint8_t cardType = SD.cardType();
	if(cardType == CARD_NONE){
		isSD = false;
		Serial.println(F("NO SD CARD"));
	}
	errorSD = !isSD;
#endif

#ifdef LCD
	display.begin();
	display.setContrast(60);  // Adjust for your display
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(BLACK);
	display.setCursor(0,0);
	display.println("espmonitor");
	display.display();
#endif

#ifdef ONEWIREBUS_PIN
	oneWireSensors.begin();
#endif

#ifdef DHT_PIN
	dht.begin();
#endif

#ifdef RFRX_PIN
	rcSwitch.enableReceive(RFRX_PIN);
#endif

//#define RFTX_PIN 22
#ifdef RFTX_PIN
	rcSwitch.enableTransmit(RFTX_PIN);
#endif

	EEPROM.begin(2048);
	if(!EEPROM.read(0)) {

		int offset = 8;
		EEPROM.get(offset, wwwUsername);
		offset += sizeof(wwwUsername);
		EEPROM.get(offset, wwwPassword);
		offset += sizeof(wwwPassword);

		EEPROM.get(offset, botToken);
		offset += sizeof(botToken);
		EEPROM.get(offset, botChatId);
		offset += sizeof(botChatId);

		EEPROM.get(offset, commInterval);
		offset += sizeof(commInterval);
		EEPROM.get(offset, reconnNum);
		offset += sizeof(reconnNum);


#ifdef THINGSSPEAK
		EEPROM.get(offset, serverName);
		offset += sizeof(serverName);
		EEPROM.get(offset, writeApiKey);
		offset += sizeof(writeApiKey);
		EEPROM.get(offset, talkbackApiKey);
		offset += sizeof(talkbackApiKey);
		EEPROM.get(offset, talkbackID);
		offset += sizeof(talkbackID);
#endif

#ifdef MQTT
		EEPROM.get(offset, mqttServer);
		offset += sizeof(mqttServer);
		EEPROM.get(offset, mqttUser);
		offset += sizeof(mqttUser);
		EEPROM.get(offset, mqttPassword);
		offset += sizeof(mqttPassword);
		EEPROM.get(offset, mqttID);
		offset += sizeof(mqttID);
		mqttClient.setServer(mqttServer, 1883);
#endif

		for(int d=0; d< DEVICES_NUM; d++) {
			EEPROM.get(EEPROM_OFFSET + sizeof(Device) * d, devices[d]);
			bitClear(devices[d].flags, OUTPUT_BIT);
		}
	}
	else {
		Serial.println(F("EEPROM RESET"));

		strcpy(wwwUsername, "admin") ;
		strcpy(wwwPassword, "") ;
		botToken[0] = '/0';
		botChatId[0] = '/0';

		commInterval = 20;
		reconnNum = 15;

#ifdef THINGSSPEAK
		strcpy(serverName, "api.thingspeak.com") ;
		writeApiKey[0] = '/0';
		talkbackID = 0;
		talkbackApiKey[0] = '/0';
#endif

#ifdef MQTT
		strcpy(mqttServer, "broker.hivemq.com") ;
		mqttUser[0] = '/0';
		mqttPassword[0] = '/0';
		//strcpy(mqttUser, "espmonitor") ;
		//strcpy(mqttPassword, "espmonitor") ;
		mqttID = 0;
#endif

		for(int i = 0; i < DEVICES_NUM; i++) {
			String name;
			if(i < DEVICES_DIGITAL_NUM) {
				name += "INPUT_";
			}
			else {
				name += "TEMP_";
			}
			name += char(i + 65);
			name.toCharArray(devices[i].name, 16);
			devices[i].name[sizeof(devices[i].name) - 1] = '\0';
		}

		saveApi();
		saveInstruments();
	}

#ifdef MQTT
	mqttRootTopic = String(ROOT_TOPIC + String(mqttID) + "/val");
#endif

#ifdef CONFIG_WIFIAP_PIN
	pinMode(CONFIG_WIFIAP_PIN, INPUT_PULLUP);
#endif

	pinMode(CONFIG_WIFI_PIN, INPUT_PULLUP);
	for(int i = 0; i < DEVICES_DIGITAL_NUM; i++) {
		pinMode(inputPins[i], INPUT_PULLUP);
	}

	for(int i = 0; i < DEVICES_NUM; i++) {
		bitClear(devices[i].flags, UNACK_BIT);
	}
	for(int i = 0; i < DEVICES_DIGITAL_NUM; i++) {
		bool pinValue = digitalRead(inputPins[i]);
		if(devices[i].par3) {
			pinValue = !pinValue;
		}
		//if(pinValue)
		//	bitSet(devices[i].flags, OUTPUT_BIT);
		//else
		//	bitClear(devices[i].flags, OUTPUT_BIT);
	}

#ifdef RFTX_PIN
	pinMode(RFTX_PIN, OUTPUT);
#endif

#ifdef OUTPUT0_PIN
	pinMode(OUTPUT0_PIN, OUTPUT);
	pinMode(OUTPUT1_PIN, OUTPUT);
	pinMode(OUTPUT2_PIN, OUTPUT);
	pinMode(OUTPUT3_PIN, OUTPUT);
	digitalWrite(OUTPUT0_PIN, HIGH);
	digitalWrite(OUTPUT1_PIN, HIGH);
	digitalWrite(OUTPUT2_PIN, HIGH);
	digitalWrite(OUTPUT3_PIN, HIGH);
#endif

#ifdef LED0_PIN
	pinMode(LED0_PIN, OUTPUT);
	digitalWrite(LED0_PIN, LOW);
#endif

#ifdef LED1_PIN
	pinMode(LED1_PIN, OUTPUT);
	digitalWrite(LED1_PIN, LOW);
#endif

#ifdef MQTT
	mqttClientId = String(MQTT_CLIENTID) + WiFi.macAddress();
	Serial.println(mqttClientId);
#endif

	//if (drd.detectDoubleReset()) {
	if (!digitalRead(CONFIG_WIFI_PIN)) {
		Serial.println(wwwUsername);
		Serial.println(wwwPassword);

#ifdef LED0_PIN
		for(int i = 0; i < 8; i++) {
			digitalWrite(LED0_PIN, true);
			delay(125);
			digitalWrite(LED0_PIN, false);
			delay(125);
		}
		digitalWrite(LED0_PIN, false);
#endif

		Serial.println(F("WIFI CONFIG!"));
		display.drawString(0, 16, "WIFI CONFIG!");
		display.drawString(0, 32, "192.168.4.1");
		display.drawString(0, 48, WiFi.SSID());
		display.display();

		WiFiManager wifiManager;

		//wifiManager.resetSettings();
		//wifiManager.startConfigPortal("ESPNODE");
		//Serial.println(wifiManager.getSSID());
		//Serial.println(wifiManager.getPassword());
		//Serial.println(WiFi.localIP());

		wifiManager.startConfigPortal();
	}
	else {
		isAP = true;

		//while(!connectWiFi());
		for (int i = 0; i < 3; i++) {
			 if(connectWiFi())
				 break;
		}
		if (isAP) {
			startWiFiAP();
		}
	}
/*
	WiFiManager wifiManager;
	//wifiManager.resetSettings();
	//wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

	if ( digitalRead(INPUT1_PIN) == LOW ) {
		strcpy(wwwPassword, "espmonitor") ;
	}

	if ( digitalRead(CONFIG_WIFI_PIN) == LOW ) {

#ifdef LED0_PIN
		digitalWrite(LED0_PIN, HIGH);
#endif

	  Serial.println(F("START WIFI CONFIG!"));
	  DRAWMESSAGE(display, "WIFI CONFIG!");

#ifndef ESP8266
	  drawDisplay(&display,  0);
#endif

	  wifiManager.resetSettings();
	  wifiManager.startConfigPortal("espmonitor");
	}
	else {
		isAP = true;
		Serial.println(F("START AP OR STA"));
		for(int i = 0; i < 3; i++) {
			DRAWMESSAGE(display, "WIFI CONN:" + String(i));
			if(wifiManager.autoConnect()) {
				isAP = false;
				Serial.print(F("IP:"));
				Serial.println(WiFi.localIP());
				DRAWMESSAGE(display, "WIFI CONN");
				break;
			}
			Serial.println(wifiManager.getSSID());
			Serial.println(wifiManager.getPassword());
		}
		if(isAP) {
			startWiFiAP();
		}
#ifdef LED1_PIN
		digitalWrite(LED1_PIN, HIGH);
#endif
	}
*/

	WiFi.printDiag(Serial);
	DRAWMESSAGE(display, "WIFI READY");

	Serial.print(F("USERNAME:"));
	Serial.println(wwwUsername);
	Serial.print(F("PASSWORD:"));
	Serial.println(wwwPassword);
	Serial.print(F("BOT:"));
	Serial.println(botToken);
	Serial.print(F("CHATID:"));
	Serial.println(botChatId);

	//client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
	//client.setInsecure();
	//bot._debug = true;
	//bot.setApi(botToken, client);
	bot.updateToken(botToken);


#ifdef MQTT
	mqttClient.setCallback(receivedCallback);
	mqttLock.clear();
#endif
	//timeClient.setUpdateInterval(60000 * 10);

	httpServer.on("/", [](){
		if(!httpServer.authenticate(wwwUsername, wwwPassword))
			return httpServer.requestAuthentication();

		String message = htmlHeader;

#ifdef NTP
		time_t t = CE.toLocal(timeClient.getEpochTime());
		tmElements_t tm;
		breakTime(t, tm);

		message += "<h2>TIME: ";
		message +=String(int2string(tm.Hour) + ':' + int2string(tm.Minute) + ':' + int2string(tm.Second) + ' ' + int2string(tm.Year + 1970) + '-' + int2string(tm.Month) + '-' + int2string(tm.Day));
		message += "</h2>";
		/*byte h = (t / 3600) % 24;
		byte m = (t / 60) % 60;
		byte s = t % 60;

		message += "<h2>TIME: ";
		if(h < 10)
			message += '0';
		message += h;
		message += ":";
		if(m < 10)
			message += '0';
		message += m;
		message += ":";
		if(s < 10)
        	message += '0';
		message += s;
		message += "</h2>";
		*/
#endif

		//if(bitRead(devices[DEV_ALARM_MAX].flags, OUTPUT_BIT))
		//	message += "<h2>ALARM: LEVEL MAX</h2>";
		//if(bitRead(devices[DEV_ALARM_MIN].flags, OUTPUT_BIT))
		//	message += "<h2>ALARM: LEVEL MIN</h2>";
		//if(isErrorConn)
		if(errorConnCounter)
			message += "<h2>ALARM:INTERNET CONNECTION ERROR</h2>";

#ifdef SD_CS_PIN
		if(errorSD)
			message += "<h2>ALARM:SD CARD</h2>";
#endif

		for(int i = 0; i < DEVICES_NUM; i++) {
			if(i == 0) {
				message += "<hr><h3>INPUTS</h3>";
			}
			if(i == DEVICES_DIGITAL_NUM) {
				message += "<hr><h3>TEMPERATURES</h3>";
			}

			message += "<hr><a href=./dev?id=";
			message += i;
			message += ">";

			message += char(i + 65); //String(i);
			message += ":";

			message += devices[i].name;
			if(i < DEVICES_DIGITAL_NUM) {
				//if(bitRead(devices[i].flags, MANUAL_BIT))
				//	message += " MANUAL";
				//else
				//	message += " AUTO";
				if(bitRead(devices[i].flags, OUTPUT_BIT))
					message += " ALARM ";
				else
					message += " OK ";
				message += String(bitRead(devices[i].flags, MANUAL_BIT));
			}
			else {
				if(bitRead(devices[i].flags, OUTPUT_BIT))
					message += " ALARM ";
				else
					message += " OK ";
				message += String(devices[i].val);
				message += " C";
			}

			message += "</a>";
		}
		message += "<hr><h3>SYSTEM</h3>";

#ifdef SD_CS_PIN
		message += "<hr><a href=/logs>LOGS</a>";
#endif

		message += htmlFooter;
		httpServer.send(200, "text/html", message);
	});

#ifdef SD_CS_PIN
	httpServer.on("/log", [](){
	  Serial.println(httpServer.args());
		if(!httpServer.authenticate(wwwUsername, wwwPassword))
		  return httpServer.requestAuthentication();
	  if(httpServer.args() > 0)
		  loadFromSdCard(httpServer.arg(0));
	});

	httpServer.on("/logs", [](){
		if(!httpServer.authenticate(wwwUsername, wwwPassword))
		  return httpServer.requestAuthentication();
		String message = htmlHeader;
		File root = SD.open("/");
		if(!root){
		  Serial.println("Failed to open directory");
		  return;
		}
		if(!root.isDirectory()){
		  Serial.println("Not a directory");
		  return;
		}

		message += "<table>";
		File file = root.openNextFile();
		while(file){
		  if(file.isDirectory()){
			  Serial.print("DIR: ");
			  Serial.println(file.name());
			  //if(levels){
			  //  listDir(fs, file.name(), levels -1);
		  }
		  else {
			  Serial.print("FILE: ");
			  Serial.print(file.name());
			  Serial.print("SIZE: ");
			  Serial.println(file.size());
			  message += "<tr><td><a href=/log?name=";
			  message += file.name();
			  message += ">";
			  message += file.name();
			  message += "</a></td><td>";
			  message += String(file.size());
			  message += "</td></tr>";
		  }
		  file = root.openNextFile();
		}
		message += "</table>";
		Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
		Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
		message += htmlFooter;
		httpServer.send(200, "text/html", message);
	});
#endif

	httpServer.on("/save", [](){
		if(!httpServer.authenticate(wwwUsername, wwwPassword))
			return httpServer.requestAuthentication();
		saveInstruments();

		String message = htmlHeader;
		message += "OK";
		message += htmlFooter;
		httpServer.send(200, "text/html", message);
	});

	httpServer.on("/settings", [](){
		if(!httpServer.authenticate(wwwUsername, wwwPassword))
		  return httpServer.requestAuthentication();

		String message = htmlHeader;

#ifdef NTP
		time_t t = CE.toLocal(timeClient.getEpochTime());
		tmElements_t tm;
		breakTime(t, tm);

		message += "<hr>";
		message += "<form action=/savesettings>";
		message += "HOURS<br><input name=hour value=";
		message += tm.Hour;
		message += "><br>";
		message += "<br>MINUTES<br><input name=minute value=";
		message += tm.Minute;
		message += "><br>";
		message += "<br>SECONDS<br><input name=second value=";
		message += tm.Second;
		message += "><br>";
		message += "<br>YEAR<br><input name=year value=";
		message += tm.Year + 1970;
		message += "><br>";
		message += "<br>MONTH<br><input name=month value=";
		message += tm.Month;
		message += "><br>";
		message += "<br>DAY<br><input name=day value=";
		message += tm.Day;
		message += "><br><br>";
		message += "<button type=submit name=cmd value=settime>SET TIME</button>";
		message += "</form>";
#endif

		message += "<hr>";
		message += "<form action=/savesettings>";
		message += "USERNAME<br><input name=wwwusername value=";
		message += wwwUsername;
		message += "><br><br>";
		message += "PASSWORD<br><input name=wwwpassword value=";
		message += wwwPassword;
		message += "><br>";
		message += "<hr>";
		message += "TELEGRAM TOKEN<br><input name=bottoken value=";
		message += botToken;
		message += "><br><br>";
		message += "LAST MESSAGE CHATID:";
		message += lastBotChatId;
		message += "<br>";
		message += "CHAT ID<br><input name=botchatid value=";
		message += botChatId;
		message += "><hr>";
		message += "COMM INTERVAL [s]<br><input name=comminterval value=";
		message += commInterval;
		message += "><br><br>";
		message += "RECONNECT [n * COMM INTERVAL]<br><input name=reconnnum value=";
		message += reconnNum;
		message += "><br>";

#ifdef THINGSSPEAK
		message += "<br>SERVER NAME<br><input name=servername value=";
		message += serverName;
		message += "><br>";
		message += "<br>WRITE API KEY<br><input name=writeapikey value=";
		message += writeApiKey;
		message += "><br>";
		message += "<br>TALKBACK ID<br><input name=talkbackid value=";
		message += talkbackID;
		message += "><br>";
		message += "<br>TALKBACK API KEY<br><input name=talkbackapikey value=";
		message += talkbackApiKey;
		message += "><br>";
#endif

#ifdef MQTT
		message += "<br>MQTT SERVER<br><input name=mqttserver value=";
		message += mqttServer;
		message += "><br>";
		message += "<br>MQTT USER<br><input name=mqttuser value=";
		message += mqttUser;
		message += "><br>";
		message += "<br>MQTT PASSWORD<br><input name=mqttpassword value=";
		message += mqttPassword;
		message += "><br>";
		message += "<br>MQTT ID<br><input name=mqttid value=";
		message += mqttID;
		message += "><br>"
#endif

		message += "<hr><button type=submit name=cmd value=setapi>SET API!</button>";
		message += "</form>";

		message += htmlFooter;
		httpServer.send(200, "text/html", message);
	});

	httpServer.on("/savesettings", [](){
		if(!httpServer.authenticate(wwwUsername, wwwPassword))
			return httpServer.requestAuthentication();

		String message = htmlHeader;

#ifdef NTP
		if(httpServer.arg("cmd").equals("settime")) {
			int offset = CE.toUTC(0) - CE.toLocal(0);
			tmElements_t tm;
			tm.Second = (unsigned long)httpServer.arg("second").toInt();
			tm.Minute = (unsigned long)httpServer.arg("minute").toInt();
			tm.Hour = (unsigned long)httpServer.arg("hour").toInt() + offset / 3600;;
			tm.Day = (unsigned long)httpServer.arg("day").toInt();
			tm.Month = (unsigned long)httpServer.arg("month").toInt();
			tm.Year = (unsigned long)httpServer.arg("year").toInt() - 1970;
			time_t t = makeTime(tm);
			timeClient.setEpochTime(t);

			message += "TIME SET";
		}
#endif

		if(httpServer.arg("cmd").equals("setapi")) {
			strncpy(wwwUsername, httpServer.arg("wwwusername").c_str(), sizeof(wwwUsername));
			strncpy(wwwPassword, httpServer.arg("wwwpassword").c_str(), sizeof(wwwPassword));
			wwwUsername[sizeof(wwwUsername) - 1] = '/0';
			wwwPassword[sizeof(wwwPassword) - 1] = '/0';

			strncpy(botToken, httpServer.arg("bottoken").c_str(), sizeof(botToken));
			strncpy(botChatId, httpServer.arg("botchatid").c_str(), sizeof(botChatId));
			botToken[sizeof(botToken) - 1] = '/0';
			botChatId[sizeof(botChatId) - 1] = 0;

			commInterval = httpServer.arg("comminterval").toInt();
			reconnNum = httpServer.arg("reconnnum").toInt();

#ifdef THINGSSPEAKS
			strcpy(serverName, httpServer.arg("servername").c_str());
			strcpy(writeApiKey, httpServer.arg("writeapikey").c_str());
			talkbackID = httpServer.arg("talkbackid").toInt();
			strcpy(talkbackApiKey, (char*)httpServer.arg("talkbackapikey").c_str());
#endif

#ifdef MQTT
			strcpy(mqttServer, (char*)httpServer.arg("mqttserver").c_str());
			strcpy(mqttUser, (char*)httpServer.arg("mqttuser").c_str());
			strcpy(mqttPassword, (char*)httpServer.arg("mqttpassword").c_str());
			mqttID = httpServer.arg("mqttid").toInt();
#endif

			message += "API SET";
			saveApi();
		}
		message += htmlFooter;
		httpServer.send(200, "text/html", message);
	});

	httpServer.on("/dev", [](){
		if(!httpServer.authenticate(wwwUsername, wwwPassword))
		  return httpServer.requestAuthentication();

		String cmd = httpServer.arg("cmd");
		int id = httpServer.arg("id").toInt();
		if(cmd.equals("set")) {
			int par1 = httpServer.arg("par1").toInt();
			int par2 = httpServer.arg("par2").toInt();
			int par3 = httpServer.arg("par3").toInt();
			int par4 = httpServer.arg("par4").toInt();
			String name = httpServer.arg("name");
			if(id >=0 && id < DEVICES_NUM) {
				devices[id].par1 = par1;
				devices[id].par2 = par2;
				devices[id].par3 = par3;
				devices[id].par4 = par4;
				strncpy(devices[id].name, name.c_str(), 16);
			}
		}
		if(cmd.equals("auto")) {
			bitClear(devices[id].flags, MANUAL_BIT);
			bitClear(devices[id].flags, RUNONCE_BIT);
		}
		if(cmd.equals("off")) {
			bitSet(devices[id].flags, MANUAL_BIT);
			bitClear(devices[id].flags, CMD_BIT);
			bitClear(devices[id].flags, OUTPUT_BIT);
		}
		if(cmd.equals("on")) {
			bitSet(devices[id].flags, MANUAL_BIT);
			bitSet(devices[id].flags, CMD_BIT);
			bitSet(devices[id].flags, OUTPUT_BIT);
		}
		String message = htmlHeader;
		message += getDeviceForm(id, devices);
		message += htmlFooter;
		httpServer.send(200, "text/html", message);
	});

#ifndef ESP8266
	//const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
	httpServer.on("/update", HTTP_GET, [](){
        if(!httpServer.authenticate(wwwUsername, wwwPassword))
        	return httpServer.requestAuthentication();
        httpServer.sendHeader("Connection", "close");
        String message = htmlHeader;
		message += serverIndex;
		message += htmlFooter;
		httpServer.send(200, "text/html", message);
	});

	httpServer.on("/update", HTTP_POST, [](){
        if(!httpServer.authenticate(wwwUsername, wwwPassword))
            return httpServer.requestAuthentication();
        httpServer.sendHeader("Connection", "close");\
        httpServer.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
        ESP.restart();
      },[](){
    	  HTTPUpload& upload = httpServer.upload();
    	  if(upload.status == UPLOAD_FILE_START){
    		  Serial.setDebugOutput(true);
    		  Serial.printf("UPDATE:%s\n", upload.filename.c_str());

#ifdef ESP8266
    		  uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
#else
    		  uint32_t maxSketchSpace = 0x140000;//(ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
#endif

    		  if(!Update.begin(maxSketchSpace)){
    			  Update.printError(Serial);
    		  }
    	  }
    	  else if(upload.status == UPLOAD_FILE_WRITE){
    		  if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
    			  Update.printError(Serial);
    		  }
    	  } else if(upload.status == UPLOAD_FILE_END){
    		  if(Update.end(true)){
    			  Serial.printf("UPDATE SUCCESS:%u\nRESET...\n", upload.totalSize);
    		  }
    		  else {
    			  Update.printError(Serial);
    		  }
    		  Serial.setDebugOutput(false);
    	  }
    	  yield();
      });
#endif

	httpServer.begin();
	MDNS.begin(host);

#ifdef ESP8266
	httpUpdater.setup(&httpServer, updatePath, wwwUsername, wwwPassword);
#endif
	MDNS.addService("http", "tcp", 80);

	//ArduinoOTA.setPort(8266);
	ArduinoOTA.setHostname(host);
	//ArduinoOTA.setPassword((const char *)"xxxxx");
	ArduinoOTA.onStart([]() {
		Serial.println(F("OTA START"));
	});
	ArduinoOTA.onEnd([]() {
		Serial.println(F("OTA DONE"));
		Serial.println(F("RESET..."));
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("PROGRESS:%u%%\r\n", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("OTA ERROR[%u]:", error);
		if (error == OTA_AUTH_ERROR)
			Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR)
			Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR)
			Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR)
			Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR)
			Serial.println("End Failed");
	});
	ArduinoOTA.begin();

	//if (digitalRead(CONFIG_WIFI_PIN) == LOW ) {
	//	//emergency restore OTA
	//	while(true) {
	//		ArduinoOTA.handle();
	//		httpServer.handleClient();
	//	}
	//}

#ifdef ESP32_THREAD
	xTaskCreatePinnedToCore(loopComm, "loopComm", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
#endif
}

void drawNextFrame(OLEDDisplay *display) {
	frameNo++;
	drawDisplay(display, frameNo);
}

////////////////////////
// communication loop
////////////////////////
void loopComm(void *pvParameters) {

#ifdef MQTT
		if(!mqttLock.test_and_set() && mqttServer[0] != 0 ) {
			DRAWMESSAGE(display, "MQTT CONN");
			/* this function will listen for incoming subscribed topic-process-invoke receivedCallback */
			mqttClient.loop();
			if (!mqttClient.connected()) {
				mqttConnect();
			}
			if (mqttClient.connected()) {
				 snprintf (msg, 20, "%d", (int)level);
				 msg[19] = 0;
				 mqttClient.publish(String(mqttRootTopic + LEVEL_VAL_TOPIC).c_str(), msg);
				 snprintf (msg, 20, "%d", bitRead(devices[DEV_ALARM_MAX].flags, OUTPUT_BIT));
				 mqttClient.publish(String(mqttRootTopic + LEVEL_MAX_TOPIC).c_str(), msg);
				 snprintf (msg, 20, "%d", bitRead(devices[DEV_ALARM_MIN].flags, OUTPUT_BIT));
				 mqttClient.publish(String(mqttRootTopic + LEVEL_MIN_TOPIC).c_str(), msg);
				 snprintf (msg, 20, "%d", bitRead(devices[0].flags, OUTPUT_BIT));
				 mqttClient.publish(String(mqttRootTopic + A_TOPIC).c_str(), msg);
				 snprintf (msg, 20, "%d", bitRead(devices[1].flags, OUTPUT_BIT));
				 mqttClient.publish(String(mqttRootTopic + B_TOPIC).c_str(), msg);
				 snprintf (msg, 20, "%d", bitRead(devices[2].flags, OUTPUT_BIT));
				 mqttClient.publish(String(mqttRootTopic + C_TOPIC).c_str(), msg);
				 snprintf (msg, 20, "%d", bitRead(devices[3].flags, OUTPUT_BIT));
				 mqttClient.publish(String(mqttRootTopic + D_TOPIC).c_str(), msg);
				 isErrorConn = false;
				 DRAWMESSAGE(display, "MQTT DONE");
			}
			else {
				DRAWMESSAGE(display, "MQTT ERROR");
			}
			mqttLock.clear();
		}
#endif

#ifdef ESP32_THREAD
	while (1) {
#else
	if(commInterval && (millis() - commMillis > commInterval * 1000)) {
#endif

		digitalWrite(LED0_PIN, false);

		DRAWMESSAGE(display, "BOT CONN...");
		//isErrorConn = !bot.checkForOkResponse(String("https://api.telegram.org/bot") + botToken);
		//Serial.println(String("https://api.telegram.org/bot") + botToken);

		//isErrorConn = true;
		//isErrorConn = !bot.getMe();
		//if(isErrorConn)
		//	isCheckIn = false;

		if(bot.getMe())
			errorConnCounter = 0;
		else
			errorConnCounter++;

		if(errorConnCounter > 2)
			isCheckIn = false;

		int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
		while(numNewMessages) {

			for (int i = 0; i < numNewMessages; i++) {

				Serial.println(bot.messages[i].chat_id);
				Serial.println(bot.messages[i].text);

				bot.messages[i].chat_id.toCharArray(lastBotChatId, 32);
				lastBotChatId[sizeof(lastBotChatId) - 1] = '\0';
				//if(bot.messages[i].chat_id != String(botChatId)) {
				//	bot.messages[i].chat_id.toCharArray(botChatId, 32);
				//	EEPROM.put(8 + sizeof(wwwUsername) + sizeof(wwwPassword) + sizeof(botToken), botChatId);
				//	EEPROM.commit();
				//}

				yield();
				//bot.sendMessage(bot.messages[i].chat_id, bot.messages[i].text, "");

				String msg = "";
				msg += "SSID:";
				msg += WiFi.SSID();
				msg += "\n";
				msg += "IP:";
				msg += WiFi.localIP().toString();
				msg += "\n";

				for(int i = 0; i < DEVICES_NUM; i++) {
					if(strlen(devices[i].name) == 0)
						continue;

					msg += String(char(i + 65)) + ":" + devices[i].name + " " + deviceToString(devices[i]) + " ";
					if(i < DEVICES_DIGITAL_NUM)
						msg += String(bitRead(devices[i].flags, MANUAL_BIT));
					else
						msg += String(devices[i].val);
					msg += "\n";
				}
				bot.sendMessage(bot.messages[i].chat_id, msg, "");
				yield();
			}
			numNewMessages = bot.getUpdates(bot.last_message_received + 1);
		}

		//if(botChatId[0] && !isErrorConn) {
		if(botChatId[0] && errorConnCounter == 0) {
			String msg = "";

			if(!isCheckIn) {
				isCheckIn = true;
				msg += "ONLINE\n";
			}

			for(int i = 0; i < DEVICES_NUM; i++) {
				if(strlen(devices[i].name) == 0)
						continue;

				if(bitRead(devices[i].flags, UNACK_BIT)) {
					msg += String(char(i + 65)) + ":" + devices[i].name + " " + deviceToString(devices[i]) + " ";
					if(i < DEVICES_DIGITAL_NUM)
						msg += String(bitRead(devices[i].flags, MANUAL_BIT));
					else
						msg += String(devices[i].val);
					msg += "\n";
					bitClear(devices[i].flags, UNACK_BIT);
				}
			}

			if(msg != "") {
				Serial.println(msg);
				yield();
				bot.sendMessage(botChatId, msg, "");
				yield();
			}
		}

		DRAWMESSAGE(display, "BOT DONE");

#ifdef MQTT
		DRAWMESSAGE(display, "NTP CONN...");
		timeClient.update();
		DRAWMESSAGE(display, "NTP DONE");
#endif

#ifdef SD_CS_PIN
		if(isSD) {
			DRAWMESSAGE(display, "SD LOG");
			int fileIndex = 0;

			time_t t = CE.toLocal(timeClient.getEpochTime());
			String path = "/" + String(year(t)) + "-" + int2string(month(t)) + "-" + int2string(day(t)) + ".csv";


			String message = String(year(t)) + "-" + int2string(month(t)) + "-" + int2string(day(t)) + " " + int2string(hour(t)) + ":" + int2string(minute(t)) + ":" + int2string(second(t)) + ";"
					+ String(bitRead(devices[DEV_ALARM_MAX].flags, OUTPUT_BIT) | bitRead(devices[DEV_ALARM_MIN].flags, OUTPUT_BIT)) + ";"
					+ String(level) + ";" + String(bitRead(devices[0].flags, OUTPUT_BIT)) + ";" + String(bitRead(devices[1].flags, OUTPUT_BIT)) + ";" + String(bitRead(devices[2].flags, OUTPUT_BIT)) + ";" + String(bitRead(devices[3].flags, OUTPUT_BIT)) + ";" + String(mqttState) + '\n';
			Serial.print(message);
			errorSD = false;
			if(!SD.exists(path)) {
				File file = SD.open(path, FILE_WRITE);
				if(file) {
					file.print("DATE TIME;ALARMS;LEVEL[mm];A: " + String(devices[0].name) + ";B: " + String(devices[1].name) +";C: " + String(devices[2].name) + ";D: " + String(devices[3].name) + ";MQTT\n");
					file.close();
				}
				else {
					Serial.println("Failed to create file");
					errorSD = true;
				}
			}
			File file = SD.open(path, FILE_APPEND);
			if(!file) {
				Serial.println("Failed to open file for appending");
				errorSD = true;
			}
			if(file) {
				if(file.print(message)){
					 //errorSD = false;
					 //Serial.println("Message appended");
				 }
				 else {
					 errorSD = true;
					 Serial.println("Failed to append file");
				 }
				 file.close();
			}
			if(errorSD)
				DRAWMESSAGE(display, "SD ERROR");
			else
				DRAWMESSAGE(display, "SD DONE");
		}
#endif

		//if(!checkIn) {
		//	DRAWMESSAGE(display, "LOG CONN...");
		//	http.begin("http://growmat.cz/growmatweb/dev/");
		//	httpCode = http.GET();
		//	if(httpCode > 0)
		//		checkIn = true;
		//	DRAWMESSAGE(display, "LOG DONE");
		//}

#ifdef THINGSSPEAK
		if(serverName[0] != 0 && !isAP) {
			do {
#ifndef ESP8266
				drawMessage(&display, "TB CONN...");
#endif

				String getTalkback = "http://" + String(serverName) + "/talkbacks/" + String(talkbackID) + "/commands/execute?api_key=" + talkbackApiKey;
				Serial.println(getTalkback);

				//TODO:
				http.begin(getTalkback);
				httpCode = http.GET();                                                                  //Send the request
				Serial.println(httpCode);
				Serial.println(http.errorToString(httpCode));

#ifndef ESP8266
				drawMessage(&display, "TB DONE");
#endif

				if(httpCode != 200) {
					httpErrorCounter++;
					isErrorConn = true;
				}
				else {
					isErrorConn = false;
				}
				if (httpCode > 0) { //Check the returning code
					String payload = http.getString();   //Get the request response payload
					Serial.println(payload);                     //Pri
					if(payload == "")
						break;
					if(payload == "error_auth_required")
						break;
					if(payload.charAt(0)=='A') {
						if(payload.charAt(1)=='0') {
							bitClear(devices[0].flags, OUTPUT_BIT);
							bitSet(devices[0].flags, MANUAL_BIT);
						}
						else if(payload.charAt(1)=='1') {
							bitSet(devices[0].flags, OUTPUT_BIT);
							bitSet(devices[0].flags, MANUAL_BIT);
						}
						else if(payload.charAt(1)=='A') {
							bitClear(devices[0].flags, RUNONCE_BIT);
							bitClear(devices[0].flags, MANUAL_BIT);
						}
					}
					if(payload.charAt(0)=='B') {
						if(payload.charAt(1)=='0') {
							bitClear(devices[1].flags, OUTPUT_BIT);
							bitSet(devices[1].flags, MANUAL_BIT);
						}
						else if(payload.charAt(1)=='1') {
							bitSet(devices[1].flags, OUTPUT_BIT);
							bitSet(devices[1].flags, MANUAL_BIT);
						}
						else if(payload.charAt(1)=='A') {
							bitClear(devices[1].flags, RUNONCE_BIT);
							bitClear(devices[1].flags, MANUAL_BIT);
						}
					}
				}
				else
					break;
			} while (true);
			String isAlarm = String(bitRead(devices[3].flags, OUTPUT_BIT) * 1 + bitRead(devices[4].flags, OUTPUT_BIT) * 10);
			String v1 = String(bitRead(devices[0].flags, OUTPUT_BIT) + bitRead(devices[0].flags, MANUAL_BIT) * 10);
			String v2 = String(bitRead(devices[1].flags, OUTPUT_BIT) + bitRead(devices[1].flags, MANUAL_BIT) * 10);
			//String get = GET + "SGRXDOXDL4F6CIGQ" + "&field1=" + isAlarm +"&field2="+ String(temperature) + "&field3=" + l  + "&field4=" + f;
			//String get = GET + writeApiKey + "&field1=" + isAlarm +"&field2="+ String(temperature) + "&field3=" + l  + "&field4=" + f;
			String get = "http://" + String(serverName) + "/update?key=" + writeApiKey + "&field1=" + isAlarm + "&field2=" + String(level) +  "&field3="+ v1  + "&field4=" + v2 + "&field5=" + millis() +  "&field6=" + httpErrorCounter +  "&field7=" + httpCode;
			Serial.println(get);
			DRAWMESSAGE(display, "TS CONN ...");
			//TODO:
			http.begin(get);
			httpCode = http.GET();
			Serial.println(httpCode);
			Serial.println(http.errorToString(httpCode));
			DRAWMESSAGE(display, "TS DONE");
			if(httpCode != 200) {
				httpErrorCounter++;
				isErrorConn = true;
			}
			else {
				isErrorConn = false;
			}
			if (httpCode > 0) { //Check the returning code
				String payload = http.getString();   //Get the request response payload
				Serial.println(payload);                     //Print the response payload
			}
			http.end();   //Close connection
		}
#endif

		if(reconnNum && (reconnCount > reconnNum)) {
			reconnCount = 0;
			for (int i = 0; i < 3; i++) {
				 if(connectWiFi())
					 break;
			}
			if (isAP)
				startWiFiAP();
		}
		if(isAP)
			reconnCount++;

		commMillis = millis();

#ifdef ESP32_THREAD
		delay(20000);
#endif
	}
}

/////////////////////////////////////
// loop
/////////////////////////////////////
void loop() {
	//drd.loop();
	// WDT test
	//ESP.wdtDisable(); //disable sw wdt, hw wdt keeps on
	//while(1){};

	ArduinoOTA.handle();
	httpServer.handleClient();

#ifndef ESP32_THREAD
	loopComm(0);
#endif

	for(int i = 0; i < DEVICES_NUM; i++) {
		if(i < DEVICES_DIGITAL_NUM) {
			bool pinValue = digitalRead(inputPins[i]);
			if(devices[i].par3) {
				pinValue = !pinValue;
			}

			if(bitRead(devices[i].flags, MANUAL_BIT) != pinValue) {
				devices[i].millis = millis();
				if(pinValue)
					bitSet(devices[i].flags, MANUAL_BIT);
				else
					bitClear(devices[i].flags, MANUAL_BIT);
			}

			if(bitRead(devices[i].flags, MANUAL_BIT)) {
				if(millis() - devices[i].millis > (unsigned long)devices[i].par1 * 1000L) {
					if(!bitRead(devices[i].flags, OUTPUT_BIT))
						bitSet(devices[i].flags, UNACK_BIT);
					bitSet(devices[i].flags, OUTPUT_BIT);
				}
			}
			else {
				if(millis() - devices[i].millis > (unsigned long)devices[i].par1 * 1000L) {
					if(bitRead(devices[i].flags, OUTPUT_BIT))
						bitSet(devices[i].flags, UNACK_BIT);
					bitClear(devices[i].flags, OUTPUT_BIT);
				}
				//bitClear(devices[i].flags, OUTPUT_BIT);
			}
		}
		else {
			if(devices[i].val < devices[i].par2 || devices[i].val > devices[i].par1) {
				if(!bitRead(devices[i].flags, OUTPUT_BIT))
					bitSet(devices[i].flags, UNACK_BIT);
				bitSet(devices[i].flags, OUTPUT_BIT);
			}
			else {
				if(bitRead(devices[i].flags, OUTPUT_BIT))
					bitSet(devices[i].flags, UNACK_BIT);
				bitClear(devices[i].flags, OUTPUT_BIT);
			}
		}
	}

	bool isAlarm = false;
	bool isUnack = false;
	for(int i = 0; i < DEVICES_NUM; i++) {
		isAlarm |= bitRead(devices[i].flags, OUTPUT_BIT);
		isUnack |= bitRead(devices[i].flags, UNACK_BIT);
	}
	isAlarm |= errorConnCounter; //isErrorConn;

	if(!digitalRead(CONFIG_WIFI_PIN)) {
		screenSaverMillis = millis();

		for(int i = 0; i < DEVICES_NUM; i++) {
			bitClear(devices[i].flags, UNACK_BIT);
		}
	}

#ifdef LCD
	display.clearDisplay();
	display.setCursor(0,0);
	display.println(WiFi.localIP());
	display.println(timeClient.getFormattedTime());
	display.display();
#endif

#ifdef LED0_PIN
	//if(millis() - ledMillis > (isErrorConn ? 125 : 1000)) {
	if(millis() - ledMillis > (errorConnCounter > 2 ? 125 : 1000)) {
		ledMillis = millis();
		digitalWrite(LED0_PIN, !digitalRead(LED0_PIN));
	}
#endif

	if (millis() - secMillis >= 1000) {
		secMillis = millis();

		oneWireSensors.requestTemperatures();
		int i = DEVICES_DIGITAL_NUM + (secCounter % (DEVICES_NUM - DEVICES_DIGITAL_NUM));

		//for(int i = DEVICES_DIGITAL_NUM; i < DEVICES_NUM; i++) {
			float val =  oneWireSensors.getTempCByIndex(i - DEVICES_DIGITAL_NUM);
			//delay(100);
			//Serial.println(val);
			if(val == -127.0) {
				if(devices[i].par4 > 2)
					devices[i].val = val;
				else
					devices[i].par4++;
			}
			else {
				devices[i].val = val;
				devices[i].par4 = 0;
			}
			//Serial.println(devices[i].val);
		//}
		//Serial.println();

#ifdef MQTT
  		if(!mqttLock.test_and_set()) {
  			mqttClient.loop();
  			//Serial.print("MQTT:");
  			//Serial.println(mqttClient.connected());
  			isErrorConn = !mqttClient.connected();
  			mqttLock.clear();
  		}
#endif

#ifndef ESP8266
  		//if(isUnack && isAlarm) {
  		//	if(secCounter % 2)
  		//		display.invertDisplay();
  		//	else
  		//		display.normalDisplay();
  		//}
  		if(digitalRead(CONFIG_WIFI_PIN)) {
  			frameNo++;
  		}
  		if(frameNo >= frameCount)
  			frameNo = 0;
  		drawDisplay(&display, frameNo);
#endif

#ifdef RFRX_PIN
		if (rcSwitch.available()) {
			Serial.print(rcSwitch.getReceivedValue());
			Serial.print('\t');
			Serial.print(rcSwitch.getReceivedBitlength());
			Serial.print('\t');
			Serial.print(rcSwitch.getReceivedDelay());
			Serial.print('\t');
			unsigned int* p = rcSwitch.getReceivedRawdata();
			for(int i = 0; i < RCSWITCH_MAX_CHANGES; i++)
				Serial.print(*(p + i));
			Serial.print('\t');
			Serial.print(rcSwitch.getReceivedProtocol());
		    rcSwitch.resetAvailable();
		    Serial.println();
		 }
#endif

//#ifdef LED0_PIN
//		if(isUnack && isAlarm) {
//			digitalWrite(LED0_PIN, !digitalRead(LED0_PIN));
//		}
//		else
//			digitalWrite(LED0_PIN, !isAlarm);
//#endif

#ifdef MQTT
		if(!mqttLock.test_and_set() && mqttServer[0] != 0) {
			//if (!mqttClient.connected())
			//	mqttConnect();
			yield();
			if (mqttClient.connected()) {
				for(int i = 0; i < DEVICES_NUM; i++) {
					if(bitRead(devices[i].flags, OUTPUT_BIT) != bitRead(devices[i].flags, PREVOUTPUT_BIT)) {
						if(bitRead(devices[i].flags, OUTPUT_BIT))
							bitSet(devices[i].flags, PREVOUTPUT_BIT);
						else
							bitClear(devices[i].flags, PREVOUTPUT_BIT);
						snprintf (msg, 20, "%d", bitRead(devices[i].flags, OUTPUT_BIT));
						if(i == 0) {
							mqttClient.publish(String(mqttRootTopic + A_TOPIC).c_str(), msg);
						}
						if(i == 1) {
							mqttClient.publish(String(mqttRootTopic + B_TOPIC).c_str(), msg);
						}
						if(i == 2) {
							mqttClient.publish(String(mqttRootTopic + C_TOPIC).c_str(), msg);
						}
						if(i == 3) {
								mqttClient.publish(String(mqttRootTopic + D_TOPIC).c_str(), msg);
						}
						if(i == DEV_ALARM_MAX) {
							mqttClient.publish(String(mqttRootTopic + LEVEL_MAX_TOPIC).c_str(), msg);
						}
						if(i == DEV_ALARM_MIN) {
							mqttClient.publish(String(mqttRootTopic + LEVEL_MIN_TOPIC).c_str(), msg);
						}
					}
				}
			}
			mqttLock.clear();
		}
#endif

		secCounter++;
	}
}
