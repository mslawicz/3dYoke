/*
 * display.cpp
 *
 *  Created on: 06.07.2019
 *      Author: Marcin
 */

#include "display.h"
#include "timer.h"

Display::Display(SpiBus* pBus, GPIO_TypeDef* portCS, uint32_t pinCS) :
    SpiDevice(pBus, portCS, pinCS)
{

}

Display::~Display()
{

}

void Display::test(void)
{
    static Timer timer;
    if(timer.elapsed(10000))
    {
        timer.reset();
        send(std::vector<uint8_t>{0x12, 0x34, 0x56, 0x78});
    }
}
