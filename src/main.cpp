#include <WiFiUdp.h>
#include <WiFiManager.h> 
#include <OSCMessage.h> 
#include <analogWrite.h> 
#include "FastAccelStepper.h"
#include <Nextion.h>
#include <EasyNextionLibrary.h>  
#include <SerialMP3Player.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <DNSServer.h>  

AsyncWebServer asyncServer(80);


#define A1 100
#define A2 100
#define A3 100
#define A4 100
#define F1 320
#define F2 1280
#define F3 2240
#define F4 3200
/*#define F1 1280
#define F2 5120
#define F3 8960
#define F4 12800*/
  
File compositionFile;
int compositionMusicStartTime;
int compositionTimestamp = 0;
boolean compositionMode = false;
int compositionDimmer;
float lastCompositionDimmer = 0;
float lastCompositionTimestamp;
int currentCompBright = 0;
float compositionTimePaused;
boolean compositionPaused = false;
String compositionFileName;
int compositionFileNumber = 1;
boolean scientificMode = false;

int sciTimestamp1 = 0;
int sciTimestamp2 = 0;
float sciWave1 = 0;
float sciWave2 = 0;

// CONFIGS!!
// ----------------------------------------------

// Set this device's name here!
// Set different names for different devices (ex: luz2, luz3, etc)
// Do NOT use "/" here, it is auto added to generate the device address later
# define DEVICE_NAME "luz2"

// OSC address to speak to all devices (set with "/")
// Not recommended to change!
# define MULTI_ROUTE1 "/global/1/1"
# define MULTI_ROUTE2 "/global/1/2"
# define MULTI_ROUTE3 "/global/1/3"
# define MULTI_ROUTE4 "/global/1/4"
# define MULTI_ROUTE5 "/global/1/5"
# define PP_ROUTE "/playpause"
boolean mp3Status = false;
# define PREVIOUS_ROUTE "/previous"
# define NEXT_ROUTE "/next"
# define DOWN_ROUTE "/vdown"
# define UP_ROUTE "/vup"


# define SYNC_ROUTE "/sync"
# define DIMMER_ROUTE "/dimmer"
# define COLOR_ROUTE "/color"
# define WHITE_ROUTE "/white"

#define SLIDER "/slider"

// Set signal out pin here!
//# define triacpin G26
// Set pin to read zero crossing signal here!
//# define zcpin G32

// Fade loop default delay (milliseconds). A full fade is this duration x 32.
// Full fade durations example: 5 = 160ms, 125 = 4s
// This value can be changed at runtime using the /global/fd address
int fadedelay = 20;

// ----------------------------------------------
// DO NOT CHANGE STUFF BELOW (unless you know what you're doing)

// used to hold the delay in use because everything uses fadeToValue() now
// this is set to fadedelay on fade to full / zero calls
// and set to 2-5 ms during regular calls
int actual_delay = fadedelay;
// normal delay for regular calls
#define normal_delay 4

//RX 17 e TX 16
/*#define B1 33
#define B2 12
#define B3 32**
#define B4 14*/

//NEO PIXEL DECLARATIONS
#define NEO_PIN 15
#define NUM_LEDS 2370
//#define DELAYVAL 500
//NEO_GRB + NEO_KHZ800
Adafruit_NeoPixel pixels(NUM_LEDS, NEO_PIN, NEO_GRBW + NEO_KHZ800);
//CRGB leds[NUM_LEDS];
//CHSV hsv;

#define mp3RX 13
#define mp3TX 0
#define NO_SERIALMP3_DELAY
int currentVolume = 20;
#define MAX_VOLUME 25;
SerialMP3Player mp3(mp3RX,mp3TX);

#define dirPinStepper 33
#define enablePinStepper 35
#define stepPinStepper 32

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;


//AccelStepper stepper(1,step,direction); 
//TM1638lite tm(18,17,16);
/*EasyButton button1(33,35,true,false);
EasyButton button2(12,35,true,false); 
EasyButton button3(32,35,true,false);
EasyButton button4(14,35,true,false);*/






NexDSButton bt0 = NexDSButton(2,3,"bt0");
NexDSButton bt1 = NexDSButton(2,4,"bt1");
NexDSButton bt2 = NexDSButton(2,5,"bt2");
NexDSButton bt3 = NexDSButton(2,6,"bt3");
NexDSButton bt4 = NexDSButton(2,7,"bt4");


NexDSButton whiteButton = NexDSButton(2,19,"bt9");
NexDSButton pinkButton = NexDSButton(2,16,"bt3");
NexDSButton purpleButton = NexDSButton(2,17,"bt4");
NexDSButton orangeButton = NexDSButton(2,18,"bt5");
NexDSButton redButton = NexDSButton(2,20,"bt2");
NexDSButton blueButton = NexDSButton(2,21,"bt6");
NexDSButton greenButton = NexDSButton(2,22,"bt7");
NexDSButton yellowButton = NexDSButton(2,23,"bt8");
NexDSButton decorButton = NexDSButton(2,24,"bt5");

NexSlider dimmerSlider = NexSlider(2,1,"h2");

