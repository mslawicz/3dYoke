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

class Display : public SpiDevice
{
public:
    Display(SpiBus* pBus, GPIO_TypeDef* portCS, uint32_t pinCS);
    ~Display();
    void test(void);
    void handler(void);
private:
    std::queue<std::vector<uint8_t>> dataQueue;
};

#endif /* DISPLAY_H_ */
