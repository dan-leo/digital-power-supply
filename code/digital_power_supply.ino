/*
 * References
 *     http://playground.arduino.cc/Code/Timer1
 */

#include "Arduino.h"
//#include "pins_arduino.h"
#include "system/nokia_lcd.h"
#include "system/pins.h"
#include "timer/TimerOne.h"

volatile uint16_t counter;
volatile uint16_t sub_counter = 100;

void callback();

// the setup function runs once when you press reset or power the board
void setup() {
	// initialize digital pin 13 as an output.
	pinMode(led_builtin, OUTPUT);
	pinMode(led_white, OUTPUT);
	pinMode(12, OUTPUT);
	digitalWrite(12, HIGH);

	Serial.begin(9600);
	Serial.print("Welcome, Daniel!\n");
	setup_lcd();

	char string[] = "Welcome,    Daniel!"; //String("Hello World!");
	LcdString(string);

	Timer1.initialize(200000);
	Timer1.pwm(12, 16);
//	Timer1.setPwmDuty(led_builtin, 16);
	Timer1.attachInterrupt(callback);
	analogReference(INTERNAL);
}

// the loop function runs over and over again forever
void loop() {
	if (Serial.available()){
		int read_in = Serial.read();
		Serial.write(read_in);
		LcdCharacter(read_in);
	}
//	if (counter < ((counter%100)/2)){
	if (counter < sub_counter){
//		  digitalWrite(led_builtin, HIGH);   // turn the LED on (HIGH is the voltage level)
//		  digitalWrite(led_white, LOW);
	}
	else{
//		  digitalWrite(led_builtin, LOW);    // turn the LED off by making the voltage LOW
//		  digitalWrite(led_white, HIGH);
	}
	if (!counter){
		int pot_reading_int = analogRead((unsigned char)A0);
//		double pot_reading = (double)pot_reading_int/1024;
		Serial.print(String(pot_reading_int) + "\n");
	}
	counter++;
	counter%=1000;
	delay(1);
}

void callback() {
	digitalWrite(led_builtin, digitalRead(led_builtin)^1);
}
