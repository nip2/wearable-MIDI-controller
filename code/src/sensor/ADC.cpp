#include "sensor/ADC.h"

using namespace team5;

ADC::ADC(uint8_t ADCSelPin, uint32_t SPI_clock, uint8_t SPI_bitOrder,
         uint8_t SPI_dataMode, String label) : ADCSelPin_(ADCSelPin),
                                 spiSettings_(SPISettings(SPI_clock, SPI_bitOrder, SPI_dataMode)),
                                 label_(label)
{
    pinMode(ADCSelPin_, OUTPUT);
    digitalWrite(ADCSelPin_, HIGH);
}

uint16_t ADC::readChannel(const uint8_t channel)
{
    // because the sample size is 12 bits, it throws everything
    // out of alignment so we have to bit fiddle
    uint8_t data[3] = {0, 0, 0};
    data[0] = 0x06; // start bit + single
    if (channel > 3)
    {
        data[0] |= 0x01;
    }
    if (channel)
    {
        data[1] |= (channel << 6);
    }

    digitalWrite(ADCSelPin_, LOW);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    for (uint8_t b = 0; b < 3; b++)
    {
        data[b] = SPI.transfer(data[b]);
    }
    digitalWrite(ADCSelPin_, HIGH);
    SPI.endTransaction();
    uint16_t out = ((256 * data[1] + data[2]) & 4095);

    return out;
}