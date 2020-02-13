#include "Adafruit_VL53L0X.h"
#include "MIDIUSB.h"

// address we will assign if dual sensor is present
#define LOX1_ADDRESS 0x30
#define LOX2_ADDRESS 0x31

// set the pins to shutdown
#define SHT_LOX1 7
#define SHT_LOX2 6

// objects for the vl53l0x
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();

// this holds the measurement
VL53L0X_RangingMeasurementData_t measure1;
VL53L0X_RangingMeasurementData_t measure2;

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}


void setID() {
  // all reset
  digitalWrite(SHT_LOX1, LOW);    
  digitalWrite(SHT_LOX2, LOW);
  delay(10);
  // all unreset
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, HIGH);
  delay(10);

  // activating LOX1 and reseting LOX2
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, LOW);

  // initing LOX1
  if(!lox1.begin(LOX1_ADDRESS)) {
    Serial.println(F("Failed to boot first VL53L0X"));
    while(1);
  }
  delay(10);

  // activating LOX2
  digitalWrite(SHT_LOX2, HIGH);
  delay(10);

  //initing LOX2
  if(!lox2.begin(LOX2_ADDRESS)) {
    Serial.println(F("Failed to boot second VL53L0X"));
    while(1);
  }
}

uint16_t last_note[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint16_t new_note[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };


void read_dual_sensors() {
  
  lox1.rangingTest(&measure1, false); // pass in 'true' to get debug data printout!
  lox2.rangingTest(&measure2, false); // pass in 'true' to get debug data printout!

  // print sensor one reading
  Serial.print("1: ");
  if(measure1.RangeStatus != 4) {     // if not out of range

     Serial.print(measure1.RangeMilliMeter);

     new_note[0] = measure1.RangeMilliMeter;
     uint16_t tmp_note_low = last_note[0] - 50;
     uint16_t tmp_note_high = last_note[0] + 50;
     
     if (new_note[0] < tmp_note_low || new_note[0] > tmp_note_high){

        noteOff(1, 48, 127);
        
        noteOn(1, 48, 127);
        MidiUSB.flush();

        last_note[0] = new_note[0];
     }
  } else {
    noteOff(1, 48, 127);
    MidiUSB.flush();

    last_note[0] = 0;
    
    Serial.print("Out of range");
  }
  
  Serial.print(" ");

  // print sensor two reading
  Serial.print("2: ");
  if(measure2.RangeStatus != 4) {
    
    Serial.print(measure2.RangeMilliMeter);
  
    new_note[1] = measure2.RangeMilliMeter;
    uint16_t tmp_note_low = last_note[1] - 50;
    uint16_t tmp_note_high = last_note[1] + 50;
    
    if (new_note[1] < tmp_note_low || new_note[1] > tmp_note_high){
    
      noteOff(2, 55, 127);
      
      noteOn(2, 55, 127);
      MidiUSB.flush();
  
      last_note[1] = new_note[1];
    }
  } else {
    noteOff(2, 55, 127);
    MidiUSB.flush();

    last_note[1] = 0;
    
    Serial.print("Out of range");
  }
  
  Serial.println();
}


void setup() {
  Serial.begin(115200);

  pinMode(SHT_LOX1, OUTPUT);
  pinMode(SHT_LOX2, OUTPUT);

  Serial.println("Shutdown pins inited...");

  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);

  Serial.println("Both in reset mode...(pins are low)");
  
  Serial.println("Starting...");
  setID();
}



void loop() {
  read_dual_sensors();
  delay(50);
}