NexButton ppButton = NexButton(3,1,"b0");
NexButton nextButton = NexButton(3,3,"b1");
NexButton previousButton = NexButton(3,2,"b2");
NexButton upButton = NexButton(3,5,"b3");
NexButton downButton = NexButton(3,4,"b4");


NexButton downloadButton = NexButton(4,23,"b6");
NexButton playCompositionButton = NexButton(4,21,"b1");
NexButton upButtonComp = NexButton(4,25,"b2");
NexButton downButtonComp = NexButton(4,24,"b0");
NexButton stopCompositionButton = NexButton(4,22,"b7");

NexDSButton comp1 = NexDSButton(4,3,"bt0");
NexDSButton comp2 = NexDSButton(4,4,"bt1");
NexDSButton comp3 = NexDSButton(4,5,"bt2");
NexDSButton comp4 = NexDSButton(4,6,"bt3");
NexDSButton comp5 = NexDSButton(4,7,"bt4");
NexDSButton comp6 = NexDSButton(4,8,"bt5");
NexDSButton comp7 = NexDSButton(4,9,"bt6");
NexDSButton comp8 = NexDSButton(4,10,"bt7");
NexDSButton comp9 = NexDSButton(4,11,"bt8");
NexDSButton comp10 = NexDSButton(4,12,"bt9");
NexDSButton comp11 = NexDSButton(4,13,"bt10");
NexDSButton comp12 = NexDSButton(4,14,"bt11");
NexDSButton comp13 = NexDSButton(4,15,"bt12");
NexDSButton comp14 = NexDSButton(4,16,"bt13");
NexDSButton comp15 = NexDSButton(4,17,"bt14");

NexButton updateButton = NexButton(5,1,"b3");
 
NexTouch *nex_listen_list[] = {
  &whiteButton,
  &pinkButton,
  &purpleButton,
  &orangeButton,
  &redButton,
  &blueButton,
  &greenButton,
  &yellowButton,
  &decorButton,
  &stopCompositionButton,
  &playCompositionButton,
  &downloadButton,
  &updateButton,
  &dimmerSlider,
  &ppButton,
  &nextButton,
  &previousButton,
  &upButton,
  &downButton,
  &bt0,
  &bt1,
  &bt2,
  &bt3,
  &bt4,
  &comp1,
  &comp2,
  &comp3,
  &comp4,
  &comp5,
  &comp6,
  &comp7,
  &comp8,
  &comp9,
  &comp10,
  &comp11,
  &comp12,
  &comp13,
  &comp14,
  &comp15,
  &upButtonComp,
  &downButtonComp,
  NULL};


//dimmerLamp dimmer(triacpin, zcpin);

// UDP communication


// stores the generated fade setting path
char fdroute[sizeof(MULTI_ROUTE1)+3];
char devroute[sizeof(DEVICE_NAME)+1];

// current light level
int current_dimval = 0;
// requested light level
int new_dimval = 0;

int currentSpeed = 0;

uint8_t DisBuff[2 + 5 * 5* 3];

int myButtons[] = {1,0,0,0,0};
char buttonsBuffer[20];

char* myRoutes[] = {"/global/1/5","/global/1/4","/global/1/3","/global/1/2","/global/1/1"};


char buffer[10];

int rainbowValue = 0;
int redValue = 0;
int greenValue = 0;
int blueValue = 0;

boolean receivingPackets = false;
unsigned long previousMillis = 0;
unsigned long previousMillisColor = 0;
unsigned long previousMillisAudio = 0;
const long interval = 500;
int colorSaturation = 0;
boolean colorReceived = false;
int newHue = 0;
int j=0;
int currentHue = 0;
int currentBrightness = 128;
/*boolean trueWhite = false;
boolean normalWhite = false;*/
uint32_t whiteMode;
boolean whiteStatus = true;


WiFiServer server(8888);
WiFiUDP Udp;
const unsigned int inPort = 8888;
const IPAddress outIP(192,168,4,255);
char serverSSID[20];
WiFiManager wm;
#define AP 1
boolean WIFI_MODE = AP;
//WiFiUDP Udp;
void applyCompositionChanges(int dimmer, String wave, String color);

void whiteNextionCallback(void *ptr){
  Serial.println("Entered White");
  applyCompositionChanges(NULL,"","white");
}
void pinkNextionCallback(void *ptr){
  Serial.println("Entered pink");
  applyCompositionChanges(NULL,"","pink");
}
void redNextionCallback(void *ptr){
  Serial.println("Entered red");
  applyCompositionChanges(NULL,"","red");
}
void greenNextionCallback(void *ptr){
  Serial.println("Entered green");
  applyCompositionChanges(NULL,"","green");
}
void orangeNextionCallback(void *ptr){
  Serial.println("Entered orange");
  applyCompositionChanges(NULL,"","orange");
}
void yellowNextionCallback(void *ptr){
  Serial.println("Entered yellow");
  applyCompositionChanges(NULL,"","yellow");
}
void purpleNextionCallback(void *ptr){
  Serial.println("Entered purple");
  applyCompositionChanges(NULL,"","purple");
}
void blueNextionCallback(void *ptr){
  Serial.println("Entered blue");
  applyCompositionChanges(NULL,"","blue");
}




