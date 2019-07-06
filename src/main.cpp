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
#include "timer.h"
			

int main(void)
{
    System::getInstance().config();

    Timer ledTimer;

    // start reception of the first character
    //System::getInstance().getConsole()->getInterface().startReception();

    // send first prompt
    //System::getInstance().getConsole()->sendPrompt();

    // main loop
    while(1)
    {
        if(ledTimer.elapsed(500000))
        {
            System::getInstance().systemLED.toggle();
            ledTimer.reset();
        }

        //System::getInstance().getConsole()->handler();
    }

    System::getInstance().terminate();
}
