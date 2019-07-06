/*
 * system.h
 *
 *  Created on: 06.07.2019
 *      Author: Marcin
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "gpio.h"

class System
{
public:
    ~System();
    static System& getInstance(void);
    void errorHandler(void) {};
    void config(void);
    void terminate(void);
    GPIO systemLED;
    GPIO systemPushbutton;
private:
    System();
    void configClock(void);
};

#endif /* SYSTEM_H_ */
