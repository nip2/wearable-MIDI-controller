#include "MIDIController.h"

using namespace team5;
using namespace etl;

uint32_t watchdog = 0;

ADC ADCLeft(
    10,        // the CS pin on the MCU for this ADC
    1000000,   // SPI clock rate to use for this ADC
    MSBFIRST,  // SPI bit order to use for this ADC
    SPI_MODE0, // SPI mode to use for this ADC
    "left");

ADC ADCRight(
    8,        // the CS pin on the MCU for this ADC
    1000000,   // SPI clock rate to use for this ADC
    MSBFIRST,  // SPI bit order to use for this ADC
    SPI_MODE0, // SPI mode to use for this ADC
    "right");

// initialize an array of "sensors". A Sensor is an accelerometer, a piezo, or a flex resistor
// each sensor has it's own state machine and it's own midi settings. This is the only place where
// anything needs to be set or changed.
vector<Sensor *, 12> sensors({

    new Accel(
        9,         // the CS pin on the MCU for this Accelerometer
        1000000,   // SPI clock rate to use for this ADC
        MSBFIRST,  // SPI bit order to use for this ADC
        SPI_MODE0, // SPI mode to use for this ADC
        "left",
        ACCEL_WAIT_TIME,
        ACCEL_THRESHOLD,
        ACCEL_DEBOUNCE_TIME,
        FLEX_MIDI_CHANNEL),

    // new Accel(
    //     7,         // the CS pin on the MCU for this Accelerometer
    //     1000000,   // SPI clock rate to use for this ADC
    //     MSBFIRST,  // SPI bit order to use for this ADC
    //     SPI_MODE0, // SPI mode to use for this ADC
    //     "right",
    //     ACCEL_WAIT_TIME,
    //     ACCEL_THRESHOLD,
    //     ACCEL_DEBOUNCE_TIME,
    //     FLEX_MIDI_CHANNEL),

    new Piezo(
        ADCLeft,
        PIEZO_DEBOUNCE_TIME, PIEZO_THRESHOLD, PIEZO_WAIT_TIME,
        0,   // ADC Channel that this piezo belongs to
        0,   // MIDI Channel this piezo will emit MIDI events on
        36, // the MIDI pitch that this piezo will use when it emits MIDI events
        "left_3"),

    new Piezo(
        ADCLeft,
        PIEZO_DEBOUNCE_TIME, PIEZO_THRESHOLD, PIEZO_WAIT_TIME,
        1,   // ADC Channel that this piezo belongs to
        0,   // MIDI Channel this piezo will emit MIDI events on
        38, // the MIDI pitch that this piezo will use when it emits MIDI events
        "left_1"),

    new Piezo(
        ADCLeft,
        PIEZO_DEBOUNCE_TIME, PIEZO_THRESHOLD, PIEZO_WAIT_TIME,
        2,   // ADC Channel that this piezo belongs to
        0,   // MIDI Channel this piezo will emit MIDI events on
        40, // the MIDI pitch that this piezo will use when it emits MIDI events
        "left_2"),

    new Piezo(
        ADCLeft,
        PIEZO_DEBOUNCE_TIME, PIEZO_THRESHOLD, PIEZO_WAIT_TIME,
        3,   // ADC Channel that this piezo belongs to
        0,   // MIDI Channel this piezo will emit MIDI events on
        41, // the MIDI pitch that this piezo will use when it emits MIDI events
        "left_5"),

    new Piezo(
        ADCLeft,
        PIEZO_DEBOUNCE_TIME, PIEZO_THRESHOLD, PIEZO_WAIT_TIME,
        4,   // ADC Channel that this piezo belongs to
        0,   // MIDI Channel this piezo will emit MIDI events on
        43, // the MIDI pitch that this piezo will use when it emits MIDI events
        "left_4"),

    new Piezo(
        ADCLeft,
        PIEZO_DEBOUNCE_TIME, PIEZO_THRESHOLD, PIEZO_WAIT_TIME,
        5,   // ADC Channel that this piezo belongs to
        0,   // MIDI Channel this piezo will emit MIDI events on
        42, // the MIDI pitch that this piezo will use when it emits MIDI events
        "left_0"),

    new Flex(
        ADCLeft,
        FLEX_WAIT_TIME,
        FLEX_THRESHOLD,
        6,   // The ADC chanel that this flex beongs to
        FLEX_MIDI_CHANNEL,   // MIDI Channel this flex will emit MIDI events on
        60, // the MIDI pitch that this flex will use when it emits events
        90, // the MIDI velocity that this flex will use when it emits MIDI events
        "left_1"),

    new Flex(
        ADCLeft,
        FLEX_WAIT_TIME,
        FLEX_THRESHOLD,
        7,   // The ADC chanel that this flex beongs to
        FLEX_MIDI_CHANNEL,   // MIDI Channel this flex will emit MIDI events on
        67, // the MIDI pitch that this flex will use when it emits events
        90, // the MIDI velocity that this flex will use when it emits MIDI events
        "left_0"),

    new Flex(
        ADCRight,
        FLEX_WAIT_TIME,
        FLEX_THRESHOLD,
        1,   // The ADC chanel that this flex beongs to
        FLEX_MIDI_CHANNEL,   // MIDI Channel this flex will emit MIDI events on
        71, // the MIDI pitch that this flex will use when it emits events
        90, // the MIDI velocity that this flex will use when it emits MIDI events
        "right_0"),

    new Flex(
        ADCRight,
        FLEX_WAIT_TIME,
        FLEX_THRESHOLD,
        0,   // The ADC chanel that this flex beongs to
        FLEX_MIDI_CHANNEL,   // MIDI Channel this flex will emit MIDI events on
        72, // the MIDI pitch that this flex will use when it emits events
        90, // the MIDI velocity that this flex will use when it emits MIDI events
        "right_1"),
});

void setup()
{
  Serial.begin(9600);
  Serial.println("serial console initialized");
  SPI.begin();
  Serial.println("MCU SPI bus initialized");
  for (Sensor *sensor : sensors)
  {
    sensor->initialize();
  }
}

void loop()
{
  // this is needed sometimes to keep the serial port from getting clogged up
  while (!Serial)
    ;

  // Sometimes the host (computer) will send us midi events. Dunno what they are, but we need to read
  // them, or else they'll block
  while(MidiUSB.available()) {
    Serial.println("reading midi message");
    midiEventPacket_t blah = MidiUSB.read();
    Serial.printf("Midi packet: %x, %x, %x, %x\n", blah.header, blah.byte1, blah.byte2, blah.byte3);
  }

  // loop though every sensor
  for (Sensor *sensor : sensors)
  {
    // call the sensor's poll() function, which may or may not return a vector of midi events
    // depending on what the current state of the sensor is
    optional<vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>> midiEvents = sensor->poll();

    // if the returned "optional" has a value, then there is at least one midi event we need to send to
    // the host, there may be more, as this is a vector so just iterate though all of them
    if (midiEvents.has_value()) {
      for (midiEventPacket_t midiEvent : midiEvents.value())
      {
          // send off the midi event
          MidiUSB.sendMIDI(midiEvent);
          MidiUSB.flush();
      }
    }
  }
  // tell the user we're still here and didn't crash
  watchdog++;

  if (watchdog % 1000000 == 0)
  {
    Serial.println("still alive");
  }
}
