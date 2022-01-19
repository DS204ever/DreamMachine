#include <WiFiUdp.h>
#include <WiFiManager.h> 
#include <OSCMessage.h> 
#include <analogWrite.h> 
#include "FastAccelStepper.h"
#include <Nextion.h>  
#include <SerialMP3Player.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <SD.h>
#include <SPI.h>

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
int currentVolume = 60;
#define MAX_VOLUME 100;
SerialMP3Player mp3(mp3RX,mp3TX);

#define dirPinStepper 18
#define enablePinStepper 21
#define stepPinStepper 19

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;


//AccelStepper stepper(1,step,direction); 
//TM1638lite tm(18,17,16);
/*EasyButton button1(33,35,true,false);
EasyButton button2(12,35,true,false); 
EasyButton button3(32,35,true,false);
EasyButton button4(14,35,true,false);*/



NexButton ppButton = NexButton(3,1,"b0");
NexButton nextButton = NexButton(3,3,"b1");
NexButton previousButton = NexButton(3,2,"b2");
NexButton upButton = NexButton(3,5,"b3");
NexButton downButton = NexButton(3,4,"b4");


NexDSButton bt0 = NexDSButton(2,6,"bt0");
NexDSButton bt1 = NexDSButton(2,7,"bt1");
NexDSButton bt2 = NexDSButton(2,8,"bt2");
NexDSButton bt3 = NexDSButton(2,9,"bt3");
NexDSButton bt4 = NexDSButton(2,10,"bt4");

NexSlider speedSlider = NexSlider(2,1,"h0");
NexSlider colorSlider = NexSlider(2,3,"h1");
NexSlider dimmerSlider = NexSlider(2,4,"h2");

NexButton coldWhite = NexButton(5,1,"b0");
NexButton pureWhite = NexButton(5,2,"b1");
NexButton warmWhite = NexButton(5,3,"b2");
NexButton whiteButton = NexButton(5,2,"b2");
NexButton updateButton = NexButton(5,5,"b3");

NexButton downloadButton = NexButton(4,2,"b0");
 
NexTouch *nex_listen_list[] = {&downloadButton, &updateButton, &whiteButton, &coldWhite, &pureWhite, &warmWhite, &speedSlider, &colorSlider, &dimmerSlider, &ppButton, &nextButton,&previousButton, &upButton, &downButton, &bt0,&bt1,&bt2,&bt3,&bt4,NULL};


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
boolean WIFI_MODE = !AP;
//WiFiUDP Udp;