void whiteCallback(OSCMessage &msg) { 
  if (msg.isFloat(0)) {
    Serial.println("White Mode Active");
    whiteStatus = true;
    int value = msg.getFloat(0);
    pixels.fill(whiteMode,0,NUM_LEDS);
    pixels.show();
  }
}



void changeBrightness(int newBrightness){
  if(whiteStatus){
    pixels.fill(whiteMode,0,NUM_LEDS);
  }
  else{
    pixels.fill(pixels.ColorHSV(currentHue,255,255),0,NUM_LEDS);
  }
  currentBrightness = newBrightness;
  pixels.setBrightness(newBrightness);
  if(newBrightness==255){
    newBrightness=254;
  }
  pixels.show();
}

void dimmerNextionCallback(void *ptr){
  Serial.println("Dimmer was pressed");
  uint32_t number = 0;
  dimmerSlider.getValue(&number);
  int newBrightness = 255*number/100;
  Serial.println(newBrightness);
  changeBrightness(newBrightness);
  OSCMessage msgOUT("/dimmer");
  float newNumber = newBrightness/255.0;
  Serial.println(newNumber);
  msgOUT.add(newNumber);
  for(int count = 0; count < 10; count++){
    Udp.beginPacket(outIP, inPort);
    msgOUT.send(Udp);
    Udp.endPacket();
    count++;
  }
  msgOUT.empty();
  
}

void dimmerCallback(OSCMessage &msg){
  if (msg.isFloat(0)){
    receivingPackets = true;
    float value = msg.getFloat(0);
    int newBrightness = 255*value;
    changeBrightness(newBrightness);    
    int newvalue = value*100;
    sprintf(buffer, "h2.val=%d", newvalue);
    unsigned long currentMillis = millis();
    previousMillis = currentMillis;
  }
}


void updateDimmer(){
  OSCMessage msgOUT(DIMMER_ROUTE);
  float value = currentBrightness/255.0;
  msgOUT.add(value);
  Serial.println(currentBrightness);
  Serial.println(currentBrightness/255.0);
  for(int count = 0; count < 10; count++){
    //Serial.println("TESTE");
    Udp.beginPacket(outIP, inPort);
    msgOUT.send(Udp);
    Udp.endPacket();
    count++;
  }
  msgOUT.empty();
  
}
void updateColor(){
  OSCMessage msgOUT(COLOR_ROUTE);
  float value = currentHue/65535.0;
  msgOUT.add(value);
  Serial.println(currentHue);
  Serial.println(currentHue/65535.0);
  for(int count = 0; count < 10; count++){
    //Serial.println("TESTE");
    Udp.beginPacket(outIP, inPort);
    msgOUT.send(Udp);
    Udp.endPacket();
    count++;
  }
  msgOUT.empty();
}
void changeRemoteLevel(char* Button){
  OSCMessage msgOUT(Button);
  msgOUT.add(1);
  for(int count = 0; count < 10; count++){
    //Serial.println("TESTE");
    Udp.beginPacket(outIP, inPort);
    msgOUT.send(Udp);
    Udp.endPacket();
    count++;
  }
  msgOUT.empty();
}
void syncCallback(OSCMessage &msg){
  for(int count = 0; count < 5; count++){
    if(myButtons[count]==1){
      changeRemoteLevel(myRoutes[count]);
    }
  }
  updateDimmer();
  updateColor();
}

void updateMyButtons(int Button){
  for(int count = 0; count < 5; count++){
    if(count!=Button){
      myButtons[count]=0;
    }
  }
  myButtons[Button]=1;
}


void bt0PopCallback(void *ptr){
    Serial.println("Button 0 was pressed!");
    stepper->stopMove();
    changeRemoteLevel(MULTI_ROUTE5);
    currentSpeed = 0;
    updateMyButtons(0);
}
void bt1PopCallback(void *ptr){
    Serial.println("Button 1 was pressed!");
    stepper->setAcceleration(A1);
    stepper->setSpeedInHz(F1);
    stepper->runBackward();
    changeRemoteLevel(MULTI_ROUTE4);
    currentSpeed = 1;
    updateMyButtons(1);
}
void bt2PopCallback(void *ptr){
    Serial.println("Button 2 was pressed!");
    stepper->setAcceleration(A2);
    stepper->setSpeedInHz(F2);
    stepper->runBackward();
    changeRemoteLevel(MULTI_ROUTE3);
    currentSpeed = 2;
    updateMyButtons(2);
}
void bt3PopCallback(void *ptr){
    Serial.println("Button 3 was pressed!");
    stepper->setAcceleration(A3);
    stepper->setSpeedInHz(F3);
    stepper->runBackward();
    changeRemoteLevel(MULTI_ROUTE2);
    currentSpeed = 3;
    updateMyButtons(3);

}
void bt4PopCallback(void *ptr){
    Serial.println("Button 4 was pressed!");
    stepper->setAcceleration(A4);
    stepper->setSpeedInHz(F4);
    stepper->runBackward();
    changeRemoteLevel(MULTI_ROUTE1);
    currentSpeed = 4;
    updateMyButtons(4);
}

