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
#include <queue>
#include <vector>
#include <utility>

#define DISPLAY_CD_PORT GPIOC
#define DISPLAY_CD_PIN  GPIO_PIN_11

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
    std::queue<DisplayContainer> dataQueue;
    GPIO commandData;
};

#endif /* DISPLAY_H_ */
