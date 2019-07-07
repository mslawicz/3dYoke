/*
 * display.cpp
 *
 *  Created on: 06.07.2019
 *      Author: Marcin
 */

#include "display.h"
#include "timer.h"
#include "gpio.h"

Display::Display(SpiBus* pBus, GPIO_TypeDef* portCS, uint32_t pinCS) :
    SpiDevice(pBus, portCS, pinCS),
    commandData(DISPLAY_CD_PORT, DISPLAY_CD_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH)
{

}

Display::~Display()
{

}

void Display::test(void)
{
    static Timer timer;
    if(timer.elapsed(2000))
    {
        timer.reset();
        dataQueue.push(std::vector<uint8_t>{0x12, 0x34, 0x56, 0x78, 0x98, 0x76, 0x54, 0x32, 0x10});
        dataQueue.push(std::vector<uint8_t>{0xab, 0xce});
        dataQueue.push(std::vector<uint8_t>{0x98, 0x76, 0x54, 0x32, 0x10});
    }
}

/*
 * display handler to be executed periodically in a loop
 */
void Display::handler(void)
{
    if((!pBus->isBusy()) && (!dataQueue.empty()))
    {
        // transmit another part of display data
        send(dataQueue.front());
        dataQueue.pop();
    }
}
