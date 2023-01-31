#pragma once

#include "sensor/sensor.h"
#include "sensor/ADC.h"
#include "etl/vector.h"

#define PIEZO_LOW 600
#define PIEZO_HIGH 2400
#define MIDI_LOW 1
#define MIDI_HIGH 127

namespace team5
{
    enum class StateName
    {
        Idle = 0,
        LookingForPeak = 1,
    };

    class Piezo : public Sensor
    {
    public:
        Piezo(ADC &adc, uint32_t debounce, uint16_t threshold, uint32_t waitTime,
              uint8_t ADCChannel, uint8_t midiChanel, uint8_t midiPitch, String label);
        etl::optional<etl::vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>> poll();
        void initialize();

    private:
        ADC &adc_;

        class State
        {
        public:
            State(uint32_t debounce, uint16_t threshold, uint32_t waitTime);
            etl::optional<uint8_t> eval(const uint16_t measured);

        private:
            StateName currentState_;
            uint16_t peak_;
            uint32_t timer_;
            uint32_t debounce_;
            uint32_t debounceCounter_;
            uint16_t threshold_;
            uint32_t waitTime_;
        };

        State state_;

        uint8_t ADCChannel_;
        uint8_t midiChannel_;
        uint8_t midiPitch_;
        String label_;
    };
}