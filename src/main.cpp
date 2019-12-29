/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on NodeMCU.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right NodeMCU module
  in the Tools -> Board menu!

  For advanced settings please follow ESP examples :
   - ESP8266_Standalone_Manual_IP.ino
   - ESP8266_Standalone_SmartConfig.ino
   - ESP8266_Standalone_SSL.ino

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Include the correct display library
// For a connection via I2C using Wire include
//#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

// OTA Includes
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

//Config Includes
#include <config.h>

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, D1, D2);

// Esp8266 pins.
const int RED_PIN   = D6; // GPIO12
const int GREEN_PIN = D7; // GPIO13
const int BLUE_PIN  = D5; // GPIO14
const int WHITE_PIN = D8; // GPIO15
const int BTN_PIN   = D3; // GPIO0

bool Connected2Blynk = false;
String localIP; 

BlynkTimer timer;
int displayTimer;
int fadeTimer;
int virtualTimer; 
int connectTimer;

int w, r, g, b; 
int fadeMode = 0;
int fadeState = 0; 
int dimmer = 0; 

unsigned long oldUpdate; 


//********************************************//

void updateDisplay();
void fadeLED();
void virtualUpdate();
void breath(int PIN);
void checkConnection();

//********************************************//

BLYNK_WRITE(V0) 
{   
  dimmer = param.asInt(); 
  oldUpdate = millis();
}

BLYNK_WRITE(V1) // Widget WRITEs to Virtual Pin V1
{   
  w = param.asInt(); 
  analogWrite(WHITE_PIN, w);
  oldUpdate = millis(); 
}

BLYNK_WRITE(V2) 
{   
  r = param.asInt(); 
  analogWrite(RED_PIN, r);
  oldUpdate = millis(); 
}

BLYNK_WRITE(V3) 
{   
  g = param.asInt(); 
  analogWrite(GREEN_PIN, g); 
  oldUpdate = millis();
}

BLYNK_WRITE(V4) 
{   
  b = param.asInt(); 
  analogWrite(BLUE_PIN, b);
  oldUpdate = millis(); 
}


BLYNK_WRITE(V5) 
{   
  fadeState = param.asInt(); 
  if(fadeState){
    timer.enable(fadeTimer);
  }else{
    timer.disable(fadeTimer);
  }
  oldUpdate = millis();
}

//Off
BLYNK_WRITE(V6) 
{   
  if(param.asInt()){
    if(fadeState){
      fadeState = 0; 
      timer.disable(fadeTimer); 
    }
    breath(RED_PIN); 
    r = 0; g = 0; b = 0; w = 0;
    analogWrite( RED_PIN, r); 
    analogWrite( BLUE_PIN, g); 
    analogWrite( GREEN_PIN, b); 
    analogWrite( WHITE_PIN, w); 
    
    virtualUpdate();
    oldUpdate = millis();
  }
}

BLYNK_CONNECTED() {
  Blynk.syncAll();
  breath(BLUE_PIN);
  oldUpdate = millis();
}



//***********************************************//

// Timed function to update the display 
void updateDisplay(){
  display.clear(); 

  // only update if data is fresh, else clear to prevent screen burn. 
  if(millis() - oldUpdate < 10000){
  
  if(WiFi.status() == WL_CONNECTED){
    display.drawString(0, 0,  localIP);
  }else{
    display.drawString(0, 0, "WiFi Fail :(");
  }

  display.drawString(0, 12, "W:" + String(w) );
  display.drawString(0, 24, "R:" + String(r) );
  display.drawString(0, 36, "G:" + String(g) );
  display.drawString(0, 48, "B:" + String(b) );
  
  display.drawString(50, 12, "F:" + String(fadeState) + String(fadeMode) );
  display.drawString(50, 24, "Dim:" + String(dimmer) );  
  display.display(); 

  Serial.println(String(w));
  Serial.println(String(r));
  Serial.println(String(g));
  Serial.println(String(b));
  Serial.println(String(fadeMode) + String(fadeState));
  Serial.println(String(dimmer)+"/n");
  }
}

// fade function for pretties
void fadeLED(){
  switch(fadeMode){
    case 0: //Red Up 
      analogWrite(RED_PIN,r++);
      if(r>1023){
        r = 1023; 
        fadeMode++;
      }
     break;

    case 1: //Green Up 
      analogWrite(GREEN_PIN,g++);
      if(g>1023){
        g = 1023; 
        fadeMode++;
      }
     break;

    case 2: //Red Down 
      analogWrite(RED_PIN,r--);
      if(r<0){
        r = 0; 
        fadeMode++;
      }
     break;

    case 3: //Blue Up 
      analogWrite(BLUE_PIN,b++);
      if(b>1023){
        b = 1023; 
        fadeMode++;
      }
     break;

    case 4: //Green Down 
      analogWrite(GREEN_PIN,g--);
      if(g<0){
        g = 0; 
        fadeMode++;
      }
     break;

    case 5: //Red Up 
      analogWrite(RED_PIN,r++);
      if(r>1023){
        r = 1023; 
        fadeMode++;
      }
     break;

    case 6: //Blue Down 
      analogWrite(BLUE_PIN,b--);
      if(b<0){
        b = 0; 
        fadeMode = 1;
      }
     break;
    
    default:
      fadeMode = 0;
      r=0; g=0; b=0; 
     break;
  }
}

