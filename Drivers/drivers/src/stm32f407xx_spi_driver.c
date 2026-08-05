#include "stm32f407xx_spi_driver.h"

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle);

void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{

    if (EnorDi == ENABLE)
    {
        if (pSPIx == SPI1)
        {
            SPI1_PCLK_EN();
        }
        else if (pSPIx == SPI2)
        {
            SPI2_PCLK_EN();
        }
        else if (pSPIx == SPI3)
        {
            SPI3_PCLK_EN();
        }
    }
    else
    {
        if (pSPIx == SPI1)
        {
            SPI1_PCLK_DI();
        }
        else if (pSPIx == SPI2)
        {
            SPI2_PCLK_DI();
        }
        else if (pSPIx == SPI3)
        {
            SPI3_PCLK_DI();
        }
    }
}

void SPI_Init(SPI_Handle_t *pSPIHandle)
{
    /* configure SPI_CR1 register */
    uint32_t tempreg = 0;

    /* configure device mode */
    tempreg |= pSPIHandle->SPIConfig.SPI_DeviceMode << 2;

    /* configure the bus config */
    if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_FD)
    {
        /* bidi mode should be cleared */
        tempreg &= ~(1 << 15);
    }
    else if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_HD)
    {
        /* bidi mode should be set */
        tempreg |= (1 << 15);
    }
    else if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_RXONLY)
    {
        /* bidi mode should be cleared */
        tempreg &= ~(1 << 15);
        /* RCONLY bit should be set */
        tempreg |= (1 << 10);
    }
    /* Configure SPI serial clock speed */
    tempreg |= pSPIHandle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR;

    /* Configure DFF */
    tempreg |= pSPIHandle->SPIConfig.SPI_DFF << SPI_CR1_DFF;

    /* configure CPOL */
    tempreg |= pSPIHandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL;

    /* configure CPHA */
    tempreg |= pSPIHandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA;

    tempreg |= pSPIHandle->SPIConfig.SPI_SSM << SPI_CR1_SSM;

    pSPIHandle->pSPIx = tempreg;
}
void SPI_DeInit(SPI_RegDef_t *pSPIx)
{
    if (pSPIx == SPI1)
    {
        SPI1_REG_RESET();
    }
    else if (pSPIx == SPI2)
    {
        SPI2_REG_RESET();
    }
    else if (pSPIx == SPI3)
    {
        SPI3_REG_RESET();
    }
}

uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName)
{
    if (pSPIx->SR & FlagName)
    {
        return FLAG_SET;
    }
    return FLAG_RESET;
}

/* Blocking call for sendData */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len)
{
    while (Len > 0)
    {
        /* Wait TXE(transmit buffer empty) is set */
        while (SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET)
        {
        }

        /* check DFF bit in CR */
        if (pSPIx->CR1 & (1 << SPI_CR1_DFF))
        {
            /* 16 bit DFF */
            pSPIx->DR = *((uint16_t *)pTxBuffer);
            Len--;
            Len--;
            (uint16_t *)pTxBuffer++;
        }
        else
        {
            /* 8 bit */
            pSPIx->DR = *pTxBuffer;
            Len--;
            pTxBuffer++;
        }
    }
}

/* Blocking call for RecvData */
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len)
{
    while (Len > 0)
    {
        /* Wait TXE(transmit buffer empty) is set */
        while (SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET)
        {
        }

        /* check DFF bit in CR */
        if (pSPIx->CR1 & (1 << SPI_CR1_DFF))
        {
            /* 16 bit DFF */
            *((uint16_t *)pRxBuffer) = pSPIx->DR;
            Len--;
            Len--;
            (uint16_t *)pRxBuffer++;
        }
        else
        {
            /* 8 bit */
            *(pRxBuffer) = pSPIx->DR;
            Len--;
            pRxBuffer++;
        }
    }
}

void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{

    if (EnorDi == ENABLE)
    {
        if (IRQNumber <= 31)
        {
            /* program ISER0 register */
            *NVIC_ISER0 |= (1 << IRQNumber);
        }
        else if (IRQNumber > 31 && IRQNumber < 64) // 32 to 63
        {
            /* program ISER1 register */
            *NVIC_ISER1 |= (1 << (IRQNumber % 32));
        }
        else if (IRQNumber >= 64 && IRQNumber < 96)
        {
            /* program ISER2 register */
            *NVIC_ISER3 |= (1 << (IRQNumber % 64));
        }
    }
    else
    {
        if (IRQNumber <= 31)
        {
            /* ICER0 register */
            *NVIC_ICER0 |= (1 << IRQNumber);
        }
        else if (IRQNumber > 31 && IRQNumber < 64)
        {
            /* ICER1 register */
            *NVIC_ICER1 |= (1 << (IRQNumber % 32));
        }
        else if (IRQNumber >= 64 && IRQNumber < 96)
        {
            /* ICER2 register */
            *NVIC_ICER3 |= (1 << (IRQNumber % 64));
        }
    }
}

void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
    /* ipr register */
    uint8_t iprx = IRQNumber / 4;
    uint8_t iprx_section = IRQNumber % 4;

    uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);

    *(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);
}

void SPI_CloseTransmisson(SPI_Handle_t *pSPIHandle)
{
    pSPIHandle->pSPIx->CR2 &= ~(1 << SPI_CR2_TXEIE);
    pSPIHandle->pTxBuffer = NULL;
    pSPIHandle->TxLen = 0;
    pSPIHandle->TxState = SPI_READY;
}

