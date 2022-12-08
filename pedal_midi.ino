/*
  Adpated from http://www.arduino.cc/en/Tutorial/MidiDevice
*/

#include <MIDIUSB.h>
#include <pitchToNote.h>

#define NUM_COLUMNS 4
#define NUM_ROWS 8

// Use fixed intensity, the range is 0-127
#define INTENSITY 127

// Data is not sent until buffer is full, size unit in bytes, should be greater than or equal to 4 (size of midiEventPacket_t)
// #define BUFFER_SIZE 64


// Define your pin numbers here
const uint8_t pedal1 = 2;
const uint8_t pedal2 = 3;
const uint8_t pedal3 = 4;
const uint8_t pedal4 = 5;
const uint8_t pedal5 = 6;
const uint8_t pedal6 = 7;
const uint8_t pedal7 = 8;
const uint8_t pedal8 = 9;

const uint8_t group1 = 10;
const uint8_t group2 = 11;
const uint8_t group3 = 12;
const uint8_t group4 = 13;

const uint8_t pedals[NUM_ROWS] = {pedal1, pedal2, pedal3, pedal4, pedal5, pedal6, pedal7, pedal8};
const uint8_t groups[NUM_COLUMNS] = {group1, group2, group3, group4};

uint8_t pressedPedals[NUM_COLUMNS] = {0, 0, 0, 0};

uint8_t previousPedals[NUM_COLUMNS] = {0, 0, 0, 0};

// unsigned int bufferCount = 0;


/*
 * Calculate pitch from column and row numbers
 * col: from 0 to NUM_COLUMNS-1
 * row: from 0 to NUM_ROWS-1
 */
uint8_t getPitch(uint8_t col, uint8_t row) {
  return pitchC1 + col * 8 + row;
}

/*
 * Set target column pin to HIGH the other to low
 * col: from 0 to NUM_COLUMNS-1
 */
void enableColumn(uint8_t col) {
  for (int i = 0; i < NUM_COLUMNS; i++) {
    if (col == i) {
      digitalWrite(groups[i], HIGH);
    } else {
      digitalWrite(groups[i], LOW);
    }
  }
}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  // bufferCount += sizeof(midiEventPacket_t);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  // bufferCount += sizeof(midiEventPacket_t);
}

// void flushBuffer() {
//   MidiUSB.flush();
//   bufferCount = 0;
// }


////////////////////////////////////////////
//////////   reserved functions   //////////
////////////////////////////////////////////
void setup() {
  for (int i = 0; i < NUM_ROWS; i++)
    pinMode(pedals[i], INPUT_PULLUP);

  for (int i = 0; i < NUM_COLUMNS; i++)
    pinMode(groups[i], OUTPUT);
}

void loop() {

  // Read pedals
  for (int i = 0; i < NUM_COLUMNS; i++) {
    enableColumn(i);
    for (int j = 0; i < NUM_ROWS; j++) {
      if (digitalRead(pedals[j]) == LOW) {
        bitWrite(pressedPedals[i], j, 1);
      } else {
        bitWrite(pressedPedals[i], j, 0);
      }
    }
  }

  // Queue events
  for (int i = 0; i < NUM_COLUMNS; i++) {
    for (int j = 0; i < NUM_ROWS; j++) {

      if (bitRead(pressedPedals[i], j) != bitRead(previousPedals[i], j)) {
        if (bitRead(pressedPedals[i], j)) {
          // press
          bitWrite(previousPedals[i], j , 1);
          noteOn(0, getPitch(i, j), INTENSITY);
          MidiUSB.flush();
        } else {
          // release
          bitWrite(previousPedals[i], j , 0);
          noteOff(0, getPitch(i, j), 0);
          MidiUSB.flush();
        }
      }

    }
  }

  // flush buffer
  // if (bufferCount >= BUFFER_SIZE)
  //   flushBuffer();
}
