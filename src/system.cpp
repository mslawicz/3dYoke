/*
 * system.cpp
 *
 *  Created on: 06.07.2019
 *      Author: Marcin
 */

#include "system.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo.h"
#include "system.h"
#include "timer.h"
#include "display.h"

System::System() :
    systemLED(LED2_GPIO_PORT, LED2_PIN, GPIO_MODE_OUTPUT_PP),
    systemPushbutton(USER_BUTTON_GPIO_PORT, USER_BUTTON_PIN, GPIO_MODE_INPUT, GPIO_PULLUP)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* MSP (MCU Support Package) initialization */
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);

    /* Configure the system clock */
    configClock();

    pSpi3 = nullptr;
    pConsole = nullptr;
    pDisplay = nullptr;
}

System::~System()
{

}

/*
 * get the singleton instance
 * object constructor is executed in the first call
 */
System& System::getInstance(void)
{
    static System singleton;
    return singleton;
}

/*
 * configuration of system clocks
 */
void System::configClock(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      errorHandler();
    }
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        errorHandler();
    }
}

/*
 * user configuration
 */
void System::config(void)
{
    Timer::config();
    pConsole = new Console;
    pConsole->sendMessage(Severity::Info,LogChannel::LC_SYSTEM, "3d yoke program start");
    // SPI3 is used for display
    pSpi3 = new SpiBus(SPI3);
    pDisplay = new Display(SpiBus::pSpi3, DISPLAY_CS_PORT, DISPLAY_CS_PIN);
}

/*
 * clean up system before exit
 */
void System::terminate(void)
{
    delete pDisplay;
    delete pConsole;
    delete pSpi3;
}

/*
 * blinks the system LED in 1 second interval
 */
void System::blinkLED(void)
{
    static Timer ledTimer;
    if(ledTimer.elapsed(500000))
    {
        ledTimer.reset();
        systemLED.toggle();
    }
}
