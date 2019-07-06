/*
 * system.h
 *
 *  Created on: 06.07.2019
 *      Author: Marcin
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "gpio.h"
#include "console.h"

class System
{
public:
    ~System();
    static System& getInstance(void);
    void errorHandler(void) {};
    void config(void);
    void terminate(void);
    void blinkLED(void);
    Console* getConsole(void) const { return pConsole; }
private:
    System();
    void configClock(void);
    GPIO systemLED;
    GPIO systemPushbutton;
    Console* pConsole;
};

#endif /* SYSTEM_H_ */