void ppNextionCallback(void *ptr){
  if(mp3Status){
    Serial.println("PAUSE MUSIC");
    mp3.pause();
    mp3Status = false;
  }
  else{
    Serial.println("RESUME MUSIC");
    mp3.play();
    mp3Status = true;
  }
}
void previousNextionCallback(void *ptr){
  Serial.println("PREVIOUS MUSIC");
  mp3.playPrevious();
  mp3Status = true;
}
void nextNextionCallback(void *ptr){
  Serial.println("NEXT MUSIC");
  mp3.playNext();
  mp3Status = true;
}
void upNextionCallback(void *ptr){
  Serial.println("UP VOLUME");
  if(currentVolume!=25){
    currentVolume+=5;
    mp3.setVol(currentVolume);
    mp3.qVol();
  }
}
void downNextionCallback(void *ptr){
  Serial.println("DOWN VOLUME");
  if(currentVolume!=0){
    currentVolume-=5;
    mp3.setVol(currentVolume);
    mp3.qVol();
  }
}
void upCallback(OSCMessage &msg){
  if (msg.isFloat(0)) {
    int value = msg.getFloat(0);
    if(value == 1.){
      Serial.println("UP VOLUME");
      if(currentVolume!=25){
        currentVolume+=5;
        mp3.setVol(currentVolume);
        mp3.qVol();
      }
    }
  }
}
void downCallback(OSCMessage &msg){
  if (msg.isFloat(0)) {
    int value = msg.getFloat(0);
    if(value == 1.){
      Serial.println("DOWN VOLUME");
      if(currentVolume!=0){
        currentVolume-=5;
        mp3.setVol(currentVolume);
        mp3.qVol();
      }
    }
  }
}
void nextCallback(OSCMessage &msg){
  if (msg.isFloat(0)) {
    int value = msg.getFloat(0);
    
    if(value == 1.){
      Serial.println("NEXT MUSIC");
      mp3.playNext();
      mp3Status = true;
    }
  }
}
void previousCallback(OSCMessage &msg){
  if (msg.isFloat(0)) {
    int value = msg.getFloat(0);
    if(value == 1.){
      Serial.println("PREVIOUS MUSIC");
      mp3.playPrevious();
      mp3Status = true;
    }
  }
}
void ppCallback(OSCMessage &msg) { 
  if (msg.isFloat(0)) {
    int value = msg.getFloat(0);
    if(value == 1.){
      if(mp3Status){
        Serial.println("PAUSE MUSIC");
        mp3.pause();
        mp3Status = false;
      }
      else{
        Serial.println("RESUME MUSIC");
        mp3.play();
        mp3Status = true;
      }
    }
  }
}


void unlockButtons(){
  for(int i=0; i<sizeof(myButtons);i++){
    if(!myButtons[i]){
      sprintf(buttonsBuffer,"tsw bt%d,1",i);
    }
    else{
      sprintf(buttonsBuffer,"tsw bt%d,0",i);
    }
    Serial2.print(buttonsBuffer);
    Serial2.print("\xFF\xFF\xFF");
  }
}

void sendSerial2(){
  for(int i=0; i<sizeof(myButtons);i++){
    if(myButtons[i]){
      sprintf(buttonsBuffer,"bt%d.val=1",i);
    }
    else{
      sprintf(buttonsBuffer,"bt%d.val=0",i);
    }
    Serial2.print(buttonsBuffer);
    Serial2.print("\xFF\xFF\xFF");
  }
}
void decorNextionCallback(void *ptr){
    Serial.println("Decor");
    //currentSpeed = 0;
    stepper->setAcceleration(A1);
    stepper->setSpeedInHz(160);
    updateMyButtons(0);
    sendSerial2();
    unlockButtons();
}
void decorCallback(OSCMessage &msg){
  if(msg.isFloat(0) && (msg.getFloat(0) == 1.)){
      Serial.println("Decor");
      //currentSpeed = 0;
      stepper->setAcceleration(A1);
      stepper->setSpeedInHz(160);
      updateMyButtons(0);
      sendSerial2();
      unlockButtons();
  }
}

