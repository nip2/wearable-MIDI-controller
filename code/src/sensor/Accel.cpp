#include "sensor/Accel.h"
#include "etl/optional.h"
#include "etl/vector.h"

using namespace team5;
using namespace etl;

Accel::Accel(uint8_t accelSelPin, uint32_t SPI_clock, uint8_t SPI_bitOrder,
             uint8_t SPI_dataMode, String label, uint32_t waitTime,
             uint16_t threshold, uint16_t debounceState, uint8_t midiChannel) : xAxisState_(XAxisState(waitTime, threshold)),
                                                                                yAxisState_(YAxisState(waitTime, threshold)),
                                                                                accelSelPin_(accelSelPin),
                                                                                spiSettings_(SPISettings(SPI_clock, SPI_bitOrder, SPI_dataMode)),
                                                                                label_(label),
                                                                                midiChannel_(midiChannel)
{
    pinMode(accelSelPin_, OUTPUT);
    digitalWrite(accelSelPin_, HIGH);
}

void Accel::initialize()
{
    digitalWrite(accelSelPin_, LOW);
    SPI.beginTransaction(spiSettings_);
    SPI.transfer(ADXL_REGISTER_WRITE); // write instruction
    SPI.transfer(ADXL_SOFT_RESET_REG);
    SPI.transfer(ADXL_SOFT_RESET_VAL);
    SPI.endTransaction();
    digitalWrite(accelSelPin_, HIGH);

    delay(50);

    // turn on bit 1 (measure mode) of the ADXL362 power control reg, basically
    // tell it to measure continuously
    digitalWrite(accelSelPin_, LOW);
    SPI.beginTransaction(spiSettings_);
    SPI.transfer(ADXL_REGISTER_READ); // read instruction
    SPI.transfer(ADXL_POWER_CTRL_REG);
    byte powerCtrlReg = SPI.transfer(0x00);
    SPI.endTransaction();
    digitalWrite(accelSelPin_, HIGH);

    powerCtrlReg |= ADXL_POWER_CTRL_CONTINUOUS_VAL;

    digitalWrite(accelSelPin_, LOW);
    SPI.beginTransaction(spiSettings_);
    SPI.transfer(ADXL_REGISTER_WRITE); // write instruction
    SPI.transfer(ADXL_POWER_CTRL_REG);
    SPI.transfer(powerCtrlReg);
    SPI.endTransaction();
    digitalWrite(accelSelPin_, HIGH);
    Serial.printf("accelerometer %s initialized\n", label_.c_str());
}

optional<vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>> Accel::poll()
{
    AccelSample sample;
    digitalWrite(accelSelPin_, LOW);
    SPI.beginTransaction(spiSettings_);

    SPI.transfer(ADXL_REGISTER_READ); // read instruction
    SPI.transfer(ADXL_XVALUE_REG);    // Start at XData Reg
    sample.X = SPI.transfer(0x00);
    sample.X += (SPI.transfer(0x00) << 8);
    sample.Y = SPI.transfer(0x00);
    sample.Y += (SPI.transfer(0x00) << 8);
    sample.Z = SPI.transfer(0x00);
    sample.Z += (SPI.transfer(0x00) << 8);
    SPI.endTransaction();
    digitalWrite(accelSelPin_, HIGH);

    optional<uint16_t> xResult = xAxisState_.eval(sample.X);
    optional<uint16_t> yResult = yAxisState_.eval(sample.Y);

    vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE> midiEvents;

    if (xResult.has_value())
    {
        Serial.printf("Acceleromter %s MIDI volume change on chanel %d, new volume: %d\n", label_.c_str(),
                      midiChannel_, xResult.value());
        midiEvents.push_back(midiEventPacket_t{MIDI_HEADER_VOLUME,
                                               static_cast<uint8_t>(MIDI_MESSAGE_VOLUME | midiChannel_),
                                               MIDI_VOLUME_MSB, static_cast<uint8_t>(xResult.value() >> 8)});
        midiEvents.push_back(midiEventPacket_t{MIDI_HEADER_VOLUME,
                                               static_cast<uint8_t>(MIDI_MESSAGE_VOLUME | midiChannel_),
                                               MIDI_VOLUME_LSB, static_cast<uint8_t>(xResult.value() | 0x00FF)});
    }

    if (yResult.has_value())
    {

        Serial.printf("Acceleromter %s MIDI pitch change on chanel %d, new pitch: %d\n", label_.c_str(),
                      midiChannel_, yResult.value());
        midiEvents.push_back(midiEventPacket_t{MIDI_HEADER_PITCH,
                                               static_cast<uint8_t>(MIDI_MESSAGE_PITCH | midiChannel_),
                                               static_cast<uint8_t>(yResult.value() | 0x00FF),
                                               static_cast<uint8_t>(yResult.value() >> 8)});
    }
    if (midiEvents.size() > 0)
    {
        return optional<vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>>(midiEvents);
    }

   return optional<vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>>();
}

AxisState::AxisState(uint32_t waitTime, uint16_t threshold, uint16_t value) : currentState_(AccelStateName::Idle),
                                                                     waitTime_(waitTime),
                                                                     threshold_(threshold),
                                                                     value_(value)
{
}

Accel::XAxisState::XAxisState(uint32_t waitTime, uint16_t threshold) : AxisState(waitTime, threshold, 64)
{
}

Accel::YAxisState::YAxisState(uint32_t waitTime, uint16_t threshold) : AxisState(100, threshold, 8192)
{
}

optional<uint16_t> Accel::XAxisState::eval(const int16_t measured)
{
    if (waitTimeCount_ > 0)
    {
        waitTimeCount_--;
        return optional<uint16_t>();
    }

    if (measured < 0)
    {
        if (value_ == 0)
        {
            return optional<uint16_t>();
        }

        if (abs(measured) > ACCEL_THRESHOLD)
        {
            waitTimeCount_ = waitTime_;
            return optional<uint16_t>(--value_);
        }
    }

    else if (measured == 0)
    {
        return optional<uint16_t>();
    }

    else
    {

        if (value_ == 127)
        {
            return optional<uint16_t>();
        }

        if (measured > ACCEL_THRESHOLD)
        {
            waitTimeCount_ = waitTime_;
            return optional<uint16_t>(++value_);
        }
    }

    // value_ = 0;
    return optional<uint16_t>();
}

optional<uint16_t> Accel::YAxisState::eval(const int16_t measured)
{

    if (waitTimeCount_ > 0)
    {
        waitTimeCount_--;
        return optional<uint16_t>();
    }

    if (measured < 0)
    {
        if (value_ < 128)
        {
            return optional<uint16_t>();
        }

        if (abs(measured) > ACCEL_THRESHOLD)
        {
            waitTimeCount_ = waitTime_;
            value_ -= 129;
            return optional<uint16_t>(value_);
        }
        else if (value_ != 8192)
        {
            value_ = 8192;
            return optional<uint16_t>(value_);
        }
    }

    else if (measured == 8192)
    {
        return optional<uint16_t>();
    }

    else if (measured > 0)
    {

        if (value_ == 16384)
        {
            return optional<uint16_t>();
        }

        if (measured > ACCEL_THRESHOLD)
        {
            waitTimeCount_ = waitTime_;
            value_ += 128;
            return optional<uint16_t>(value_);
        }
        else if (value_ != 8192)
        {
            value_ = 8192;
            return optional<uint16_t>(value_);
        }
    }

    value_ = 8192;
    return optional<uint16_t>();
}