void whiteNextionCallback(void *ptr){
  Serial.println("White Mode Active");
  whiteStatus = true;
  pixels.fill(whiteMode,0,NUM_LEDS);
  pixels.show();
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
void coldWhiteCallback(void *ptr){
  Serial.println("Cold White Selected");
  whiteStatus = true;
  whiteMode = pixels.Color(255,255,255,0);
  pixels.fill(whiteMode,0,NUM_LEDS);
  pixels.show();
}
void pureWhiteCallback(void *ptr){
  Serial.println("Pure White Selected");
  whiteStatus = true;
  whiteMode = pixels.Color(255,255,255,255);
  pixels.fill(whiteMode,0,NUM_LEDS);
  pixels.show();
}
void warmWhiteCallback(void *ptr){
  Serial.println("Warm White Selected");
  whiteStatus = true;
  whiteMode = pixels.Color(0,0,0,255);
  pixels.fill(whiteMode,0,NUM_LEDS);
  pixels.show();
}

void colorNextionCallback(void *ptr){
  Serial.println("Color was pressed");
  colorReceived=true;
  uint32_t number = 0;
  colorSlider.getValue(&number);
  newHue = 65535*number/100;
  OSCMessage msgOUT("/color");
  float newNumber = newHue/65535.0;
  msgOUT.add(newNumber);
  for(int count = 0; count < 10; count++){
    Udp.beginPacket(outIP, inPort);
    msgOUT.send(Udp);
    Udp.endPacket();
    count++;
  }
  msgOUT.empty();
}

void colorCallback(OSCMessage &msg){
   if (msg.isFloat(0)){
    float value = msg.getFloat(0);
    newHue = value*65535;
    colorReceived = true;
    int newValue = value*100;
    sprintf(buffer, "h1.val=%d", newValue);
    unsigned long currentMillis = millis();
    previousMillis = currentMillis;
    receivingPackets = true;
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
  if(currentVolume!=100){
    currentVolume+=20;
    mp3.setVol(currentVolume);
    mp3.qVol();
  }
}
void downNextionCallback(void *ptr){
  Serial.println("DOWN VOLUME");
  if(currentVolume!=0){
    currentVolume-=20;
    mp3.setVol(currentVolume);
    mp3.qVol();
  }
}
void upCallback(OSCMessage &msg){
  if (msg.isFloat(0)) {
    int value = msg.getFloat(0);
    if(value == 1.){
      Serial.println("UP VOLUME");
      if(currentVolume!=100){
        currentVolume+=20;
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
        currentVolume-=20;
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
void speedNextionCallback(void *ptr){
  Serial.println("Speed Slider was pressed.");
  uint32_t number = 0;
  speedSlider.getValue(&number);
  int sliderSpeed = number*F4/100;
  if(number==0){
    stepper->stopMove();
    updateMyButtons(0);
    sendSerial2();
    unlockButtons();
  }
  else if((sliderSpeed > (F1-50)) && (sliderSpeed < (F1+50))){
    stepper->setAcceleration(A1);
    stepper->setSpeedInHz(sliderSpeed);
    stepper->runBackward();
    updateMyButtons(1);
    sendSerial2();
    unlockButtons();
  }
  else if((sliderSpeed > (F2-50)) && (sliderSpeed < (F2+50))){
    stepper->setAcceleration(A2);
    stepper->setSpeedInHz(sliderSpeed);
    stepper->runBackward();
    updateMyButtons(2);
    sendSerial2();
    unlockButtons();
  }
  else if((sliderSpeed > (F3-50)) && (sliderSpeed < (F3+50))){
    stepper->setAcceleration(A1);
    stepper->setSpeedInHz(sliderSpeed);
    stepper->runBackward();
    updateMyButtons(3);
    sendSerial2();
    unlockButtons();
  }
  else if((sliderSpeed > (F4-100)) && (sliderSpeed < (F4))){
    stepper->setAcceleration(A1);
    stepper->setSpeedInHz(sliderSpeed);
    stepper->runBackward();
    updateMyButtons(4);
    sendSerial2();
    unlockButtons();
  }
}

void sliderCallback(OSCMessage &msg){
  if (msg.isFloat(0)) {
    float value = msg.getFloat(0);
    int sliderSpeed = value*F4;
    if(value==0){
      Serial.println("Entrou");
      stepper->stopMove();
      updateMyButtons(0);
      sendSerial2();
      unlockButtons();
    }
    else if((sliderSpeed > (F1-50)) && (sliderSpeed < (F1+50))){
    stepper->setAcceleration(A1);
    stepper->setSpeedInHz(sliderSpeed);
    stepper->runBackward();
    updateMyButtons(1);
    sendSerial2();
    unlockButtons();
    }
    else if((sliderSpeed > (F2-50)) && (sliderSpeed < (F2+50))){
      stepper->setAcceleration(A2);
      stepper->setSpeedInHz(sliderSpeed);
      stepper->runBackward();
      updateMyButtons(2);
      sendSerial2();
      unlockButtons();
    }
    else if((sliderSpeed > (F3-50)) && (sliderSpeed < (F3+50))){
      stepper->setAcceleration(A1);
      stepper->setSpeedInHz(sliderSpeed);
      stepper->runBackward();
      updateMyButtons(3);
      sendSerial2();
      unlockButtons();
    }
    else if((sliderSpeed > (F4-100)) && (sliderSpeed < (F4))){
      stepper->setAcceleration(A1);
      stepper->setSpeedInHz(sliderSpeed);
      stepper->runBackward();
      updateMyButtons(4);
      sendSerial2();
      unlockButtons();
    }
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
void updateCallback(void *ptr){
  Serial.println("Update Initialized");
  server.stop();
  Udp.stop();
  WiFi.disconnect();
  //wm.resetSettings();
  WiFi.mode(WIFI_STA); 
  wm.setConfigPortalBlocking(true);
  if(!wm.autoConnect("DreamMachineUpdater")) {
      Serial.println("Failed to connect");
  } 
  else { 
      Serial.println("Connected.");
  }
  if ((WiFi.status() == WL_CONNECTED)) {
  WiFiClient client;
  t_httpUpdate_return ret = httpUpdate.update(client, "http://sousadreammachine.duckdns.org:8123/firmware.bin");
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
    }
  }
}
void downloadMusicsCallback(void *ptr){
  Serial.println("Entered");
  HTTPClient http;
  if(WIFI_MODE==AP){
    Serial.println("Changing WIFI MODE");
    server.stop();
    Udp.stop();
    WiFi.disconnect();
    
    WiFi.mode(WIFI_STA); 
    wm.setConfigPortalBlocking(true);
    if(!wm.autoConnect("DreamMachineDownloader")) {
        Serial.println("Failed to connect");
    } 
    else { 
        Serial.println("Connected.");
    }
  }else{
    Serial.println("WIFI MODE CORRECT");
  }
  if(WiFi.status() == WL_CONNECTED){
    for (int i = 1; i <= 20; i++)
    {
      String file_name = String("/" + String(i) + ".txt");
      Serial.println(file_name);
      http.begin("http://sousadreammachine.duckdns.org:8123"+file_name);
      if(http.GET()==HTTP_CODE_OK){
        SD.remove(file_name);
        File f = SD.open(file_name,"w");
        http.writeToStream(&f);
        f.close();
      }
      else{
        Serial.println("File " + file_name + " not found.");
      }
      http.end();
    }
  }
}

void playProgramCallback(void *ptr){
  //File f = SD.open(,"r");

}


void setup() {
  
  Serial.begin(115200);
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
  
  dimmerSlider.attachPop(dimmerNextionCallback);
  dimmerSlider.attachPush(dimmerNextionCallback);
  colorSlider.attachPop(colorNextionCallback);
  colorSlider.attachPush(colorNextionCallback);
  speedSlider.attachPop(speedNextionCallback);
  speedSlider.attachPush(speedNextionCallback);
  
  coldWhite.attachPop(coldWhiteCallback);
  pureWhite.attachPop(pureWhiteCallback);
  warmWhite.attachPop(warmWhiteCallback);
  whiteButton.attachPop(whiteNextionCallback);

  updateButton.attachPop(updateCallback);
  downloadButton.attachPop(downloadMusicsCallback);
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
  

  //SD CARD CODE
  int count = 0;
  while(!SD.begin(4)||count!=10){
    count++;
    //Serial.println("Error initializing SD CARD.");
    delay(250);
  }
  String comboBoxValues = "cb0.path=";
  if(SD.begin(4)){
    Serial.println("SD CARD Initialized.");
    File f;
    for (int i = 1; i <= 20; i++)
    {
      String file_name = String("/" + String(i) + ".txt");
      if(f = SD.open(file_name, "r")){
        comboBoxValues.concat(String(i) + "\r");
        f.close();
      }
      
    }
    
  }else{
    Serial.println("Card couldn't be initialized");
  }
  
  /*Serial.println("\"" + comboBoxValues + "\"");
  Serial2.print(comboBoxValues);
  Serial2.print("\xFF\xFF\xFF");*/
  //END

  







  Udp.begin(inPort);
  server.begin();
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
  Serial.println("teste");
  
  Serial2.print("page 1");
  Serial2.print("\xFF\xFF\xFF");

  
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
    msgIn.dispatch(SLIDER, sliderCallback);
    
    msgIn.dispatch(PP_ROUTE, ppCallback);
    msgIn.dispatch(PREVIOUS_ROUTE,previousCallback);
    msgIn.dispatch(NEXT_ROUTE,nextCallback);
    msgIn.dispatch(DOWN_ROUTE, downCallback);
    msgIn.dispatch(UP_ROUTE, upCallback);
    
    msgIn.dispatch(SYNC_ROUTE, syncCallback);
    msgIn.dispatch(DIMMER_ROUTE, dimmerCallback);
    msgIn.dispatch(COLOR_ROUTE, colorCallback);
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

  
  currentMillis = millis();
  if(colorReceived && currentHue < newHue){
    //trueWhite = false;
    //normalWhite = false;
    whiteStatus = false;
    for(; (currentHue<newHue && (currentMillis - previousMillisColor >= 10)); (currentHue+=1500)){
      pixels.fill(pixels.ColorHSV(currentHue,255,255),0,NUM_LEDS);
      pixels.show();
      Serial.println(currentHue);
      currentMillis = millis();
      previousMillisColor = currentMillis;
      
    }
    if(currentHue>=newHue){
      Serial.println("SAIO");
      if(currentHue>65535){
        currentHue = 65535;
        pixels.fill(pixels.ColorHSV(currentHue,255,255),0,NUM_LEDS);
        pixels.show();
      }
      colorReceived = false; 
    }
  }
  else if(colorReceived && currentHue > newHue){
    //trueWhite = false;
    //normalWhite = false;
    whiteStatus = false;
    for(; (currentHue > newHue && (currentMillis - previousMillisColor >= 10)); (currentHue-=1500)){
      pixels.fill(pixels.ColorHSV(currentHue,255,255),0,NUM_LEDS);
      currentMillis = millis();
      previousMillisColor = currentMillis;
      pixels.show();
      Serial.println(currentHue);
    }
    if(currentHue<=newHue){
      Serial.println("SAIO");
      if(currentHue<0){
        currentHue=0;
        pixels.fill(pixels.ColorHSV(currentHue,255,255),0,NUM_LEDS);
        pixels.show();
      }
      colorReceived = false; 
    }
  }
  nexLoop(nex_listen_list);
  //wm.process();
}