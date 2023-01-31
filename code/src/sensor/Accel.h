#pragma once

#include "sensor/sensor.h"
#include "etl/optional.h"
#include "etl/vector.h"
#include <SPI.h>

#define ADXL_WRITE_INSTRUCTION 0x0A
#define ADXL_RESET_ADDR 0x1F
#define ADXL_RESET_VALUE 0x52

#define ADXL_REGISTER_WRITE 0x0A
#define ADXL_REGISTER_READ 0x0B

#define ADXL_SOFT_RESET_REG 0x1F
#define ADXL_SOFT_RESET_VAL 0x52

#define ADXL_XVALUE_REG 0x0E

#define ADXL_POWER_CTRL_REG 0x2D
#define ADXL_POWER_CTRL_CONTINUOUS_VAL 0x02

#define MIDI_VOLUME_MSB 0x07
#define MIDI_VOLUME_LSB 0x27

#define MIDI_HEADER_VOLUME 0x0B
#define MIDI_HEADER_PITCH 0x0E

#define MIDI_MESSAGE_VOLUME 0xB0
#define MIDI_MESSAGE_PITCH 0xE0

#define ACCEL_MIN_VALUE -2048
#define ACCEL_MAX_VALUE 2047

namespace team5
{

    enum class AccelStateName
    {
        Idle = 0,
        Sustaining = 1,
    };

    class AccelSample
    {
    public:
        int16_t X;
        int16_t Y;
        int16_t Z;
    };

    class AxisState
    {
    public:
        AxisState(uint32_t waitTime, uint16_t threshold, uint16_t value);
        virtual etl::optional<uint16_t> eval(const int16_t measured) = 0;

    protected:
        AccelStateName currentState_;
        uint32_t waitTime_;
        uint32_t waitTimeCount_;
        uint16_t threshold_;
        uint16_t debounceState_;
        uint16_t value_;
    };

    class Accel : public Sensor
    {
    public:
        Accel(uint8_t accelSelPin, uint32_t SPI_clock, uint8_t SPI_bitOrder,
              uint8_t SPI_dataMode, String label, uint32_t waitTime,
              uint16_t threshold, uint16_t debounceState, uint8_t midiChannel);
        void initialize();
        etl::optional<etl::vector<midiEventPacket_t, MIDI_PACKET_VECTOR_MAXSIZE>> poll();

    private:

        class XAxisState: public AxisState {
            public:
                XAxisState(uint32_t waitTime, uint16_t threshold);
                etl::optional<uint16_t> eval(const int16_t measured);
        };
        class YAxisState: public AxisState {
            public:
                YAxisState(uint32_t waitTime, uint16_t threshold);
                etl::optional<uint16_t> eval(const int16_t measured);
        };

        XAxisState xAxisState_;
        YAxisState yAxisState_;

        uint8_t accelSelPin_;
        SPISettings spiSettings_;
        String label_;
        uint8_t midiChannel_;
    };
}
