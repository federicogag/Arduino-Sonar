#include <Servo.h>.
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"
#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif
#define FACTORYRESET_ENABLE         1
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"


// Defines Trig and Echo pins of the Ultrasonic Sensor
const int trigPin = 10;
const int echoPin = 9;
// Variables for the duration and the distance
long duration;
int distance;
Servo myServo; // Creates a servo object for controlling the servo motor
int d = 50;
String str;
int degree;
//Variables for custom commands
String newCommandList[3]= {"null", "null", "null"};
String newCommand0[10] = {"null","null","null","null","null","null","null","null","null","null"};
String newCommand1[10] = {"null","null","null","null","null","null","null","null","null","null"};
String newCommand2[10] = {"null","null","null","null","null","null","null","null","null","null"};

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

void setup(void) {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  while(!Serial);
  delay(500);
  Serial.begin(115200);
  myServo.attach(1); // Defines on which pin is the servo motor attached

   if (!ble.begin(VERBOSE_MODE)){
    Serial.println(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    }
  Serial.println( F("OK!") );
 
 if ( FACTORYRESET_ENABLE )
  {
    // Perform a factory reset to make sure everything is in a known state
    //Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      //Serial.println(F("Couldn't factory reset"));
    }
  }

   ble.echo(false);
   Serial.println("Requesting Bluefruit info:");
   ble.info();
   Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
   Serial.println();
   ble.verbose(false);
   
  //Wait for connection
  while (! ble.isConnected()) {
      delay(500);
  }
  Serial.println("Device Connected!");

  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
     //Change Mode LED Activity
    Serial.println(F("******************************"));
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
    Serial.println(F("******************************"));
  }

  

  
}