void multi5Callback(OSCMessage &msg) { 
  if (msg.isFloat(0)) {
    int value = msg.getFloat(0);
    if(value == 1.){
      Serial.println("0");
      currentSpeed = 0;
      stepper->stopMove();
      updateMyButtons(0);
      sendSerial2();
      unlockButtons();
    }
      
   }
}
void multi4Callback(OSCMessage &msg) { 
  if (msg.isFloat(0)) {
    int value = msg.getFloat(0);
    if (value == 1.) 
    {
      currentSpeed = 1;
      Serial.println("F1");
      stepper->setAcceleration(A1);
      stepper->setSpeedInHz(F1);
      stepper->runBackward();
      updateMyButtons(1);
      sendSerial2();
      unlockButtons();
    }
  }
}
void multi3Callback(OSCMessage &msg) { 
  if (msg.isFloat(0)) {
    int value = msg.getFloat(0);
    if (value == 1.) 
    {
      currentSpeed = 2;
      Serial.println("F2");
      stepper->setAcceleration(A1);
      stepper->setSpeedInHz(F2);
      stepper->runBackward();
      updateMyButtons(2);
      sendSerial2();
      unlockButtons();
    }
  }
}
void multi2Callback(OSCMessage &msg) { 
  if (msg.isFloat(0)) {
    int value = msg.getFloat(0);
    if (value == 1.) 
    {
      currentSpeed = 3;
      Serial.println("F3");
      stepper->setAcceleration(A1);
      stepper->setSpeedInHz(F3);
      stepper->runBackward();
      updateMyButtons(3);
      sendSerial2();
      unlockButtons();
    }
  }
}
void multi1Callback(OSCMessage &msg) { 
  if (msg.isFloat(0)) {
    int value = msg.getFloat(0);
    if (value == 1.) 
    {
      currentSpeed = 4;
      Serial.println("F4");
      stepper->setAcceleration(A1);
      stepper->setSpeedInHz(F4);
      stepper->runBackward();
      updateMyButtons(4);
      sendSerial2();
      unlockButtons();
    }
  }
}
void comp1Callback(void *ptr){
  compositionFileNumber=1;
}
void comp2Callback(void *ptr){
  compositionFileNumber=2;
}
void comp3Callback(void *ptr){
  compositionFileNumber=3;
}
void comp4Callback(void *ptr){
  compositionFileNumber=4;
}
void comp5Callback(void *ptr){
  compositionFileNumber=5;
}
void comp6Callback(void *ptr){
  compositionFileNumber=6;
}
void comp7Callback(void *ptr){
  compositionFileNumber=7;
}
void comp8Callback(void *ptr){
  compositionFileNumber=8;
}
void comp9Callback(void *ptr){
  compositionFileNumber=9;
}
void comp10Callback(void *ptr){
  compositionFileNumber=10;
}
void comp11Callback(void *ptr){
  compositionFileNumber=11;
}
void comp12Callback(void *ptr){
  compositionFileNumber=12;
}
void comp13Callback(void *ptr){
  compositionFileNumber=13;
}
void comp14Callback(void *ptr){
  compositionFileNumber=14;
}
void comp15Callback(void *ptr){
  compositionFileNumber=15;
}

void updateCallback(void *ptr){
  Serial.println("Update Initialized");
  server.stop();
  
  Udp.stop();
  WiFi.disconnect();
  //wm.resetSettings();
  WiFi.mode(WIFI_STA); 
  wm.setConfigPortalBlocking(true);
  wm.resetSettings();
  if(!wm.autoConnect("Firmware Updater")){
    Serial.println("Failed to connect");
  }
  else { 
    Serial.println("Connected.");
  }
  
}
void downloadMusicsCallback(void *ptr){
  Serial.println("entrou no download");
  asyncServer.begin();
}

void stopCompositionCallback(void *ptr){
    Serial.println("entrou no stop");
    stepper->stopMove();
    compositionMode=false;  
    mp3.stop();
    compositionFile.close();
    compositionTimestamp = 0;
    currentCompBright = 0;
    compositionMusicStartTime = 0;
}

void pauseCompositionCallback(void *ptr){
  compositionPaused = true;
  mp3.pause();
}

void playCompositionCallback(void *ptr){
  if(compositionPaused){
    compositionPaused=false;
    compositionMusicStartTime += compositionTimePaused;
    compositionTimePaused = 0;
    mp3.play();
  }else{
    //compositionMode = true;
    pixels.setBrightness(5);
    //pixels.show();
    
    //char program[10];
    
    //String teste = String(program);
    
    compositionFileName = "/" + String(compositionFileNumber) + ".txt";
    compositionFile = SPIFFS.open(compositionFileName, "r");
    if(compositionFile.available()){
      mp3.wakeup();
      mp3.play(compositionFileNumber);
      Serial.println(compositionFileName);
      //mp3.qTTracks();
      //String music_name = mp3.decodeMP3Answer();
      //String file_name = String(program);

      //Serial.println(music_name);
      
      compositionMusicStartTime = millis()/1000;
      compositionTimestamp = compositionFile.readStringUntil(',').toInt();

      compositionMode = true;
      /*int count= f.readStringUntil('\r\n').toInt();
      Serial.println("count: " + count);*/
    }else{
      Serial.println("File doesnt exist");
    }
    
  }
  
}

