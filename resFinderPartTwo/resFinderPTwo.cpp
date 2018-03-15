
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_ILI9341.h>
#include "functions.h"
#include <TouchScreen.h>

bool mouseIsActive;//stores value of the click


int main() {
	setup();

  while(true){
    mouseIsActive = digitalRead(2);//reads SW value on joystick

    if(mouseIsActive == true){
      modeZero();//default is mode 0
			waitOnTouch();
    }
    else if(mouseIsActive == false){//when button is clicked
      modeOne(); //call modeOne
    }

  }

	Serial.end();
	return 0;
}
