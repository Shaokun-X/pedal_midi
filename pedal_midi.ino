/*
  Adpated from http://www.arduino.cc/en/Tutorial/MidiDevice
*/

#include <MIDIUSB.h>

#define PEDALS_CHANNEL 0
#define CONTROL_CHANNEL 1

#define PITCH_C1 24

#define NUM_PEDALS 30

// in micro seconds
#define BOUNCE_WINDOW 500

// Use fixed intensity, the range is 0-127
#define INTENSITY 127

// Data is not sent until buffer is full, size unit in bytes, should be greater than or equal to 4 (size of midiEventPacket_t)
// #define BUFFER_SIZE 64

// Define your pin numbers here
#define PEDAL1 22
#define PEDAL2 23
#define PEDAL3 24
#define PEDAL4 25
#define PEDAL5 26
#define PEDAL6 27
#define PEDAL7 28
#define PEDAL8 29
#define PEDAL9 30
#define PEDAL10 31
#define PEDAL11 32
#define PEDAL12 33
#define PEDAL13 34
#define PEDAL14 35
#define PEDAL15 36
#define PEDAL16 37
#define PEDAL17 38
#define PEDAL18 39
#define PEDAL19 40
#define PEDAL20 41
#define PEDAL21 42
#define PEDAL22 43
#define PEDAL23 44
#define PEDAL24 45
#define PEDAL25 46
#define PEDAL26 47
#define PEDAL27 48
#define PEDAL28 49
#define PEDAL29 50
#define PEDAL30 51
#define EXPRESSION_PEDAL A0

const uint8_t pedals[NUM_PEDALS] = {
    PEDAL1, PEDAL2, PEDAL3, PEDAL4, PEDAL5, PEDAL6, PEDAL7, PEDAL8,
    PEDAL9, PEDAL10, PEDAL11, PEDAL12, PEDAL13, PEDAL14, PEDAL15, PEDAL16,
    PEDAL17, PEDAL18, PEDAL19, PEDAL20, PEDAL21, PEDAL22, PEDAL23, PEDAL24,
    PEDAL25, PEDAL26, PEDAL27, PEDAL28, PEDAL29, PEDAL30};

// one bit per pedal
unsigned long int pressedPedals = 0;
unsigned long int previousPedals = 0;

unsigned lastExpressionPedal = 0;

unsigned long lastTriggerTime[NUM_PEDALS];

// unsigned int bufferCount = 0;

/*
 * Calculate pitch from key number
 */
uint8_t getPitch(int n)
{
  return PITCH_C1 + n;
}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).
void noteOn(byte channel, byte pitch, byte velocity)
{
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  // bufferCount += sizeof(midiEventPacket_t);
}

void noteOff(byte channel, byte pitch, byte velocity)
{
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  // bufferCount += sizeof(midiEventPacket_t);
}

void controlChange(byte channel, byte control, byte value) 
{
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
// void flushBuffer() {
//   MidiUSB.flush();
//   bufferCount = 0;
// }

////////////////////////////////////////////
//////////   reserved functions   //////////
////////////////////////////////////////////
void setup()
{
  // Due support 12 bits resolution 0-4095
  analogReadResolution(12);
  for (int i = 0; i < NUM_PEDALS; i++)
  {
    pinMode(pedals[i], INPUT_PULLUP);
    lastTriggerTime[i] = 0;
  }
}

void loop()
{

  // Read debounced pedals
  for (int i = 0; i < NUM_PEDALS; i++)
  {
    if (lastTriggerTime[i] == 0) {
      // first check
      if (digitalRead(pedals[i]) == LOW) {
        lastTriggerTime[i] = micros();
      } 
      else 
      {
        // no need to check release
        bitWrite(pressedPedals, i, 0);
      }
    }
    else
    {
      // when elapsed time reaches the threshold, do a second check
      if (micros() - lastTriggerTime[i] > BOUNCE_WINDOW)
      {
        lastTriggerTime[i] = 0;
        if (digitalRead(pedals[i]) == LOW) bitWrite(pressedPedals, i, 1);
      }
    }
  }

  // sent pedal midi signals
  for (int i = 0; i < NUM_PEDALS; i++)
  {
    if (bitRead(pressedPedals, i) != bitRead(previousPedals, i))
    {
      if (bitRead(pressedPedals, i)) {
          // press
          bitWrite(previousPedals, i, 1);
          noteOn(PEDALS_CHANNEL, getPitch(i), INTENSITY);
          MidiUSB.flush();
      }
      else
      {
          // release
          bitWrite(previousPedals, i, 0);
          noteOff(PEDALS_CHANNEL, getPitch(i), 0);
          MidiUSB.flush();
      }
    }
  }

  // read the analog input of expression pedal
  unsigned currentExpressionPedal = analogRead();
  if (currentExpressionPedal != lastExpressionPedal)
  {
    lastExpressionPedal = currentExpressionPedal;
    unsigned char value = map(currentExpressionPedal, 0, 4095, 0, 127);
    value = constrain(value, 0, 127);
    controlChange(CONTROL_CHANNEL, 11, value);
  }

  // flush buffer
  // if (bufferCount >= BUFFER_SIZE)
  //   flushBuffer();
}
