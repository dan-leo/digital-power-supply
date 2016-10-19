/*  
 *  Controller
 *  
 *  Controller - Software for the Arduino to be used for Design E344 
 *  Updated on 12/07/2016
 *  Author: R. D. Beyers (rdb@sun.ac.za)
 *  
 *  This is free software - you may redistribute and modify it freely.
 *  This software is provided without any warranty.
 */


#include "SimpleTimer.h"
#include <EEPROM.h>

// declaration and initialization of variables and constants

// time between samples in milliseconds
const int sampleInterval = 200; 
// digital & analog I/O pin setup
const int pwmVRefPin = 5;
const int pwmIRefPin = 6;
const int VsensePin = A0;
const int IsensePin = A1;
const int LEDPin = 13;
// measurement variables
int sensorBufferLength = 5;
int analogSensorA0Value = 0;
int sensorA0ZeroVoltReference;
int sensorA0ZeroVoltReferenceDef = 80;
int sensorA0OtherReference;
int sensorA0OtherReferenceDef = 800;
int calOtherReferenceVoltage;
int calOtherReferenceVoltageDef = 10000;
int correctedVoltageValue;
int sensorA0SampleBuffer[20];
int sensorA0SampleBufferPosition = 0;
int analogSensorA1Value = 0;
int sensorA1ZeroMilliAReference;
int sensorA1ZeroMilliAReferenceDef = 8;
int sensorA1OtherReference;
int sensorA1OtherReferenceDef = 80;
int calOtherReferenceCurrent;
int calOtherReferenceCurrentDef = 100;
int correctedCurrentValue;
int sensorA1SampleBuffer[20];
int sensorA1SampleBufferPosition = 0;
// output variables
int VRefValue = 0;
int VRefPwmValue = 0;
int IRefValue = 0;
int IRefPwmValue = 0;
// serial communication buffers
char serialInBuffer[64];
// timer variables
SimpleTimer timer;
int serialReceiveTimerID;
int serialTransmitTimerID;
int sensorSampleTimerID;
int timedLoopTimerID;
// status variables
int isStreamingSensorData;
int isStreamingSensorDataDef = 0;
int isFirstRun;
int isFirstRunDef = 101;

// standard Arduino setup function, automatically called before the loop (below) function is called
void setup() {
  clearSampleBuffers();
  Serial.setTimeout(50);
  Serial.begin(9600);
  loadSettings();
  pinSetup();
  startTimedLoop();
}

// standard "loop" function for Arduino (similar to "main" in C program)
void loop() {
  // put your main code here, to run repeatedly:
  timer.run();
}

// setup the timed loop execution
void startTimedLoop() {
  timedLoopTimerID = timer.setInterval(sampleInterval, timedLoop); 
}

// program loop executed at a fixed interval (also the sample rate)
void timedLoop() {    
  receiveSerial();  
  updateVRefPwmValue();
  updateIRefPwmValue();
  sampleSensors();
  // fix for some compilers: not called if deeply nested
  analogWrite(pwmIRefPin,IRefPwmValue);
  analogWrite(pwmVRefPin,VRefPwmValue);
}

// function for initializing sample buffers to all zeros
void clearSampleBuffers() {
  int i = 0;
  while (i < sensorBufferLength) {
    sensorA0SampleBuffer[i] = 0;
    sensorA1SampleBuffer[i] = 0;
    i = i + 1;
  }
}

// setup pins
void pinSetup() {
  pinMode(pwmVRefPin, OUTPUT);
  analogWrite(pwmVRefPin, 0);
  pinMode(pwmIRefPin, OUTPUT);
  analogWrite(pwmIRefPin, 0);
  
  pinMode(LEDPin, OUTPUT);
  
  for (int pin=1;pin<=12;pin++){
    pinMode(pin, OUTPUT);
    analogWrite(pin, 50);
  }
}

// load settings from EEPROM
void loadSettings() {
  // read from EEPROM
  isFirstRun = int(EEPROM.read(1));
  digitalWrite(LEDPin,isFirstRun);
  isStreamingSensorData = int(EEPROM.read(2));
  sensorA0ZeroVoltReference = readSensorValueFromEEPROM(3);
  sensorA0OtherReference = readSensorValueFromEEPROM(5);
  sensorA1ZeroMilliAReference = readSensorValueFromEEPROM(7);
  sensorA1OtherReference = readSensorValueFromEEPROM(9);
  calOtherReferenceVoltage = readSensorValueFromEEPROM(11);
  calOtherReferenceCurrent = readSensorValueFromEEPROM(13);

  // check whether this is the first run
  if (isFirstRun != isFirstRunDef) {
    isFirstRun = isFirstRunDef;
    isStreamingSensorData = isStreamingSensorDataDef;
    sensorA0ZeroVoltReference = sensorA0ZeroVoltReferenceDef;
    sensorA0OtherReference = sensorA0OtherReferenceDef;
    sensorA1ZeroMilliAReference = sensorA1ZeroMilliAReferenceDef;
    sensorA1OtherReference = sensorA1OtherReferenceDef;
    calOtherReferenceVoltage = calOtherReferenceVoltageDef;
    calOtherReferenceCurrent = calOtherReferenceCurrentDef;
    // save defaults to memory
    saveSettings();
  }
}

