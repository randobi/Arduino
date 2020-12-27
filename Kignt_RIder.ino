
/* Knight Rider 1
 * --------------
 *
 * Basically an extension of Blink_LED.
 *
 *
 * (cleft) 2005 K3, Malmo University
 * @author: David Cuartielles
 * @hardware: David Cuartielles, Aaron Hallborg
 */

int pin2 = 2;
int pin3 = 3;
int pin4 = 4;
int pin5 = 5;
int inPin = 6;
int outPin = 12;
int timer = 200;

int state = HIGH;      // the current state of the output pin
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin

// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers


void setup(){
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);
  pinMode(pin5, OUTPUT);
  pinMode(inPin, INPUT);
  pinMode(outPin, OUTPUT);

  }

void loop() {
  reading = digitalRead(inPin);

  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember
  // the time
  if (reading == HIGH && previous == LOW && millis() - time > debounce) {
    if (state == HIGH)
      state = LOW;
    else
      state = HIGH;

    time = millis();    
}
  digitalWrite(outPin, state);

  previous = reading;

  /* on/off switch
  digitalWrite(pin5, HIGH);
  delay (10000);
  digitalWrite(pin5, LOW);
  delay (10000);
  */
  // Set 1
   digitalWrite(pin2, HIGH);
   digitalWrite(pin3, LOW);
   digitalWrite(pin4, LOW);
   delay(timer);
   
   digitalWrite(pin2, LOW);   
   digitalWrite(pin3, HIGH);
   digitalWrite(pin4, LOW);
   delay(timer);
   
   digitalWrite(pin2, LOW);
   digitalWrite(pin3, LOW);
   digitalWrite(pin4, HIGH);
   delay(timer);
   
/* digitalWrite(pin2, HIGH);
   digitalWrite(pin4, LOW);
   digitalWrite(pin2, LOW);
   delay(timer);
*/
}