void SPI_CloseReception(SPI_Handle_t *pSPIHandle)
{
    pSPIHandle->pSPIx->CR2 &= ~(1 << SPI_CR2_RXNEIE);
    pSPIHandle->pRxBuffer = NULL;
    pSPIHandle->RxLen = 0;
    pSPIHandle->RxState = SPI_READY;
}

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx)
{
    uint8_t temp;
    temp = pSPIx->DR;
    temp = pSPIx->SR;
}

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    /* check the DFF bit in CR1 */
    if ((pSPIHandle->pSPIx->CR1 & (1 << SPI_CR1_DFF)))
    {
        /* 16 bit DFF load the data in to the DR */
        pSPIHandle->pSPIx->DR = *((uint16_t *)pSPIHandle->pTxBuffer);
        pSPIHandle->TxLen--;
        pSPIHandle->TxLen--;
        (uint16_t *)pSPIHandle->pTxBuffer++;
    }
    else
    {
        /* 8 bit DFF */
        pSPIHandle->pSPIx->DR = *pSPIHandle->pTxBuffer;
        pSPIHandle->TxLen--;
        pSPIHandle->pTxBuffer++;
    }

    if (!pSPIHandle->TxLen)
    {
        /* TxLen is zero , so close the spi transmission and inform the application that TX is over. */
        /* this prevents interrupts from setting up of TXE flag */
        SPI_CloseTransmisson(pSPIHandle);
        SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_TX_COMPLETE);
    }
}
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    if (pSPIHandle->pSPIx->CR1 & (1 << SPI_CR1_DFF))
    {
        *((uint16_t *)pSPIHandle->pRxBuffer) = (uint16_t)pSPIHandle->pSPIx->DR;
        pSPIHandle->RxLen -= 2;
        pSPIHandle->pRxBuffer++;
        pSPIHandle->pRxBuffer++;
    }
    else
    {
        *(pSPIHandle->pRxBuffer) = (uint8_t)pSPIHandle->pSPIx->DR;
        pSPIHandle->RxLen--;
        pSPIHandle->pRxBuffer++;
    }

    if (!pSPIHandle->RxLen)
    {
        SPI_CloseReception(pSPIHandle);
        SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_RX_COMPLETE);
    }
}

static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle)
{

    uint8_t temp;
    /* clear the ovr flag */
    if (pSPIHandle->TxState != SPI_BUSY_IN_TX)
    {
        temp = pSPIHandle->pSPIx->DR;
        temp = pSPIHandle->pSPIx->SR;
    }
    /* inform the application */
    SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_OVR_ERR);
}

void SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len)
{
    uint8_t state = pSPIHandle->TxState;

    if (state != SPI_BUSY_IN_TX)
    {
        /* Save the Tx buffer address and Len information in some global variables */
        pSPIHandle->pTxBuffer = pTxBuffer;
        pSPIHandle->TxLen = Len;
        /* Mark SPI state as in transmission so that no other code can take over same SPI peripheral until transmission is over */
        pSPIHandle->TxState = SPI_BUSY_IN_TX;
        /* Enable TXEIE control bit to get interrupt whenever TXE flag is set in SR */
        pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_TXEIE);
    }
    return state;
}
void SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len)
{
    uint8_t state = pSPIHandle->RxState;

    if (state != SPI_BUSY_IN_RX)
    {
        /* Save the Rx buffer address and Len information in some global variables */
        pSPIHandle->pRxBuffer = pRxBuffer;
        pSPIHandle->RxLen = Len;
        /* Mark SPI state as in reception so that no other code can take over same SPI peripheral until reception is over */
        pSPIHandle->RxState = SPI_BUSY_IN_RX;
        /* Enable RxEIE control bit to get interrupt whenever RxE flag is set in SR */
        pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_RXNEIE);
    }
    return state;
}

void SPI_IRQHandling(SPI_Handle_t *pHandle)
{
    uint8_t temp1, temp2;

    /* Check for TXE */
    temp1 = pHandle->pSPIx->SR & (1 << SPI_SR_TXE);
    temp2 = pHandle->pSPIx->CR2 & (1 << SPI_CR2_TXEIE);

    /* has enabled TXEIE and TXE has been set */
    if (temp1 && temp2)
    {
        /* handle TXE */
        spi_txe_interrupt_handle(pHandle);
    }

    /* check for RXNE */
    temp1 = pHandle->pSPIx->SR & (1 << SPI_SR_RXNE);
    temp2 = pHandle->pSPIx->CR2 & (1 << SPI_CR2_RXNEIE);

    if (temp1 && temp2)
    {
        /* handle RXNE */
        spi_rxne_interrupt_handle(pHandle);
    }

    /* check for OVR */
    temp1 = pHandle->pSPIx->SR & (1 << SPI_SR_OVR);
    temp2 = pHandle->pSPIx->CR2 & (1 << SPI_CR2_ERRIE);

    /* check for OVR flag */
    if (temp1 && temp2)
    {
        /* handle OVR */
        spi_ovr_err_interrupt_handle(pHandle);
    }

    return;
}

__weak void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv)
{
    /* This is a weak implementation */
    /* User application may override this function. */
}