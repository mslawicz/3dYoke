/**
  ******************************************************************************
  * @file    main.c
  * @author  Marcin
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  * @project 3dYoke
  ******************************************************************************
*/


#include "stm32f4xx.h"
#include "system.h"
#include "gpio.h"
			

int main(void)
{
    System::getInstance().config();

    // start reception of the first character
    System::getInstance().getConsole()->getInterface().startReception();

    // send first prompt
    System::getInstance().getConsole()->sendPrompt();

    // main loop
    while(1)
    {
        System::getInstance().blinkLED();

        System::getInstance().getConsole()->handler();
        System::getInstance().getDisplay()->test();
        System::getInstance().getDisplay()->handler();
    }

    System::getInstance().terminate();
}