void loop(void) {
  
  char inputs[BUFSIZE+1];
   if ( getUserInput(inputs, BUFSIZE) )
  {
    //Send characters to Bluefruit
    //Serial.print("[Send] ");
    //Serial.println(inputs);

    ble.print("AT+BLEUARTTX=");
    ble.println(inputs);

    // check response stastus
    if (! ble.waitForOK() ) {
      //Serial.println(F("Failed to send?"));
    }
  }

  ble.println("AT+BLEUARTRX");
  ble.readline();
  if (strcmp(ble.buffer, "OK") == 0) {
    // no data
    return;
  }
  // Some data was found, its in the buffer
  
  if(strcmp(ble.buffer, "scan") == 0){
      for(int i=15;i<=165;i++){
        myServo.write(i);
        delay(d);
        distance = calculateDistance();
        Serial.print("scan;");
        Serial.print(distance);
        Serial.print(",");
        Serial.println(i);
        ble.print("AT+BLEUARTTX=");
        ble.print(distance);
        ble.println(" cm");
        delay(80);
      }
       for(int i=165;i>15;i--){  
        myServo.write(i);
        delay(d);
        distance = calculateDistance();
        Serial.print("scan;");
        Serial.print(distance);
        Serial.print(",");
        Serial.println(i);
        ble.print("AT+BLEUARTTX=");
        ble.print(distance);
        ble.println(" cm");
        delay(80);
    }
  }
  else if(strcmp(ble.buffer, "setdelay") == 0){
    Serial.println("Command setdelay");
    delay(50);
    ble.println("AT+BLEUARTTX=Insert delay (ms)");
    delay(100);
    //ciclo che aspetta inserimento del numero dall'utente
    do{
     ble.println("AT+BLEUARTRX");
     delay(20);
     ble.readline();
    }while(strcmp(ble.buffer, "OK") == 0); 
     str = ble.buffer;
     d = str.toInt();
     Serial.print("set Delay to :");
     Serial.println(d);
     delay(50);
      ble.print("AT+BLEUARTTX=");
      ble.print("Delay changed to ");
      ble.println(d);
    }
    else if(strcmp(ble.buffer, "getdist") == 0){
     Serial.println("Command getdist");
     delay(50);
     ble.println("AT+BLEUARTTX=Insert degree ");
     do{
     ble.println("AT+BLEUARTRX");
     delay(20);
     ble.readline();
    }while(strcmp(ble.buffer, "OK") == 0); 
     str = ble.buffer;
     degree = str.toInt();     
     ble.print("AT+BLEUARTTX=");
     ble.print("Degree changed to");
     ble.println(degree);
     myServo.write(degree);
     distance = calculateDistance();
     SerialReset();
     Serial.print("getdist;");
     Serial.print(distance);
     Serial.print(",");
     Serial.println(degree);
     ble.print("AT+BLEUARTTX=");
     ble.print("Distanza (cm)");
     ble.println(distance);
   }
   else if(strcmp(ble.buffer, "show") == 0){
    degree = myServo.read();
    Serial.print("Showing current degree :");
    Serial.println(degree);
    ble.print("AT+BLEUARTTX=");
    ble.print("Current degree :");
    ble.println(degree);
   }
   else if(strcmp(ble.buffer, "right") == 0){
    degree = myServo.read();
    degree = degree - 10;
    myServo.write(degree);
    delay(200);
    distance = calculateDistance();
    
    Serial.print("right");
    Serial.print(";");
    Serial.print(distance);
    Serial.print(",");
    Serial.println(degree);
    
    ble.print("AT+BLEUARTTX=");
    ble.print("Current degree ");
    ble.println(degree);
    delay(50);
    ble.print("AT+BLEUARTTX=");
    ble.print("Distanza (cm)");
    ble.println(distance);
   }
   else if(strcmp(ble.buffer, "left") == 0){
   degree = myServo.read();
    degree = degree + 10;
    myServo.write(degree);
    delay(200);
    distance = calculateDistance();
    
    Serial.print("left");
    Serial.print(";");
    Serial.print(distance);
    Serial.print(",");
    Serial.println(degree);
    
    ble.print("AT+BLEUARTTX=");
    ble.print("Current degree ");
    ble.println(degree);
    delay(50);
    ble.print("AT+BLEUARTTX=");
    ble.print("Distanza (cm)");
    ble.println(distance);
   }
   else if(strcmp(ble.buffer, "reset") == 0){
    myServo.write(0);
    delay(200);
    distance = calculateDistance();
    Serial.print("reset;");
    Serial.print(distance);
    Serial.print(",");
    Serial.println("0");
   }
   else if(strcmp(ble.buffer, "list") == 0){
    Serial.println("Show list to device");
    ble.println("AT+BLEUARTTX=Command list");
    delay(120);
    ble.println("AT+BLEUARTTX=scan");
    delay(120);
    ble.println("AT+BLEUARTTX=setdelay");
    delay(120);
    ble.println("AT+BLEUARTTX=getdist");
    delay(120);
    ble.println("AT+BLEUARTTX=show");
    delay(120);
    ble.println("AT+BLEUARTTX=right");
    delay(120);
    ble.println("AT+BLEUARTTX=left");
    delay(120);
    ble.println("AT+BLEUARTTX=reset");
    delay(120);
    ble.println("AT+BLEUARTTX=newcommand");
    
   }
   else if(strcmp(ble.buffer, "newcommand")==0){
    if(isFull(newCommandList)){
      Serial.println("Error, newCommandList is full");
      ble.println("AT+BLEUARTTX=Error, no space for new commands");
    }else{
      ble.println("AT+BLEUARTTX=Insert new command name");
      delay(100);
     do{
     ble.println("AT+BLEUARTRX");
     delay(20);
     ble.readline();
    }while(strcmp(ble.buffer, "OK") == 0); 
    str = ble.buffer;
    int n = getSpaceIndex(newCommandList);
    newCommandList[n] = str;
    Serial.println("Insert new command in newCommandList");

    if(n == 0){
      //RIEMPIO NEWCOMMAND0
      for(int i=0; i<9; i++){
        ble.println("AT+BLEUARTTX=Insert basic command name, type \"fin\" to finish");
        delay(100);
        do{
          ble.println("AT+BLEUARTRX");
          delay(20);
          ble.readline();
          }while(strcmp(ble.buffer, "OK") == 0); 
        str = ble.buffer;
        if(str == "fin") break;
        newCommand0[i] = str;
        Serial.print("insert command \"");
        Serial.print(str);
        Serial.println("\" in newCommand0");
        delay(50);
        ble.println("AT+BLEUARTTX=command inserted");
      }
    }else if(n == 1){
      //RIEMPIO NEWCOMMAND1
      for(int i=0; i<9; i++){
        ble.println("AT+BLEUARTTX=Insert basic command name, fin to finish");
        do{
          ble.println("AT+BLEUARTRX");
          delay(20);
          ble.readline();
          }while(strcmp(ble.buffer, "OK") == 0); 
        str = ble.buffer;
        if(str == "fin") break;
        newCommand1[i] = str;
        Serial.print("insert command \"");
        Serial.print(str);
        Serial.println("\" in newCommand1");
        delay(50);
        ble.println("AT+BLEUARTTX=command inserted");
      }
    }
    else{
      //RIEMPIO NEWCOMMAND2
      for(int i=0; i<9; i++){
        ble.println("AT+BLEUARTTX=Insert basic command name, fin to finish");
        do{
          ble.println("AT+BLEUARTRX");
          delay(20);
          ble.readline();
          }while(strcmp(ble.buffer, "OK") == 0); 
        str = ble.buffer;
        if(str == "fin") break;
        newCommand2[i] = str;
        Serial.print("insert command \"");
        Serial.print(str);
        Serial.println("\" in newCommand2");
        delay(50);
        ble.println("AT+BLEUARTTX=command inserted");
             }
       }
    }
}
    else if(String(ble.buffer) == newCommandList[0]){
    for(int i=0; i< 9; i++){
      doSomething(newCommand0[i]);
    }
   }else if(String(ble.buffer) == newCommandList[1]){
    for(int i=0; i< 9; i++){
      doSomething(newCommand1[i]);
    } 
   }else if(String(ble.buffer) == newCommandList[2]){
    for(int i=0; i< 9; i++){
      doSomething(newCommand2[i]);
    }
   }
  else{
    ble.println("AT+BLEUARTTX=Unknown Command");
    Serial.println("Bad Command");
   }
   
   }




