
#include <EEPROM.h>

const int buttonPin = 2;     // the number of the pushbutton pin
const int ledPin =  13;      // the number of the LED pin

// Variables will change:
int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

// contador de brejas bebidas
int brejaCounter = 0;

void setupBrejaButton() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(3,OUTPUT);
  Serial.begin(9600);
  brejaCounter = EEPROM.read(0);
  brejaCounter |= EEPROM.read(1) << 8;
}



void brejaButton() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button 
  // (i.e. the input went from LOW to HIGH),  and you've waited 
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  } 
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    if (buttonState != reading && reading == HIGH) {
       brejaCounter++;
       EEPROM.write(0, brejaCounter&0xFF);
       EEPROM.write(1, brejaCounter>>8);
       Serial.print("uma breja, total: ");
       Serial.println(brejaCounter);
       for (int x = 0; x < 3; x++) {
          digitalWrite(3, HIGH);
          delay(100);
          digitalWrite(3, LOW);
          delay(100);
       }
    }
    buttonState = reading;

  }
  
  // set the LED using the state of the button:
  digitalWrite(ledPin, buttonState);
  

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
}