void applyCompositionChanges(int dimmer, String wave, String color){
    color.trim();
    //Serial.print(color);
    //pixels.fill(pixels.Color(0,0,0,255),0,NUM_LEDS);
    if(color.equals("white")){
      //Serial.println("ENTROU white");
      pixels.fill(pixels.Color(0,0,0,255),0,NUM_LEDS);
    }else if(color.equals("red")){
      //Serial.println("ENTROU red");
      pixels.fill(pixels.Color(255,0,0,0),0,NUM_LEDS);
    }else if(color.equals("blue")){
      pixels.fill(pixels.Color(0,0,255,0),0,NUM_LEDS);
    }else if(color.equals("green")){
      pixels.fill(pixels.Color(0,255,0,0),0,NUM_LEDS);
    }else if(color.equals("purple")){
      pixels.fill(pixels.Color(128,0,128,0),0,NUM_LEDS);
    }else if(color.equals("yellow")){
      pixels.fill(pixels.Color(255,192,0,0),0,NUM_LEDS);
    }else if(color.equals("orange")){
      pixels.fill(pixels.Color(255,165,0,0),0,NUM_LEDS);
    }else if(color.equals("pink")){
      pixels.fill(pixels.Color(255,20,147,0),0,NUM_LEDS);
    }
    stepper->setAcceleration(A3);
    if(wave.equals("delta")){
      stepper->setSpeedInHz(F1);
      stepper->runBackward();
    }else if(wave.equals("theta")){
      stepper->setSpeedInHz(F2);
      stepper->runBackward();
    }else if(wave.equals("alpha")){
      stepper->setSpeedInHz(F3);
      stepper->runBackward();
    }else if(wave.equals("beta")){
      stepper->setSpeedInHz(F4);
      stepper->runBackward();
    }else if(wave.equals("off")){
      stepper->stopMove();
      compositionMode=false;
      mp3.stop();
      compositionFile.close();
      while(currentCompBright!=0){
        currentCompBright=currentCompBright-5;
        Serial.println(currentCompBright);
        pixels.setBrightness(currentCompBright);
        pixels.show();
      }
    }
    
    //pixels.setBrightness(254*(dimmer/10.0));
    pixels.show();
}
void compositionCallbackOSC(OSCMessage &msg, int addressOffset){
  if(msg.getFloat(0)==1.){
    char buffer[3];
    int line, column;
    msg.getAddress(buffer,7);
    sscanf(buffer, "%d/%d", &column, &line);
    compositionFileNumber=0;
    if(column == 1){
      compositionFileNumber = line;
    }else if(column == 2){
      compositionFileNumber = line+5;
    }else if(column == 3){
      compositionFileNumber = line+10;
    }
    
    Serial.println(compositionFileNumber);
  }
  
}

void playCompositionCallbackOSC(OSCMessage &msg){
  if(msg.isFloat(0) && (msg.getFloat(0) == 1.)){
    playCompositionCallback(NULL);
  }
}

void stopCompositionCallbackOSC(OSCMessage &msg){
  if(msg.isFloat(0) && (msg.getFloat(0) == 1.)){
    stopCompositionCallback(NULL);
  }
}

void colorsCallbackOSC(OSCMessage &msg, int addressOffset){
  if(msg.getFloat(0)==1.){
    char buffer[6];
    String color;
    msg.getAddress(buffer,8);
    //Serial.println(buffer);
    //sscanf(buffer, "%s", &color);
    applyCompositionChanges(NULL,"",buffer);
    Serial.println(buffer);
  }
  
}
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open("/" + filename, "w");
    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    request->redirect("/");
  }
}