// Update the data in the app 
void virtualUpdate(){
  Blynk.virtualWrite(V0, dimmer);
  Blynk.virtualWrite(V1, w);
  Blynk.virtualWrite(V2, r);
  Blynk.virtualWrite(V3, g);
  Blynk.virtualWrite(V4, b);
  Blynk.virtualWrite(V5, fadeState);
}

//Little breathing animation
void breath(int PIN){
  for(int c = 0; c < 2 ; c++){
    for( int i = -1023; i <= 1023; i++){
      analogWrite(PIN, 1023-abs(i)); 
      delay(1); 
    }
  }
}

void checkConnection(){    // check every 11s if connected to Blynk server
  if(!Blynk.connected()){
    Serial.println("Not connected to Blynk server"); 
    Blynk.connect();  // try to connect to server with default timeout
  }
  else{
    Serial.println("Connected to Blynk server");     
  }
}

//********************************************//

void setup()
{
  // Debug console
  Serial.begin(115200);

  // LED Output Setup 
  pinMode( RED_PIN, OUTPUT );
  pinMode( GREEN_PIN, OUTPUT );
  pinMode( BLUE_PIN, OUTPUT );
  pinMode( WHITE_PIN, OUTPUT );
  pinMode( BTN_PIN, INPUT );

  r=0; 
  g=0;
  b=0; 
  w=0; 
  analogWrite( RED_PIN, r); 
  analogWrite( BLUE_PIN, g); 
  analogWrite( GREEN_PIN, b); 
  analogWrite( WHITE_PIN, w); 


  // initialize display
  display.init();
  //display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setContrast(255);
  display.drawString(0, 0, "Connecting...");
  display.drawString(0, 12, "SSID:");
  display.drawString(28, 12, ssid);
  display.display(); 

  WiFi.hostname("ESP_KitchenBlynk");

  // Connect to wifi
  WiFi.begin(ssid, pass);  
  int n = 0; 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.drawString(n, 24, ".");
    display.display(); 
    n = n+8; 
    if(n>120){break;}
  }


  // Connect Blynk 
  Blynk.config(auth);

  if(WiFi.status() == WL_CONNECTED){
    localIP = WiFi.localIP().toString();

  display.clear();
  display.drawString(0, 0, "WiFi Connected!");
  display.drawString(0, 12, localIP);
  display.drawString(0, 24, "Connecting Blynk");
  display.display(); 
  //Blynk.begin(auth, ssid, pass);
  Connected2Blynk = Blynk.connect(); // default timeout


  display.clear();
  display.drawString(0, 36, "Connected Blynk... ");
  display.display(); 

  display.drawString(0, 48, "Setting Up OTA.. ");
  display.display(); 

  ArduinoOTA.begin();
  ArduinoOTA.onStart([]() {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.drawString(display.getWidth()/2, display.getHeight()/2 - 10, "OTA Update");
    display.display();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    display.drawProgressBar(4, 32, 120, 8, progress / (total / 100) );
    display.display();
  });

  ArduinoOTA.onEnd([]() {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.drawString(display.getWidth()/2, display.getHeight()/2, "Restart");
    display.display();
  });

  // Wifi Failed
  }  else{
  display.clear();
  display.drawString(0, 0, "WiFi Fail :(");
  display.display(); 
  }

  // Setup a function to be called x millisecond
  displayTimer = timer.setInterval(100L, updateDisplay);
  fadeTimer = timer.setInterval(20L, fadeLED); 
  virtualTimer = timer.setInterval(1000L, virtualUpdate);
  connectTimer = timer.setInterval(10000L, checkConnection);
  timer.disable(fadeTimer); 
  timer.disable(virtualTimer); 

  fadeState=0;

  // update the App 
  virtualUpdate();


  // blink green on boot
  breath(GREEN_PIN);
}

void loop()
{
  // do Blynk Things
  if(Blynk.connected()){
    Blynk.run();
  }

  // run the Blynk Timer
  timer.run();  
  
  // White Button Overide
  if(!digitalRead(BTN_PIN)){ // Pressed
    delay(30); // debounce
    while(!digitalRead(BTN_PIN)); // Pressed, just wait
    
    if( w > 0 ) w = 0;
    else w = 1023; 
    r = 0; g = 0; b = 0; fadeState = 0; timer.disable(fadeTimer); 
    analogWrite(WHITE_PIN, w); 
    analogWrite(RED_PIN, r); 
    analogWrite(GREEN_PIN, g); 
    analogWrite(BLUE_PIN, b); 
    // update the App 
    virtualUpdate();
    oldUpdate = millis();
  }

  //Do OTA stuff
  ArduinoOTA.handle();
}