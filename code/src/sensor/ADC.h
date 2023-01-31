#pragma once
#include <SPI.h>

#define ADC_MAX_VALUE 4095

namespace team5
{
    class ADC
    {
    public:
        ADC(uint8_t ADCSelPin, uint32_t SPI_clock, uint8_t SPI_bitOrder, uint8_t SPI_dataMode, String label);
        uint16_t readChannel(const uint8_t channel);

    private:
        uint8_t ADCSelPin_;
        SPISettings spiSettings_;
        String label_;
    };
}