// save settings to EEPROM
void saveSettings() {
  EEPROM.write(1, isFirstRun);
  EEPROM.write(2, isStreamingSensorData);
  writeSensorValueToEEPROM(sensorA0ZeroVoltReference, 3);
  writeSensorValueToEEPROM(sensorA0OtherReference, 5);
  writeSensorValueToEEPROM(sensorA1ZeroMilliAReference, 7);
  writeSensorValueToEEPROM(sensorA1OtherReference, 9);
  writeSensorValueToEEPROM(calOtherReferenceVoltage, 11);
  writeSensorValueToEEPROM(calOtherReferenceCurrent, 13);
}

// sample the voltage and current values at pins A0 and A1, respectively
void sampleSensors() {
  int tempV = analogRead(VsensePin);
  int tempI = analogRead(IsensePin);
  analogSensorA0Value = tempV;
  analogSensorA1Value = tempI;
  // store the result in the sensor A0 value buffer
  sensorA0SampleBuffer[sensorA0SampleBufferPosition] = analogSensorA0Value;
  sensorA0SampleBufferPosition = sensorA0SampleBufferPosition + 1;
  if (sensorA0SampleBufferPosition > sensorBufferLength - 1) {
    sensorA0SampleBufferPosition = 0;
  }
  // store the result in the sensor A1 value buffer
  sensorA1SampleBuffer[sensorA1SampleBufferPosition] = analogSensorA1Value;
  sensorA1SampleBufferPosition = sensorA1SampleBufferPosition + 1;
  if (sensorA1SampleBufferPosition > sensorBufferLength - 1) {
    sensorA1SampleBufferPosition = 0;
  }
  updateCorrectedVoltageValue();
  updateCorrectedCurrentValue();  
  if (isStreamingSensorData >= 1) {    
    String messagePartStr = "";
    messagePartStr.concat("voltage.");
    messagePartStr.concat(correctedVoltageValue);
    messagePartStr.concat("\n");
    messagePartStr.concat("current.");
    messagePartStr.concat(correctedCurrentValue);
    messagePartStr.concat("\n");
    char messagePart[64];
    messagePartStr.toCharArray(messagePart, 64);
    transmitSerial(messagePart);
  }  
}

// write a sensor value (requires two bytes - a word) to EEPROM
void writeSensorValueToEEPROM(int value, int startAddress) {
  int byte1 = value/255; // max value per byte is 255, so calculate value using byte1*255 + byte2
  int byte2 = value%255;
  EEPROM.write(startAddress, byte1);
  EEPROM.write(startAddress + 1, byte2);
}

// read a sensor value (requires two bytes - a word) from EEPROM
int readSensorValueFromEEPROM(int startAddress) {
  int byte1 = int(EEPROM.read(startAddress));
  int byte2 = int(EEPROM.read(startAddress + 1));
  return byte1*255 + byte2;
}

// receive a message via the serial port
void receiveSerial() {
  if (Serial.available() > 0) {
    int messageLength = Serial.readBytesUntil('\0', serialInBuffer, 64);
    serialInBuffer[messageLength] = '\0';
    parseMessage(serialInBuffer);
  }
}

// transmit a message via the serial port
void transmitSerial(char message[]) {
  Serial.print(message);
}

// update the measured voltage value (aggregate)
void updateCorrectedVoltageValue() {
  int i = 0;
  float temp = 0;
  while (i < sensorBufferLength) {
    temp = temp + (float)getCorrectedVoltageValue(sensorA0SampleBuffer[i]);
    i = i + 1;
  }
  correctedVoltageValue = ((int)(temp/(float)sensorBufferLength/(float)50))*50; // floor on 50 mV
}

// return the calibrated measured voltage value
int getCorrectedVoltageValue(int rawVoltageValue) {
  int delta = sensorA0OtherReference - sensorA0ZeroVoltReference;
  if (delta == 0) {
    delta = 1;
  }
  float m = (float)calOtherReferenceVoltage/((float)(sensorA0OtherReference - sensorA0ZeroVoltReference));
  return (int)(m*rawVoltageValue + (0 - m*sensorA0ZeroVoltReference));
}

// update the measured current value (aggregate)
void updateCorrectedCurrentValue() {
  int i = 0;
  float temp = 0;
  while (i < sensorBufferLength) {
    temp = temp + (float)getCorrectedCurrentValue(sensorA1SampleBuffer[i]);
    i = i + 1;
  }
  correctedCurrentValue = ((int)(temp/(float)sensorBufferLength/(float)5))*5; // floor on 5 mA
}

