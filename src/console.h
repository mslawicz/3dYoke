/*
 * console.h
 *
 *  Created on: 06.07.2019
 *      Author: Marcin
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "uart.h"

enum Severity
{
    Error,
    Warning,
    Info,
    Debug
};

enum LogChannel
{
    LC_SYSTEM,
    LC_CONSOLE,
    LC_I2C,
    LC_SPI,
    LC_END
};

class Console
{
public:
    Console();
    ~Console();
    UART& getInterface(void) { return interface; }
    void handler(void);
    void sendPrompt(void) { interface.send(">"); }
    void sendMessage(Severity level, LogChannel channel, std::string message);
    static std::string toHex(uint32_t value, uint8_t positions = 0, bool prefix = true);
    static const bool IsChannelActive[LogChannel::LC_END];
private:
    UART interface;
};

#endif /* CONSOLE_H_ */
