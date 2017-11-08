/*

 ██╗██████╗ ██████╗ ███████╗
███║╚════██╗╚════██╗╚════██║
╚██║ █████╔╝ █████╔╝    ██╔╝
 ██║ ╚═══██╗ ╚═══██╗   ██╔╝ 
 ██║██████╔╝██████╔╝   ██║  
 ╚═╝╚═════╝ ╚═════╝    ╚═╝  
                            
 */

/* Analog Pin Map : 
 *  A0 - PhotoCell
 */

/* Digital Pin Map:
 *  D1 - LED Strip
 *  D2 - DHT11
 *  D3 - Master Switch
 *  D4 - Pressure Plate
 *  D6 - Lights
 *  D7 - Thermostat
 *  D8 - Digispark
 */

/* Virtual Pin Map:
 *  V1 - Ideal Temperature
 *  V2 - Room Temperature
 *  V3 - Light Status
 *  V4 - Thermostat Status
 *  V5 - PC Status
 *  V6 - LED Status
 *  V7 - zeRGBa
 *  V8 - LED Brightness
 *  V9 - Master Timeout
 */
 
#define BLYNK_PRINT Serial
#define DHTPIN D2
#define DHTTYPE 11
#define LEDPIN D1
#define LEDCOUNT 37

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <WS2812FX.h>
#include <WiFiUDP.h>

char auth[] = "f1ed5674a08345bc9382431a77a859a5";
char ssid[] = "1337";
char pass[] = "xX1337Xx";

// Global Variables:
BlynkTimer leetTimer;
DHT dht(DHTPIN, DHTTYPE);
WS2812FX ws2812fx = WS2812FX(LEDCOUNT, LEDPIN, NEO_GRB + NEO_KHZ800);
float idealTemp, roomTemp;
int desiredMode, desiredBrightness;
int r, g, b, desiredColor;
int masterTimeout, timeCounter;   // (seconds)
float photocellReading = 190;
bool pcTrigger = 0, ledTrigger = 0;

WiFiUDP udpc;

// Status Variables: (0 - OFF, 1 - ON, 2 - Auto)
int pcStatus;
int thermostatStatus;
int lightStatus;


// Functions:

void blynkDelay(int milli)
{
  int endTime = millis() + milli;
  while(millis() < endTime){
    Blynk.run();
    yield();   // Pass control to other tasks.
  }
}

void lightsON()
{
  digitalWrite(D6, HIGH);
}

void lightsOFF()
{
  digitalWrite(D6, LOW);
}

void lights()
{
  photocellReading = analogRead(A0);
  if(photocellReading <= 800)
    lightsON();
  else
    lightsOFF();
  yield();
}

void thermostatON()
{ 
  digitalWrite(D7, HIGH);
}

void thermostatOFF()
{
  digitalWrite(D7, LOW);
}

void thermostat()
{
  do{
  roomTemp = dht.readTemperature(false);
  }while(isnan(roomTemp));
  
  if (idealTemp > roomTemp)
    thermostatON();
  else
    thermostatOFF();
}

void pc()
{
  if(pcTrigger == 0){
    digitalWrite(D8, HIGH);
    blynkDelay(10000);
    digitalWrite(D8, LOW);
    pcTrigger = 1;
  }
}

void sendTemp()
{
  do{
  roomTemp = dht.readTemperature(false);
  }while(isnan(roomTemp));
  Blynk.virtualWrite(V2, roomTemp);
}


// Virtual Pin Functions:

BLYNK_WRITE(V1)
{
  idealTemp = param.asDouble();
}

BLYNK_WRITE(V3)
{
  lightStatus = param.asInt();
}

BLYNK_WRITE(V4)
{
  thermostatStatus = param.asInt();
}

BLYNK_WRITE(V5)
{
  pcStatus = param.asInt();
}

BLYNK_WRITE(V6)
{
  desiredMode = param.asInt();
  switch (desiredMode) {
    case 0:
      ws2812fx.setBrightness(0);
      ws2812fx.setColor(0x000000);
      ws2812fx.setSpeed(150);
      ws2812fx.setMode(FX_MODE_STATIC);
      ws2812fx.start();
      break;
    case 1:
      ws2812fx.setBrightness(desiredBrightness);
      ws2812fx.setColor(desiredColor);
      ws2812fx.setSpeed(150);
      ws2812fx.setMode(FX_MODE_STATIC);
      ws2812fx.start();
      break;
    case 2:
      ws2812fx.setBrightness(desiredBrightness);
      ws2812fx.setColor(desiredColor);
      ws2812fx.setSpeed(150);
      ws2812fx.setMode(FX_MODE_FADE);
      ws2812fx.start();
      break;
    case 3:
      ws2812fx.setBrightness(desiredBrightness);
      ws2812fx.setSpeed(200);
      ws2812fx.setMode(FX_MODE_COLOR_WIPE_RANDOM);
      ws2812fx.start();
      break;
    case 4:
      ws2812fx.setBrightness(desiredBrightness);
      ws2812fx.setSpeed(150);
      ws2812fx.setMode(FX_MODE_RAINBOW);
      ws2812fx.start();
      break;
    case 5:
      ws2812fx.setBrightness(desiredBrightness);
      ws2812fx.setSpeed(150);
      ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
      ws2812fx.start();
      break;
  }
}

BLYNK_WRITE(V7)
{
   r = param[0].asInt();
   g = param[1].asInt();
   b = param[2].asInt();
   desiredColor = b + (g << 8) + (r << 16);
   ws2812fx.setColor(desiredColor);
}

BLYNK_WRITE(V8)
{
  desiredBrightness = param.asInt();
  ws2812fx.setBrightness(desiredBrightness);
}

BLYNK_WRITE(V9)
{
  masterTimeout = param.asInt();
  masterTimeout *= 60;
}

BLYNK_CONNECTED()
{
  Blynk.syncAll();
}

void leet()
{
  sendTemp();
  if((digitalRead(D3) == LOW) || (digitalRead(D4) == LOW) || (timeCounter < (masterTimeout-1))){
    if(ledTrigger){
      ledTrigger = 0;
      ws2812fx.setBrightness(desiredBrightness);
    }
    switch(lightStatus){
      case 0:
        lightsOFF();
        break;
      case 1:
        lightsON();
        break;
      case 2:
        lights();
        break;
    }

    switch(thermostatStatus){
      case 0:
        thermostatOFF();
        break;
      case 1:
        thermostatON();
        break;
      case 2:
        thermostat();
        break;  
    }
    
    switch(pcStatus){
      case 0:
        break;
      case 1:
      case 2:
        pc();
        break;
    }
  }

  if((digitalRead(D3) == HIGH) && (digitalRead(D4) == HIGH)){
     timeCounter++;
  }else{
     timeCounter = 0;
  }

  if(timeCounter == (masterTimeout - 1)){
    pcTrigger = 0;
    ledTrigger = 1;
    lightsOFF();
    ws2812fx.setBrightness(0);
    thermostatOFF();
  }
  yield();
}

void setup()
{
  Serial.begin(115200);

  // Pin Setup:
  pinMode(D1, OUTPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);

  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
  
  udpc.begin(9);   // In case of future WOL integration.
  ws2812fx.init();
  leetTimer.setInterval(1000L, leet);
  Blynk.begin(auth, ssid, pass);
}

void loop()
{
  Blynk.run();
  leetTimer.run();
  ws2812fx.service();
}
