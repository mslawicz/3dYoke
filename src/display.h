/*
 * display.h
 *
 *  Created on: 06.07.2019
 *      Author: Marcin
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "spi.h"
#include "gpio.h"
#include "timer.h"
#include <queue>
#include <vector>
#include <utility>

#define DISPLAY_CD_PORT GPIOC
#define DISPLAY_CD_PIN  GPIO_PIN_11
#define DISPLAY_RESET_PORT GPIOC
#define DISPLAY_RESET_PIN  GPIO_PIN_14

enum DisplayState
{
    DS_start,
    DS_initialize,
    DS_wait,
    DS_send_loop
};

typedef std::pair<bool, std::vector<uint8_t>> DisplayContainer;

class Display : public SpiDevice
{
public:
    Display(SpiBus* pBus, GPIO_TypeDef* portCS, uint32_t pinCS);
    ~Display();
    void test(void);
    void handler(void);
private:
    void sendCommand(std::vector<uint8_t> commands) { dataQueue.push(DisplayContainer{true, commands}); }
    void sendData(std::vector<uint8_t> data) { dataQueue.push(DisplayContainer{false, data}); }
    void setColumn(uint8_t column) { sendCommand(std::vector<uint8_t>{static_cast<uint8_t>(column & 0x0F), static_cast<uint8_t>(0x10 | ((column >> 4) & 0x0F))}); }
    std::queue<DisplayContainer> dataQueue;
    GPIO commandData;
    GPIO reset;
    DisplayState state;
    Timer displayTimer;
    const uint32_t WaitForInitializationTime = 100000;
};

#endif /* DISPLAY_H_ */
