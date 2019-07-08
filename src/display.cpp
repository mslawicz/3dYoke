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
    commandData(DISPLAY_CD_PORT, DISPLAY_CD_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH),
    reset(DISPLAY_RESET_PORT, DISPLAY_RESET_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH)
{
    state = DisplayState::DS_start;
}

Display::~Display()
{

}

void Display::test(void)
{
    static Timer timer;
    static uint8_t onOff = 0;
    return;
    if(timer.elapsed(100000))
    {
        timer.reset();
        sendCommand(std::vector<uint8_t>{static_cast<uint8_t>(0xA4 | (++onOff & 0x01))});
    }
}

/*
 * display handler to be executed periodically in a loop
 */
void Display::handler(void)
{
    switch(state)
    {
    case DS_start:
        reset.write(GPIO_PinState::GPIO_PIN_RESET);
        //send a dummy byte to get SPI ready
        send(std::vector<uint8_t>{0xAA});
        state = DS_reset_off;
        break;
    case DS_reset_off:
        //wait for SPI to be ready
        if(!pBus->isBusy())
        {
            unselect();
            reset.write(GPIO_PinState::GPIO_PIN_SET);
            displayTimer.reset();
            state = DS_wait_before_init;
        }
        break;
    case DS_wait_before_init:
        if(displayTimer.elapsed(WaitBeforeInitTime))
        {
            state = DS_initialize;
        }
        break;
    case DS_initialize:
        //display command is to be sent
        commandData.write(GPIO_PinState::GPIO_PIN_RESET);
        //send the initialization data
        send(initData);
        displayTimer.reset();
        state = DS_wait_after_init;
        break;
    case DS_wait_after_init:
        if(displayTimer.elapsed(WaitAfterInitTime))
        {
            state = DS_send_loop;
        }
        break;
    case DS_send_loop:
        if((!pBus->isBusy()) && (!dataQueue.empty()))
        {
            //queue is not empty and SPI is free
            // set command (0) or data (1) line
            commandData.write(dataQueue.front().first ? GPIO_PinState::GPIO_PIN_RESET : GPIO_PinState::GPIO_PIN_SET);
            // send bytes to display
            send(dataQueue.front().second);
            dataQueue.pop();
        }
        break;
    default:
        break;
    }
}
