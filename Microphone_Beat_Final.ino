//Dancing Narwal code

// Our Global Sample Rate, 5000hz
#define SAMPLEPERIODUS 200

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

int beatslength = 0;
unsigned long delaytime = 40000000;
unsigned long nextTime = micros();

#include <Servo.h>

Servo myservo;  // create servo object to control a servo
Servo myotherservo;  // a maximum of eight servo objects can be created
Servo C;
Servo D;
Servo E;
Servo F;

int pos = 0;    // variable to store the servo position
int counter = 0; //variable to keep track of times looped
boolean startedpositions = false;

void setup() {
  //Serial.begin(9600); //this is useful for debugging if you would like to edit the code!

  myservo.attach(6);  // attaches the servo on pin 6 to the servo object
  myotherservo.attach(10);
  C.attach(11);
  D.attach(9);
  E.attach(5);
  F.attach(3);

  // Set ADC to 77khz, max for 10bit
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  //The pin with the output to the oscilloscope
  pinMode(2, OUTPUT);
}

// 20 - 200hz Single Pole Bandpass IIR Filter
float bassFilter(float sample) {
  static float xv[3] = {0, 0, 0}, yv[3] = {0, 0, 0};
  xv[0] = xv[1]; xv[1] = xv[2];
  xv[2] = sample / 9.1f;
  yv[0] = yv[1]; yv[1] = yv[2];
  yv[2] = (xv[2] - xv[0])
          + (-0.7960060012f * yv[0]) + (1.7903124146f * yv[1]);
  return yv[2];
}

// 10hz Single Pole Lowpass IIR Filter
float envelopeFilter(float sample) { //10hz low pass
  static float xv[2] = {0, 0}, yv[2] = {0, 0};
  xv[0] = xv[1];
  xv[1] = sample / 160.f;
  yv[0] = yv[1];
  yv[1] = (xv[0] + xv[1]) + (0.9875119299f * yv[0]);
  return yv[1];
}

// 1.7 - 3.0hz Single Pole Bandpass IIR Filter
float beatFilter(float sample) {
  static float xv[3] = {0, 0, 0}, yv[3] = {0, 0, 0};
  xv[0] = xv[1]; xv[1] = xv[2];
  xv[2] = sample / 7.015f;
  yv[0] = yv[1]; yv[1] = yv[2];
  yv[2] = (xv[2] - xv[0])
          + (-0.7169861741f * yv[0]) + (1.4453653501f * yv[1]);
  return yv[2];
}

void loop() {
  unsigned long time = micros(); // Used to track rate
  unsigned long newhighTime;
  unsigned long oldhighTime;
  unsigned long difTime;
  float sample, value, envelope, beat, thresh;
  unsigned char i;
  boolean isHigh = false;
  int pos = 0;
  boolean rising = true;



  for (i = 0; ; i++) {

    // Read ADC and center so +-512
    sample = (float)analogRead(0) - 503.f;

    // Filter only bass component
    value = bassFilter(sample);

    // Take signal amplitude and filter
    if (value < 0)value = -value;
    envelope = envelopeFilter(value);

    // Every 200 samples (25hz) filter the envelope
    if (i == 200) {
      counter++;
      i = 0; //reset sample counter
      // Filter out repeating bass sounds 100 - 180bpm
      beat = beatFilter(envelope);

      // Threshold it based on potentiometer on AN1
      thresh = 0.01f * (float)analogRead(1);

      // If we are above threshold, send a high
      if (beat > thresh) {
        digitalWrite(2, HIGH);
        if (!isHigh) {
          oldhighTime = newhighTime;
          newhighTime = micros();
          difTime = newhighTime - oldhighTime;
          //add(difTime);
          isHigh = true;
        }
      }

      else {
        digitalWrite(2, LOW);
        isHigh = false;
      }

      if (micros() - nextTime > difTime) {
        
        myservo.write(pos);              // tell servo to go to position in variable 'pos'
        myotherservo.write(pos);
        C.write(pos);
        D.write(pos);
        E.write(pos);
        F.write(pos);

        if (pos == 0){ //pos variable alternates between 0 and 90
          pos = 90;
        }
        else{
          pos = 0;
        }

        nextTime += difTime;
      }

      if (counter > 10) {
        startedpositions = true;
        counter = 0; //reset counter

        if (difTime > 5000) { //makes the narwal stop dancing if enough time has passed
          startedpositions = false;
        }
      }

    }
  }
}
