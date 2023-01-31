#pragma once

#include <MIDIUSB.h>
#include <etl/optional.h>
#include <etl/optional.h>
#include <etl/vector.h>
#include "Configuration.h"

namespace team5
{
    class Sensor
    {
    public:
        virtual etl::optional<etl::vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>> poll();
        virtual void initialize();

    protected:
        uint8_t midiChan;
        uint8_t midiPitch;
    };
}