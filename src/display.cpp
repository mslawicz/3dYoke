/*
 * display.cpp
 *
 *  Created on: 06.07.2019
 *      Author: Marcin
 */

#include "display.h"
#include "timer.h"
#include "console.h"//XXX
#include "system.h"//XXX

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
    if(timer.elapsed(100000))
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
    static bool bs = false;
    if(bs != pBus->isBusy())
    {
        bs = pBus->isBusy();
        System::getInstance().getConsole()->sendMessage(Severity::Info, LogChannel::LC_SPI, "busy=" + Console::toHex((int)bs));//XXX
    }
    if((!pBus->isBusy()) && (!dataQueue.empty()))
    {
        System::getInstance().getConsole()->sendMessage(Severity::Info, LogChannel::LC_SPI, "queue size=" + Console::toHex(dataQueue.size()));//XXX
        // transmit another part of display data
        send(dataQueue.front());
        dataQueue.pop();
    }
}
