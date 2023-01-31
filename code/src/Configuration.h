#pragma once

#define MIDI_PACKET_VECTOR_MAXSIZE 10

#define FLEX_WAIT_TIME 50
#define FLEX_THRESHOLD 1500
#define FLEX_MIDI_CHANNEL 1

#define PIEZO_THRESHOLD 700
#define PIEZO_WAIT_TIME 200
#define PIEZO_DEBOUNCE_TIME 400

#define ACCEL_WAIT_TIME 100
#define ACCEL_THRESHOLD 500
#define ACCEL_DEBOUNCE_TIME 500

// change this to #define to print accel values
#undef DEBUG_ACCEL