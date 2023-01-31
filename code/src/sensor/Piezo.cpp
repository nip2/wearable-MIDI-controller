#include "sensor/piezo.h"

using namespace team5;
using namespace etl;

Piezo::Piezo(ADC &adc, uint32_t debounce, uint16_t threshold,
             uint32_t waitTime, uint8_t ADCChannel, uint8_t midiChannel,
             uint8_t midiPitch, String label) : adc_(adc),
                                                state_(State(debounce, threshold, waitTime)),
                                                ADCChannel_(ADCChannel),
                                                midiChannel_(midiChannel),
                                                midiPitch_(midiPitch),
                                                label_(label)
{
}

void Piezo::initialize()
{
    Serial.printf("piezo sensor %s initialized\n", label_.c_str());
}

optional<vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>> Piezo::poll()
{
    uint16_t piezoValue = adc_.readChannel(ADCChannel_);

    optional<uint8_t> velocity = state_.eval(piezoValue);
    if (velocity.has_value())
    {
        Serial.printf("piezo %s MIDI note on: channel: %d, pitch: %d, velocity: %d\n",
                      label_.c_str(), midiChannel_, midiPitch_, velocity.value());

        // vector of two midi events: note on, immediately followed by a not off
        vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE> result({
            midiEventPacket_t{0x09, static_cast<uint8_t>(0x90 | midiChannel_), midiPitch_, velocity.value()}, 
            midiEventPacket_t{0x08, static_cast<uint8_t>(0x80 | midiChannel_), midiPitch_, velocity.value()}});
        
        return optional<vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>>(result);
    }
    return optional<vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>>();
}

Piezo::State::State(uint32_t debounce, uint16_t threshold, uint32_t waitTime) : currentState_(StateName::Idle),
                                                                                peak_(0),
                                                                                timer_(0),
                                                                                debounce_(debounce),
                                                                                debounceCounter_(debounce),
                                                                                threshold_(threshold),
                                                                                waitTime_(waitTime)
{
}

optional<uint8_t> Piezo::State::eval(const uint16_t measured)
{
    if (debounceCounter_ > 0)
    {
        debounceCounter_--;
    }
    else if (measured > 2400)
    {
    }
    else if (currentState_ == StateName::Idle)
    {
        if (measured > threshold_)
        {
            currentState_ = StateName::LookingForPeak;
            peak_ = measured;
            timer_ = 0;
        }
    }
    else if (currentState_ == StateName::LookingForPeak)
    {
        if (measured < 2000 && measured > peak_)
        {
            peak_ = measured;
            timer_ = 0;
        }
        else if (timer_ >= waitTime_)
        {
            uint16_t newValue = constrain(peak_, threshold_, PIEZO_HIGH);
            uint8_t velocity = newValue >= 4095 ? MIDI_HIGH : map(peak_, PIEZO_LOW, PIEZO_HIGH, MIDI_LOW, MIDI_HIGH);

            timer_ = 0;
            peak_ = 0;
            currentState_ = StateName::Idle;
            debounceCounter_ = debounce_;

            return optional<uint8_t>(velocity);
        }
        else
        {
            timer_++;
        }
    }

    return optional<uint8_t>();
}