void setup() {
  
  
  Serial.begin(115200);
  
  //SD CARD CODE
  int count = 0;
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  //String comboBoxValues = "page6.cb0.path=\"";
  //String comboBoxValues = "cb0.path=";
  
  Serial.println("SPIFFS Initialized.");
  
  for (int i = 1; i <= 15; i++)
  {
    String file_name = String("/" + String(i) + ".txt");
    File f = SPIFFS.open(file_name, "r");
    if(!f.available()){
      Serial.println(file_name + " not found");
    }
    f.close();
  }
  
  
  
  //END
  //MP3 CODE
  mp3.begin(9600);      
  delay(500);            
  mp3.sendCommand(CMD_SEL_DEV, 0, 2); 
  delay(500);    
  mp3.stop();
  mp3.reset();
  mp3.setVol(currentVolume);
  mp3.qVol();
  //END
  
  //PIXEL INIT
  pixels.begin();
  whiteMode = pixels.Color(0,0,0,255);
  pixels.fill(pixels.Color(0,0,0,0),0,NUM_LEDS);
  pixels.show();
  //END
  
  //NEXTION CODE
  
  nexInit();
  /*SD.begin(4);
  File f = SD.open("/1.txt", "r");
  String s = f.readString();
  Serial.println("s: " + s);
  f.close();*/

  bt0.attachPop(bt0PopCallback, &bt0);
  bt1.attachPop(bt1PopCallback, &bt1);
  bt2.attachPop(bt2PopCallback, &bt2);
  bt3.attachPop(bt3PopCallback, &bt3);
  bt4.attachPop(bt4PopCallback, &bt4);
  ppButton.attachPop(ppNextionCallback, &ppButton);
  previousButton.attachPop(previousNextionCallback, &previousButton);
  nextButton.attachPop(nextNextionCallback, &nextButton);
  upButton.attachPop(upNextionCallback, &upButton);
  downButton.attachPop(downNextionCallback, &downButton);
  upButtonComp.attachPop(upNextionCallback);
  downButtonComp.attachPop(downNextionCallback);
  
  dimmerSlider.attachPop(dimmerNextionCallback);
  dimmerSlider.attachPush(dimmerNextionCallback);
  //speedSlider.attachPop(speedNextionCallback);
  //speedSlider.attachPush(speedNextionCallback);
  
  whiteButton.attachPop(whiteNextionCallback);
  pinkButton.attachPop(pinkNextionCallback);
  purpleButton.attachPop(purpleNextionCallback);
  redButton.attachPop(redNextionCallback);
  greenButton.attachPop(greenNextionCallback);
  yellowButton.attachPop(yellowNextionCallback);
  orangeButton.attachPop(orangeNextionCallback);
  blueButton.attachPop(blueNextionCallback);

  decorButton.attachPop(decorNextionCallback);

  comp1.attachPop(comp1Callback);
  comp2.attachPop(comp2Callback);
  comp3.attachPop(comp3Callback);
  comp4.attachPop(comp4Callback);
  comp5.attachPop(comp5Callback);
  comp6.attachPop(comp6Callback);
  comp7.attachPop(comp7Callback);
  comp8.attachPop(comp8Callback);
  comp9.attachPop(comp9Callback);
  comp10.attachPop(comp10Callback);
  comp11.attachPop(comp11Callback);
  comp12.attachPop(comp12Callback);
  comp13.attachPop(comp13Callback);
  comp14.attachPop(comp14Callback);
  comp15.attachPop(comp15Callback);


  updateButton.attachPop(updateCallback);
  downloadButton.attachPop(downloadMusicsCallback);
  playCompositionButton.attachPop(playCompositionCallback);
  //pauseCompositionButton.attachPop(pauseCompositionCallback);
  stopCompositionButton.attachPop(stopCompositionCallback);
  //selectedProgram.attachPop(changeSelectedProgramCallback);
  //END

  //STEPPER CODE
  engine.init();
  stepper = engine.stepperConnectToPin(stepPinStepper);
  stepper->setDirectionPin(dirPinStepper);
  stepper->setEnablePin(enablePinStepper);
  stepper->setAutoEnable(true);
  //END
  
  //WIFI CODE
  
  if(WIFI_MODE==AP){
    Serial.println("Scanning Networks...");
    int n = WiFi.scanNetworks();
    Serial.println("Scan Done.");
    int machineCount=0;
    if(n==0){
      Serial.println("No networks found.");
    }
    else{
      Serial.print(n);
      Serial.println(" networks were found.");
      for(int i=0; i<n;i++){
        String newSSID = WiFi.SSID(i);
        newSSID.remove(12);
        if(newSSID.equals("DreamMachine")){
          newSSID = WiFi.SSID(i);
          if(newSSID.endsWith((String)machineCount)){
            machineCount++;
            i=0;
          }
        }
        sprintf(serverSSID, "DreamMachine%d", machineCount);   
      }
    }
    Serial.println("Setting AP (Access Point)…");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(serverSSID);
    Serial.println("Setup Successful.");
    Serial.print("Initialized as ");
    Serial.println(serverSSID);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }else{
    WiFi.mode(WIFI_STA); 
    wm.setConfigPortalBlocking(true);
    if(!wm.autoConnect("DreamMachine - Network Selection")){
      Serial.println("Failed to connect");
    }
    else { 
      Serial.println("Connected.");
    }
  }
  

  
  
  







  Udp.begin(inPort);
  //server.begin();
  //END

  /*pixels.fill(pixels.Color(0,0,0,255),0,NUM_LEDS);
  for(int i = 1; i<206; i=i+5){
      pixels.setBrightness(i);
      pixels.show();
      delay(1);
  }
  delay(500);
  pixels.fill(pixels.Color(0,0,0,0),0,NUM_LEDS);
  pixels.show();
  delay(500);
  pixels.fill(pixels.Color(0,0,0,255),0,NUM_LEDS);
  pixels.show();
  delay(500);
  pixels.fill(pixels.Color(0,0,0,0),0,NUM_LEDS);
  pixels.show();
  delay(500);
  pixels.fill(pixels.Color(0,0,0,255),0,NUM_LEDS);
  pixels.show();
  delay(500);
  pixels.fill(pixels.Color(0,0,0,0),0,NUM_LEDS);
  pixels.show();
  delay(500);
  pixels.fill(pixels.Color(0,0,0,255),0,NUM_LEDS);
  pixels.show();*/
  //delay(1000);
  /*Serial.println("teste");
  Serial.println(comboBoxValues);
  Serial.println(comboBoxValues);
  Serial.println(comboBoxValues);*/
  
  //Serial2.print("page6.cb0.path=\"1\r\"");
  
  /*Serial2.print(comboBoxValues);
  Serial2.print("\xFF\xFF\xFF");*/

  Serial2.print("page 1");
  Serial2.print("\xFF\xFF\xFF");

  
  
  asyncServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  asyncServer.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css","text/css");
  });
  asyncServer.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "Files Sent");
  }, handleUpload);
  //server.onFileUpload(handleUpload);
  
  
  

  //testFunction();
}