// return the calibrated measured current value
int getCorrectedCurrentValue(int rawCurrentValue) {
  int delta = sensorA1OtherReference - sensorA1ZeroMilliAReference;
  if (delta == 0) {
    delta = 1;
  }
  float m = (float)calOtherReferenceCurrent/((float)(sensorA1OtherReference - sensorA1ZeroMilliAReference));
  return (int)(m*rawCurrentValue + (0 - m*sensorA1ZeroMilliAReference));
}

// update the pwm output based on the current value of VRefValue (given in mV)
void updateVRefPwmValue() {
  VRefPwmValue = floor((float)VRefValue*((float)256/(float)25600)); // VRefValue given in millivolts
  analogWrite(pwmVRefPin, VRefPwmValue);
}

// update the pwm output based on the current value of IRefValue (given in mA)
void updateIRefPwmValue() {
  IRefPwmValue = floor((float)IRefValue*((float)256/(float)1280)); // IRefValue given in milliamps **************************************************
  analogWrite(pwmIRefPin, IRefPwmValue);
}

// parse a message received via serial port
void parseMessage(char message[]) {
  int i = 0;
  int j = 0;
  char messagePart[64];
  while (message[i] != '\0') {
    if (message[i] == '\n') {
      if(j > 0) {
        messagePart[j] = '\0';
        interpretMessagePart(messagePart);
      }
      j = 0;
    } else {
      messagePart[j] = message[i];
      j = j + 1;      
    }    
    i = i + 1;
  }
}

// get the length of an array of characters
int getCharArrayLength(char array[]) {
  int arrayLength = 0;
  while (array[arrayLength] != '\0') {
    arrayLength = arrayLength + 1;
  }
  return arrayLength;
}

// extracts a command from a message part
int getCommandFromPart(char command[], char messagePart[]) {
  String messagePartStr(messagePart);
  int firstPeriod = messagePartStr.indexOf('.');
  if (firstPeriod >= 0) {
    messagePartStr.toCharArray(command, firstPeriod + 1);
    command[firstPeriod] = '\0';
    return firstPeriod;
  } else {
    command[0] = '\0';
    return -1;
  }
}

// extracts parameters from a message part
int getParamsFromPart(char params[], char messagePart[]) {
  String messagePartStr(messagePart);
  int firstPeriod = messagePartStr.indexOf('.');
  int messagePartLength = getCharArrayLength(messagePart);
  if (firstPeriod >= 0) {
    messagePartStr.substring(firstPeriod + 1).toCharArray(params, messagePartLength - firstPeriod);
    params[messagePartLength - firstPeriod] = '\0';
    return messagePartLength - firstPeriod;
  } else {
    params[0] = '\0';
    return -1;
  }
}

// interprets the ascii message from the PC
void interpretMessagePart(char messagePart[]) {
  char command[32];
  char params[32]; 
  getCommandFromPart(command, messagePart);
  getParamsFromPart(params, messagePart); 
  if (!strcmp(command, "stream")) {
    if (!strcmp(params, "on")) {
      isStreamingSensorData = 1;
    } else if (!strcmp(params, "off")) {
      isStreamingSensorData = 0;
    }
    saveSettings();
  } else if (!strcmp(command, "calibrate")) {
    getCommandFromPart(command, params);
    getParamsFromPart(params, params);
    if (!strcmp(command, "voltage")) {
      if (!strcmp(params, "zero")) {
        sensorA0ZeroVoltReference = analogSensorA0Value;
      } else if (!strcmp(params, "other")) {
        sensorA0OtherReference = analogSensorA0Value;
      }
      saveSettings();
    } else if (!strcmp(command, "current")) {
      if (!strcmp(params, "zero")) {
        sensorA1ZeroMilliAReference = analogSensorA1Value;
      } else if (!strcmp(params, "other")) {
        sensorA1OtherReference = analogSensorA1Value;
      }
      saveSettings();
    }
  } else if (!strcmp(command, "set")) {
    getCommandFromPart(command, params);
    getParamsFromPart(params, params);
    if (!strcmp(command, "voltage")) {
      int i = 0;
      int val = 0;
      while (params[i] != '\0') {
        val = val*10 + (params[i] - '0');
        i = i + 1;
      }
      VRefValue = val; // val in millivolts;
    } else if (!strcmp(command, "current")) {
      int i = 0;
      int val = 0;
      while (params[i] != '\0') {
        val = val*10 + (params[i] - '0');
        i = i + 1;
      }
      IRefValue = val; // val in milliamps;
    } else if (!strcmp(command, "calVoltage")) {
      int i = 0;
      int val = 0;
      while (params[i] != '\0') {
        val = val*10 + (params[i] - '0');
        i = i + 1;
      }
      calOtherReferenceVoltage = val; // val in millivolts;
      saveSettings();
    } else if (!strcmp(command, "calCurrent")) {
      int i = 0;
      int val = 0;
      while (params[i] != '\0') {
        val = val*10 + (params[i] - '0');
        i = i + 1;
      }
      calOtherReferenceCurrent = val; // val in milliamps;
      saveSettings();
    }
  }
}











