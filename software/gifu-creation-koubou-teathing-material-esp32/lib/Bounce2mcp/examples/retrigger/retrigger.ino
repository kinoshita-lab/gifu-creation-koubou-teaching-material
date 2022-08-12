
/* 
DESCRIPTION
====================
Example of the bounce library that shows how to retrigger an event when a button is held down.
In this case, the debug LED will blink every 500 ms as long as the button is held down.
Open the Serial Monitor (57600 baud) for debug messages.

*/

// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce-Arduino-Wiring

#include <Wire.h>
#include <Bounce2mcp.h>
#include <Adafruit_MCP23017.h>


#define BUTTON_PIN 2
#define LED_PIN 13

// Instantiate a Bounce object
BounceMcp debouncer = BounceMcp(); 

// Instantiate an MCP object
Adafruit_MCP23017 mcp0;

int buttonState;
unsigned long buttonPressTimeStamp;

int ledState;

void setup() {
  
  Serial.begin(57600);
  
  // Begin the MCP object
  mcp0.begin(0);

  // Setup the button with an internal pull-up :
  mcp0.pinMode(BUTTON_PIN, INPUT);
  mcp0.pullUp(BUTTON_PIN, HIGH); 

  // After setting up the button, setup the Bounce instance :
  debouncer.attach(mcp0, BUTTON_PIN, 5);

  //Setup the LED
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN,ledState);
  
}

void loop() {
 // Update the debouncer and get the changed state
  boolean changed = debouncer.update();


  
  if ( changed ) {
       // Get the update value
     int value = debouncer.read();
    if ( value == HIGH) {
       ledState = LOW;
       digitalWrite(LED_PIN, ledState );
   
       buttonState = 0;
       Serial.println("Button released (state 0)");
   
   } else {
          ledState = HIGH;
       digitalWrite(LED_PIN, ledState );
       
         buttonState = 1;
         Serial.println("Button pressed (state 1)");
         buttonPressTimeStamp = millis();
     
   }
  }
  
  if  ( buttonState == 1 ) {
    if ( millis() - buttonPressTimeStamp >= 500 ) {
         buttonPressTimeStamp = millis();
         if ( ledState == HIGH ) ledState = LOW;
         else if ( ledState == LOW ) ledState = HIGH;
         digitalWrite(LED_PIN, ledState );
        Serial.println("Retriggering button");
    }
  }
 
 
}