bool getUserInput(char buffer[], uint8_t maxSize)
{
  // timeout in 100 milliseconds
  TimeoutTimer timeout(100);

  memset(buffer, 0, maxSize);
  while( (!Serial.available()) && !timeout.expired() ) { delay(1); }

  if ( timeout.expired() ) return false;

  delay(2);
  uint8_t count=0;
  do
  {
    count += Serial.readBytes(buffer+count, maxSize);
    delay(2);
  } while( (count < maxSize) && (Serial.available()) );
  return true;
}

boolean isFull(String arr[]){
 for(int i=0; i<=2; i++){
  if(arr[i] == "null") return false;
 }
 return true;
}

int getSpaceIndex(String arr[]){
  for(int i=0; i<=2; i++){
    if(arr[i] == "null") return i;
  }
  return -1;
}

void doSomething(String command){
  if(command == "scan"){
    for(int i=15;i<=165;i++){
        myServo.write(i);
        delay(d);
        distance = calculateDistance();
        Serial.print("scan;");
        Serial.print(distance);
        Serial.print(",");
        Serial.println(i);
        ble.print("AT+BLEUARTTX=");
        ble.print(distance);
        ble.println(" cm");
        delay(80);
      }
       for(int i=165;i>15;i--){  
        myServo.write(i);
        delay(d);
        distance = calculateDistance();
        Serial.print("scan;");
        Serial.print(distance);
        Serial.print(",");
        Serial.println(i);
        ble.print("AT+BLEUARTTX=");
        ble.print(distance);
        ble.println(" cm");
        delay(80);
    }
  }
  else if(command == "setdelay"){
    Serial.println("Command setdelay");
    delay(50);
    ble.println("AT+BLEUARTTX=Insert delay (ms)");
    delay(100);
    //ciclo che aspetta inserimento del numero dall'utente
    do{
     ble.println("AT+BLEUARTRX");
     delay(20);
     ble.readline();
    }while(strcmp(ble.buffer, "OK") == 0); 
     str = ble.buffer;
     d = str.toInt();
     Serial.print("set Delay to :");
     Serial.println(d);
     delay(50);
      ble.print("AT+BLEUARTTX=");
      ble.print("Delay changed to ");
      ble.println(d);
      delay(100);
    }
    else if(command == "getdist"){
     Serial.println("Command getdist");
     delay(50);
     ble.println("AT+BLEUARTTX=Insert degree ");
     do{
     ble.println("AT+BLEUARTRX");
     delay(20);
     ble.readline();
    }while(strcmp(ble.buffer, "OK") == 0); 
     str = ble.buffer;
     degree = str.toInt();     
     ble.print("AT+BLEUARTTX=");
     ble.print("Degree changed to");
     ble.println(degree);
     myServo.write(degree);
     distance = calculateDistance();
     SerialReset();
     Serial.print("getdist;");
     Serial.print(distance);
     Serial.print(",");
     Serial.println(degree);
     ble.print("AT+BLEUARTTX=");
     ble.print("Distanza (cm)");
     ble.println(distance);
   }
   else if(command == "show"){
    degree = myServo.read();
    Serial.print("Showing current degree :");
    Serial.println(degree);
    ble.print("AT+BLEUARTTX=");
    ble.print("Current degree :");
    ble.println(degree);
    delay(100);
   }
   else if(command == "right"){
    degree = myServo.read();
    degree = degree - 10;
    myServo.write(degree);
    delay(200);
    distance = calculateDistance();
    
    Serial.print("right");
    Serial.print(";");
    Serial.print(distance);
    Serial.print(",");
    Serial.println(degree);
    
    ble.print("AT+BLEUARTTX=");
    ble.print("Current degree ");
    ble.println(degree);
    delay(50);
    ble.print("AT+BLEUARTTX=");
    ble.print("Distanza (cm)");
    ble.println(distance);
   }
   else if(command == "left"){
    degree = myServo.read();
    degree = degree + 10;
    myServo.write(degree);
    delay(200);
    distance = calculateDistance();
    
    Serial.print("left");
    Serial.print(";");
    Serial.print(distance);
    Serial.print(",");
    Serial.println(degree);
    
    ble.print("AT+BLEUARTTX=");
    ble.print("Current degree ");
    ble.println(degree);
    delay(50);
    ble.print("AT+BLEUARTTX=");
    ble.print("Distanza (cm)");
    ble.println(distance);
   }
   else if(command == "reset"){
    myServo.write(0);
    delay(200);
    distance = calculateDistance();
    Serial.print("reset;");
    Serial.print(distance);
    Serial.print(",");
    Serial.println("0");
   }
   else if(command == "list") {
    Serial.println("Show list to device");
    ble.println("AT+BLEUARTTX=Command list");
    delay(120);
    ble.println("AT+BLEUARTTX=scan");
    delay(120);
    ble.println("AT+BLEUARTTX=setdelay");
    delay(120);
    ble.println("AT+BLEUARTTX=getdist");
    delay(120);
    ble.println("AT+BLEUARTTX=show");
    delay(120);
    ble.println("AT+BLEUARTTX=right");
    delay(120);
    ble.println("AT+BLEUARTTX=left");
    delay(120);
    ble.println("AT+BLEUARTTX=reset");
    delay(100);
}else if(command == "null"){
  delay(100);
}else{
  ble.println("AT+BLEUARTTX=Unknown Command");
  Serial.println("Bad Command");
  delay(100);
}
  
}
// Function for calculating the distance measured by the Ultrasonic sensor
int calculateDistance(){ 
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance = duration*0.034/2;
  return distance;
}

void SerialReset(){
  Serial.end();
  Serial.begin(115200);
}
