#include "sensor/Flex.h"

using namespace team5;
using namespace etl;

Flex::Flex(ADC &adc, uint32_t waitTime, uint16_t threshold, uint8_t ADCChannel, uint8_t midiChannel,
           uint8_t midiPitch, uint8_t midiVelocity, String label) : adc_(adc),
                                                                    state_(State(waitTime, threshold)),
                                                                    ADCChannel_(ADCChannel),
                                                                    midiChannel_(midiChannel),
                                                                    midiPitch_(midiPitch),
                                                                    midiVelocity_(midiVelocity),
                                                                    label_(label)
{
}

void Flex::initialize()
{
    Serial.printf("flex sensor %s initialized\n", label_.c_str());
}

optional<vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>> Flex::poll()
{
    uint16_t flexValue = adc_.readChannel(ADCChannel_);
    optional<EvalResult> evalResult = state_.eval(flexValue);

    if (evalResult.has_value() && evalResult.value() == EvalResult::NoteOn)
    {
        Serial.printf("flex %s MIDI note on: channel: %d, pitch: %d, velocity: %d\n", label_.c_str(),
                      midiChannel_, midiPitch_, midiVelocity_);
        vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE> result{
            midiEventPacket_t{0x09, static_cast<uint8_t>(0x90 | midiChannel_), midiPitch_, midiVelocity_}};
        return optional<vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>>(result);
    }

    else if (evalResult.has_value() && evalResult.value() == EvalResult::NoteOff)
    {
        Serial.printf("flex %s MIDI note off: channel: %d, pitch: %d, velocity: %d\n", label_.c_str(),
                      midiChannel_, midiPitch_, midiVelocity_);

        vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE> result{
            midiEventPacket_t{0x08, static_cast<uint8_t>(0x80 | midiChannel_), midiPitch_, midiVelocity_}};
        return optional<vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>>(result);
    }

    return optional<vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>>();
}

Flex::State::State(uint32_t waitTime, uint16_t threshold) : currentState_(FlexStateName::Idle),
                                                            waitTime_(waitTime),
                                                            threshold_(threshold),
                                                            debounceState_(0)
{
}

optional<EvalResult> Flex::State::eval(const uint16_t measured)
{
    if (waitTime_ > 0)
    {
        waitTime_--;
        return optional<EvalResult>();
    }

    if ((currentState_ == FlexStateName::Idle) && (measured < threshold_))
    {
        debounceState_ = (debounceState_ << 1) | 0x01 | 0xe000;
        if (debounceState_ == 0xffff)
        {
            currentState_ = FlexStateName::Sustaining;
            waitTime_ = 50;
            return optional<EvalResult>(EvalResult::NoteOn);
        }
    }

    else if ((currentState_ == FlexStateName::Sustaining) && (measured >= threshold_))
    {
        debounceState_ = (debounceState_ << 1) | 0xe000;
        if (debounceState_ == 0xe000)
        {
            currentState_ = FlexStateName::Idle;
            waitTime_ = 50;
            return optional<EvalResult>(EvalResult::NoteOff);
        }
    }

    return optional<EvalResult>();
}