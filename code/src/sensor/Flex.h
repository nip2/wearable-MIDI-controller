#pragma once

#include "sensor/sensor.h"
#include "sensor/ADC.h"
#include "etl/optional.h"
#include "etl/vector.h"

namespace team5
{
    enum class EvalResult
    {
        NoteOn = 0,
        NoteOff = 1,
    };

    enum class FlexStateName
    {
        Idle = 0,
        Sustaining = 1,
    };

    class Flex : public Sensor
    {
    public:
        Flex(ADC &adc, uint32_t waitTime, uint16_t threshold, uint8_t ADCChannel, uint8_t midiChanel,
             uint8_t midiPitch, uint8_t midiVelocity, String label);
        etl::optional<etl::vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>> poll();
        void initialize();

    private:
        ADC &adc_;

        class State
        {
        public:
            State(uint32_t waitTime, uint16_t threshold);
            etl::optional<EvalResult> eval(const uint16_t measured);

        private:
            FlexStateName currentState_;
            uint32_t waitTime_;
            uint16_t threshold_;
            uint16_t debounceState_;
        };

        State state_;
        uint8_t ADCChannel_;
        uint8_t midiChannel_;
        uint8_t midiPitch_;
        uint8_t midiVelocity_;
        String label_;
    };
}