void loop(){ 
  WiFiClient client = server.available();
  int packetSize = Udp.parsePacket();

  /*if(Serial.readString()=="download"){
    downloadMusicsCallback();
  }*/

  if(packetSize) {
    Serial.println("Received Packet");
    byte buf[packetSize];
    int bytesRead = Udp.read(buf, packetSize);
    OSCMessage msgIn;
    msgIn.fill(buf, packetSize);
    msgIn.dispatch(MULTI_ROUTE1, multi1Callback);
    msgIn.dispatch(MULTI_ROUTE2, multi2Callback);
    msgIn.dispatch(MULTI_ROUTE3, multi3Callback);
    msgIn.dispatch(MULTI_ROUTE4, multi4Callback);
    msgIn.dispatch(MULTI_ROUTE5, multi5Callback);
    //msgIn.dispatch(SLIDER, sliderCallback);
    
    msgIn.route("/comps", compositionCallbackOSC);
    msgIn.route("/colors", colorsCallbackOSC);
    msgIn.dispatch("/playcomp", playCompositionCallbackOSC);
    msgIn.dispatch("/stopcomp", stopCompositionCallbackOSC);
    msgIn.dispatch("/decor", decorCallback);

    msgIn.dispatch(PP_ROUTE, ppCallback);
    msgIn.dispatch(PREVIOUS_ROUTE,previousCallback);
    msgIn.dispatch(NEXT_ROUTE,nextCallback);
    msgIn.dispatch(DOWN_ROUTE, downCallback);
    msgIn.dispatch(UP_ROUTE, upCallback);
    
    msgIn.dispatch(SYNC_ROUTE, syncCallback);
    msgIn.dispatch(DIMMER_ROUTE, dimmerCallback);
    msgIn.dispatch(WHITE_ROUTE, whiteCallback);
  }

  unsigned long currentMillis = millis();
  if((currentMillis - previousMillis >= interval) && receivingPackets){
    Serial.println(buffer);
    Serial2.print(buffer);
    Serial2.print("\xFF\xFF\xFF");
    previousMillis = currentMillis;
    receivingPackets = false;
  }

  nexLoop(nex_listen_list);
  //wm.process();
  if(compositionMode){

      float currentTime = millis()/1000;
      //Serial.println("currentCompBright: " + String(currentCompBright) + " compositionDimmer: " + String(compositionDimmer));
      if((currentCompBright>compositionDimmer) && !compositionPaused){
        currentCompBright=currentCompBright-5;
        Serial.println(currentCompBright);
        pixels.setBrightness(currentCompBright);
        pixels.show();
      }
      if((currentCompBright<compositionDimmer) && !compositionPaused){
        Serial.println(currentCompBright);
        currentCompBright=currentCompBright+5;
        pixels.setBrightness(currentCompBright);
        pixels.show();
      }
      if(scientificMode){
        if(sciWave2 == -1){
          sciWave1=0;
          sciWave2=0;
          sciTimestamp1=0;
          sciTimestamp2=0;
          scientificMode=false;
          compositionMode=false;
          stepper->stopMove();
          mp3.stop();
          compositionFile.close();
          Serial2.print("tsw b5,1");
          Serial2.print("\xFF\xFF\xFF");
          Serial2.print("tsw b6,1");
          Serial2.print("\xFF\xFF\xFF");
        }else{
          float speedDifferece = sciWave2 - sciWave1;
          float timeDifference = sciTimestamp2 - sciTimestamp1;
          float sciCurrentTime = millis()/1000 - compositionMusicStartTime - sciTimestamp1;
          float percentage = sciCurrentTime/timeDifference;
          Serial.println("percentage: " + String(percentage));
          float currentSpeed = sciWave1 + (speedDifferece*percentage);
          int speedInHz = strobeToMotorHz(currentSpeed)*1600;
          Serial.println(speedInHz);
          stepper->setSpeedInHz(speedInHz);
          stepper->runBackward();
        }
      }
      
      if((compositionTimestamp<(currentTime-compositionMusicStartTime)) && !scientificMode){
        Serial.println("currentTime: " + String(currentTime) + "   compositionMusicStartTime: " + compositionMusicStartTime);
        //lastCompositionDimmer=compositionDimmer;
        String wave = compositionFile.readStringUntil(',');
        compositionDimmer = compositionFile.readStringUntil(',').toFloat();
        compositionDimmer = 255*compositionDimmer/10.0;
        String color = compositionFile.readStringUntil('\n');
        

        Serial.println("timestamp: " + String(compositionTimestamp));
        Serial.println("wave: " + wave);
        Serial.println("dimmer: " + String(compositionDimmer));
        Serial.println("color: " + color);
        applyCompositionChanges(compositionDimmer,wave,color);
        //lastCompositionTimestamp = compositionTimestamp;
        compositionTimestamp = compositionFile.readStringUntil(',').toInt()-1;
        
        //i++;
      }
      //Serial.println("sciTimestamp2: " + String(sciTimestamp2));
      //Serial.println("elapsed: " + String(currentTime-compositionMusicStartTime));
      if((sciTimestamp2<(currentTime-compositionMusicStartTime)) && scientificMode){
        sciWave1 = sciWave2;
        sciTimestamp1 = sciTimestamp2;
        compositionDimmer = compositionFile.readStringUntil(',').toFloat();
        compositionDimmer = 255*compositionDimmer/10.0;
        String color = compositionFile.readStringUntil('\n');
        sciTimestamp2 = compositionFile.readStringUntil(',').toInt();
        sciWave2 = compositionFile.readStringUntil(',').toFloat();

        Serial.println("timestamp1: " + String(sciTimestamp1));
        Serial.println("timestamp2: " + String(sciTimestamp2));
        Serial.println("wave1: " + String(sciWave1));
        Serial.println("wave2: " + String(sciWave2));
        Serial.println("dimmer: " + String(compositionDimmer));
        Serial.println("color: " + color);
      }
    //}
    
    //compositionFile.close(); //MUDAR PARA QUANDO STOP É CLICADO
    //ADICIONAR TEMPO DE FIM DA MUSICA
  }
}