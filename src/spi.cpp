/*
 * spi.cpp
 *
 *  Created on: 06.07.2019
 *      Author: Marcin
 */

#include "spi.h"
#include "system.h"

SpiBus* SpiBus::pSpi3 = nullptr;

SpiBus::SpiBus(SPI_TypeDef* instance) :
    instance(instance)
{
    std::string name;
    if(instance == SPI3)
    {
        name = "SPI3";

        // MOSI pin
        GPIO(GPIOC, GPIO_PIN_12, GPIO_MODE_AF_PP, GPIO_PULLDOWN, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF6_SPI3);
        // SCK pin
        GPIO(GPIOC, GPIO_PIN_10, GPIO_MODE_AF_PP, GPIO_PULLDOWN, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF6_SPI3);
        /* Peripheral clock enable */
        __HAL_RCC_SPI3_CLK_ENABLE();
        /* Peripheral interrupt init */
        HAL_NVIC_SetPriority(SPI3_IRQn, 1, 1);
        HAL_NVIC_EnableIRQ(SPI3_IRQn);

        /* DMA interrupt init */
        /* DMA1_Stream5_IRQn interrupt configuration */
        HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

        /* SPI3_TX Init */
        hDmaTx.Instance = DMA1_Stream5;
        hDmaTx.Init.Channel = DMA_CHANNEL_0;
        hDmaTx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hDmaTx.Init.PeriphInc = DMA_PINC_DISABLE;
        hDmaTx.Init.MemInc = DMA_MINC_ENABLE;
        hDmaTx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hDmaTx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hDmaTx.Init.Mode = DMA_NORMAL;
        hDmaTx.Init.Priority = DMA_PRIORITY_LOW;
        hDmaTx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hDmaTx) != HAL_OK)
        {
            System::getInstance().getConsole()->sendMessage(Severity::Error, LogChannel::LC_SPI, name + " TX DMA initialization failed");
        }
        __HAL_LINKDMA(&hSpi,hdmatx,hDmaTx);

        pSpi3 = this;
    }
    hSpi.Instance = instance;
    hSpi.Init.Mode = SPI_MODE_MASTER;
    hSpi.Init.Direction = SPI_DIRECTION_2LINES;
    hSpi.Init.DataSize = SPI_DATASIZE_8BIT;
    hSpi.Init.CLKPolarity = SPI_POLARITY_LOW;
    hSpi.Init.CLKPhase = SPI_PHASE_1EDGE;
    hSpi.Init.NSS = SPI_NSS_SOFT;
    hSpi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;    // clock 666 kHz
    hSpi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hSpi.Init.TIMode = SPI_TIMODE_DISABLE;
    hSpi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hSpi.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hSpi) == HAL_OK)
    {
        System::getInstance().getConsole()->sendMessage(Severity::Info, LogChannel::LC_SPI, name + " initialized");
    }
    else
    {
        System::getInstance().getConsole()->sendMessage(Severity::Error, LogChannel::LC_SPI, name + " initialization failed");
    }
    busy = false;
    pLastServedDevice = nullptr;
    System::getInstance().testPin.write(GPIO_PinState::GPIO_PIN_RESET);//XXX
}

SpiBus::~SpiBus()
{
    // TODO Auto-generated destructor stub
}

/*
 * mark the SPI bus inactive and release chip select of the current device
 */
void SpiBus::markAsFree(void)
{
    busy = false;
    System::getInstance().testPin.write(GPIO_PinState::GPIO_PIN_RESET);//XXX
    if((pLastServedDevice != nullptr) && (pLastServedDevice->autoCS))
    {
        // current served device chip select inactive
        pLastServedDevice->chipSelect.write(GPIO_PinState::GPIO_PIN_SET);
    }
}


SpiDevice::SpiDevice(SpiBus* pBus, GPIO_TypeDef* portCS, uint32_t pinCS, bool autoCS) :
        pBus(pBus),
        chipSelect(portCS, pinCS, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH),
        autoCS(autoCS)
{
    System::getInstance().getConsole()->sendMessage(Severity::Info, LogChannel::LC_SPI, "SPI device created, CS=" + Console::toHex(reinterpret_cast<uint32_t>(portCS)) + "/" + Console::toHex(pinCS));

    chipSelect.write(GPIO_PinState::GPIO_PIN_SET);
    newDataReady = false;
}

SpiDevice::~SpiDevice() {}

/*
 * send data to SPI device
 */
void SpiDevice::send(std::vector<uint8_t> data)
{
    dataToSend = data;
    if(autoCS)
    {
        chipSelect.write(GPIO_PinState::GPIO_PIN_RESET);
    }
    pBus->markAsBusy();
    if(HAL_SPI_Transmit_DMA(pBus->getHandle(), &dataToSend[0], dataToSend.size()) == HAL_OK)
    {
        pBus->pLastServedDevice = this;
    }
    else
    {
        // no transmission started
        chipSelect.write(GPIO_PinState::GPIO_PIN_SET);
        pBus->pLastServedDevice = nullptr;
        pBus->markAsFree();
    }
}

/*
 * receive data from SPI device
 */
void SpiDevice::receiveRequest(uint16_t size)
{
    receptionBuffer.assign(size, 0);
    if(autoCS)
    {
        chipSelect.write(GPIO_PinState::GPIO_PIN_RESET);
    }
    if(HAL_SPI_Receive_DMA(pBus->getHandle(), &receptionBuffer[0], size) == HAL_OK)
    {
        pBus->markAsBusy();
        pBus->pLastServedDevice = this;
    }
    else
    {
        // no reception started
        chipSelect.write(GPIO_PinState::GPIO_PIN_SET);
        pBus->pLastServedDevice = nullptr;
    }
}

/*
 * bidirectional transmit to/from SPI device
 */
void SpiDevice::sendReceiveRequest(std::vector<uint8_t> data)
{
    dataToSend = data;
    receptionBuffer.assign(data.size(), 0);
    if(autoCS)
    {
        chipSelect.write(GPIO_PinState::GPIO_PIN_RESET);
    }
    if(HAL_SPI_TransmitReceive_DMA(pBus->getHandle(), &dataToSend[0], &receptionBuffer[0], data.size()) == HAL_OK)
    {
        pBus->markAsBusy();
        pBus->pLastServedDevice = this;
    }
    else
    {
        // no reception started
        chipSelect.write(GPIO_PinState::GPIO_PIN_SET);
        pBus->pLastServedDevice = nullptr;
    }
}

/**
  * @brief  Tx Transfer completed callback.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi->Instance == SPI3)
    {
        // mark this SPI bus as free
        SpiBus::pSpi3->markAsFree();
    }
}


/**
  * @brief  Rx Transfer completed callback.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
}

/**
  * @brief  Tx and Rx Transfer completed callback.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
}

/*
 * mark that new received data is ready in the device buffer
 */
void SpiBus::markNewDataReady(void)
{
    if(pLastServedDevice != nullptr)
    {
        pLastServedDevice->newDataReady = true;
    }
}

/**
  * @brief  SPI error callback.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    System::getInstance().getConsole()->sendMessage(Severity::Error, LogChannel::LC_SPI, " SPI error code=" + std::to_string(HAL_SPI_GetError(hspi)));
}

//XXX
void SpiBus::markAsBusy(void)
{
    busy = true;
    System::getInstance().testPin.write(GPIO_PinState::GPIO_PIN_SET);
}
