
// Detect the falling edge

// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce-Arduino-Wiring
#include <Wire.h>
#include <Bounce2mcp.h>
#include <Adafruit_MCP23017.h>

#define BUTTON_PIN 2
#define LED_PIN 13

int ledState = LOW;

// Instantiate a Bounce object
BounceMcp debouncer = BounceMcp(); 

// Instantiate an MCP object
Adafruit_MCP23017 mcp0;

void setup() {
  
  // Begin the MCP object
  mcp0.begin(0);

  // Setup the button with an internal pull-up :
  mcp0.pinMode(BUTTON_PIN, INPUT);
  mcp0.pullUp(BUTTON_PIN, HIGH); 

  // After setting up the button, setup the Bounce instance :
  debouncer.attach(mcp0, BUTTON_PIN, 5);
    
  // Setup the LED :
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN,ledState);
 
  
}

void loop() {

  // Update the Bounce instance :
   debouncer.update();
   
   // Call code if Bounce fell (transition from HIGH to LOW) :
   if ( debouncer.fell() ) {
      
     // Toggle LED state :
     ledState = !ledState;
     digitalWrite(LED_PIN,ledState);
     
   }
}

