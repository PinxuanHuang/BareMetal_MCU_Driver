#include "stm32f407xx_i2c_driver.h"

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);
void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);
static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle);

static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle);
static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle);

static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
    /* 7bit */
    SlaveAddr = SlaveAddr << 1;

    /* 1 bit as w */
    SlaveAddr &= ~(1);

    /* set into data register */
    pI2Cx->DR = SlaveAddr;
}

static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
    /* 7bit */
    SlaveAddr = SlaveAddr << 1;

    /* 1 bit as r */
    SlaveAddr |= 1;

    /* set into data register */
    pI2Cx->DR = SlaveAddr;
}

static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle)
{
    uint32_t dummy_read;
    // check for device mode
    if (pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_MSL))
    {
        // device is in master mode
        if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
        {
            if (pI2CHandle->RxSize == 1)
            {
                // first disable the ack
                I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);

                /* clear the ADDR flag ( read SR1 , read SR2) */
                dummy_read = pI2CHandle->pI2Cx->SR1;
                dummy_read = pI2CHandle->pI2Cx->SR2;
            }
        }
        else
        {
            /* clear the ADDR flag ( read SR1 , read SR2) */
            dummy_read = pI2CHandle->pI2Cx->SR1;
            dummy_read = pI2CHandle->pI2Cx->SR2;
        }
    }
    else
    {
        /* device is in slave mode, clear the ADDR flag ( read SR1 , read SR2) */
        dummy_read = pI2CHandle->pI2Cx->SR1;
        dummy_read = pI2CHandle->pI2Cx->SR2;
    }
}

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx)
{
    pI2Cx->CR1 |= (1 << I2C_CR1_START);
}

void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx)
{
    pI2Cx->CR1 |= (1 << I2C_CR1_STOP);
}

void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle)
{
    /* disable ITBUFEN Control Bit */
    pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);

    /* disable ITEVFEN Control Bit */
    pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);

    pI2CHandle->TxRxState = I2C_READY;
    pI2CHandle->pRxBuffer = NULL;
    pI2CHandle->RxLen = 0;
    pI2CHandle->RxSize = 0;

    if (pI2CHandle->I2C_Config.I2C_AckControl == I2C_ACK_ENABLE)
    {
        I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
    }
}

void I2C_CloseSendData(I2C_Handle_t *pI2CHandle)
{
    /* disable ITBUFEN Control Bit */
    pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);

    /* disable ITEVFEN Control Bit */
    pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);

    pI2CHandle->TxRxState = I2C_READY;
    pI2CHandle->pTxBuffer = NULL;
    pI2CHandle->TxLen = 0;
}

void I2C_SlaveEnableDisableCallbackEvents(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
    if (EnorDi == ENABLE)
    {
        pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);
        pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);
        pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);
    }
    else
    {
        pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);
        pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);
        pI2Cx->CR2 &= ~(1 << I2C_CR2_ITERREN);
    }
}

void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        pI2Cx->CR1 |= (1 << I2C_CR1_PE);
        /* pI2cBaseAddress->CR1 |= I2C_CR1_PE_Bit_Mask; */
    }
    else
    {
        pI2Cx->CR1 &= ~(1 << 0);
    }
}

void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
    if (EnorDi == ENABLE)
    {
        if (pI2Cx == I2C1)
        {
            I2C1_PCLK_EN();
        }
        else if (pI2Cx == I2C2)
        {
            I2C2_PCLK_EN();
        }
        else if (pI2Cx == I2C3)
        {
            I2C3_PCLK_EN();
        }
    }
    else
    {
        if (pI2Cx == I2C1)
        {
            I2C1_PCLK_DI();
        }
        else if (pI2Cx == I2C2)
        {
            I2C2_PCLK_DI();
        }
        else if (pI2Cx == I2C3)
        {
            I2C3_PCLK_DI();
        }
    }
}

uint16_t AHB_PreScaler[8] = {2, 4, 8, 16, 64, 128, 256, 512};
uint8_t APB1_PreScaler[4] = {2, 4, 8, 16};

uint32_t RCC_GetPLLOutputClock()
{
    /* Useless */
    return 0;
}

uint32_t RCC_GetPCLK1Value(void)
{
    uint32_t pclk1, SystemClk;

    uint8_t clksrc, temp, ahbp, apb1p;

    clksrc = ((RCC->CFGR >> 2) & 0x3);

    if (clksrc == 0)
    {
        SystemClk = 16000000;
    }
    else if (clksrc == 1)
    {
        SystemClk = 8000000;
    }
    else if (clksrc == 2)
    {
        SystemClk = RCC_GetPLLOutputClock();
    }

    /* For AHB */
    temp = ((RCC->CFGR >> 4) & 0xF);

    if (temp < 8)
    {
        ahbp = 1;
    }
    else
    {
        ahbp = AHB_PreScaler[temp - 8];
    }

    /* For APB1 */
    temp = ((RCC->CFGR >> 10) & 0x7);

    if (temp < 4)
    {
        apb1p = 1;
    }
    else
    {
        apb1p = APB1_PreScaler[temp - 4];
    }

    pclk1 = (SystemClk / ahbp) / apb1p;

    return pclk1;
}

void I2C_Init(I2C_Handle_t *pI2CHandle)
{
    uint8_t trise;
    uint32_t tempreg = 0;

    /* ACK control bit */
    tempreg |= pI2CHandle->I2C_Config.I2C_AckControl << 10;
    pI2CHandle->pI2Cx->CR1 = tempreg;

    /* Configure the FREQ field of CR2 */
    tempreg = 0;
    tempreg |= RCC_GetPCLK1Value() / 1000000U;
    pI2CHandle->pI2Cx->CR2 = (tempreg & 0x3F);

    /* Device Address */
    tempreg |= pI2CHandle->I2C_Config.I2C_DeviceAddress << 1;
    tempreg |= (1 << 14);
    pI2CHandle->pI2Cx->OAR1 = tempreg;

    /* CCR calculation */
    uint16_t ccr_value = 0;
    tempreg = 0;
    if (pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
    {
        /* Standard Mode */
        ccr_value = (RCC_GetPCLK1Value() / (2 * pI2CHandle->I2C_Config.I2C_SCLSpeed));
        tempreg |= (ccr_value & 0xFFF);
    }
    else
    {
        /* Fast Mode*/
        tempreg |= (1 << 15);
        tempreg |= (pI2CHandle->I2C_Config.I2C_FMDutyCycle << 14);
        if (pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_2)
        {
            ccr_value = (RCC_GetPCLK1Value() / (3 * pI2CHandle->I2C_Config.I2C_SCLSpeed));
        }
        else
        {
            ccr_value = (RCC_GetPCLK1Value() / (25 * pI2CHandle->I2C_Config.I2C_SCLSpeed));
        }
        tempreg |= (ccr_value & 0xFFF);
    }
    pI2CHandle->pI2Cx->CCR = tempreg;

    /* TRISE */
    if (pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
    {
        /* Standard Mode */
        trise = (RCC_GetPCLK1Value() / 1000000U) + 1;
    }
    else
    {
        /* Fast Mode*/
        trise = ((RCC_GetPCLK1Value() * 300) / 1000000000U) + 1;
    }
    pI2CHandle->pI2Cx->TRISE = (tempreg & 0x3F);
}

void I2C_DeInit(I2C_RegDef_t *pI2Cx)
{
    if (pI2Cx == I2C1)
    {
        I2C1_REG_RESET();
    }
    else if (pI2Cx == I2C2)
    {
        I2C2_REG_RESET();
    }
    else if (pI2Cx == I2C3)
    {
        I2C3_REG_RESET();
    }
}

void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{

    if (EnorDi == ENABLE)
    {
        if (IRQNumber <= 31)
        {
            /* ISER0 register */
            *NVIC_ISER0 |= (1 << IRQNumber);
        }
        else if (IRQNumber > 31 && IRQNumber < 64)
        {
            /* ISER1 register */
            *NVIC_ISER1 |= (1 << (IRQNumber % 32));
        }
        else if (IRQNumber >= 64 && IRQNumber < 96)
        {
            /* ISER2 register */
            /* 64 to 95 */
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

void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
    /* ipr register */
    uint8_t iprx = IRQNumber / 4;
    uint8_t iprx_section = IRQNumber % 4;

    uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);

    *(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);
}

uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName)
{
    if (pI2Cx->SR1 & FlagName)
    {
        return FLAG_SET;
    }
    return FLAG_RESET;
}

void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr)
{
    /* I2C start condition */
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

    /* Check SB flag in SR1 */
    /* Until SB is cleared, SCL will be stretched (pull to low) */
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB))
    {
    }

    /* Send Address of slave and rw bit */
    I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, SlaveAddr);

    /* Confirm that address phase is completed by checking ADDR flag in SR1 */
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR))
    {
    }

    /* Clear ADDR flag according to its sw sequence */
    /* Until ADDR is cleared, SCL will be stretched (pull to low) */
    I2C_ClearADDRFlag(pI2CHandle);

    /* Send data until len becomes 0 */
    while (Len > 0)
    {
        while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE))
        {
        }
        pI2CHandle->pI2Cx->DR = *pTxBuffer;
        pTxBuffer++;
        Len--;
    }

    /* when Len becomes zero wait for TXE=1 and BTF=1 before generating the STOP condition */
    /* TXE=1 , BTF=1 , means that both SR and DR are empty and next transmission should begin. When BTF=1, SCL will be stretched (pulled to low) */
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE))
    {
    }

    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF))
    {
    }

    /* Generate STOP condition and master need not to wait for the completion of stop condition. */
    /* Generating STOP, automatically clears the BTF */
    if (Sr == I2C_DISABLE_SR)
        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
}

void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t Sr)
{
    /* I2C start condition */
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

    /* Check SB flag in SR1 */
    /* Until SB is cleared, SCL will be stretched (pull to low) */
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB))
    {
    }

    /* Send Address of slave and rw bit */
    I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, SlaveAddr);

    /* Confirm that address phase is completed by checking ADDR flag in SR1 */
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR))
    {
    }

    /* read only 1 byte from slave */
    if (Len == 1)
    {
        /* Disable Acking */
        I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

        /* clear ADDR flag */
        I2C_ClearADDRFlag(pI2CHandle);

        /* wait until RXNE becomes 1 */
        while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE))
        {
        }

        /* generate STOP condition */
        if (Sr == I2C_DISABLE_SR)
            I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

        /* read data into buffer*/
        *pRxBuffer = pI2CHandle->pI2Cx->DR;
    }

    /* read more than 1 byte */
    if (Len > 1)
    {
        /* clear ADDR flag */
        I2C_ClearADDRFlag(pI2CHandle->pI2Cx);

        /* read data until Len becomes 0 */
        for (uint32_t i = Len; i > 0; i--)
        {
            /* wait until RXNE become 1 */
            while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE))
            {
            }

            if (i == 2)
            {
                /* clear ACK bit */
                I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

                /* generate stop condition */
                I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
            }

            /* read data into buffer*/
            *pRxBuffer = pI2CHandle->pI2Cx->DR;

            /* increment buffer address */
            pRxBuffer++;
        }
    }
    /* Enable ACKING again at the end */
    if (pI2CHandle->I2C_Config.I2C_AckControl == I2C_ACK_ENABLE)
    {
        I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_ENABLE);
    }
}

void I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
    if (EnorDi == I2C_ACK_ENABLE)
    {
        // enable the ack
        pI2Cx->CR1 |= (1 << I2C_CR1_ACK);
    }
    else
    {
        // disable the ack
        pI2Cx->CR1 &= ~(1 << I2C_CR1_ACK);
    }
}

uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr)
{
    uint8_t busystate = pI2CHandle->TxRxState;

    if ((busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
    {
        pI2CHandle->pTxBuffer = pTxBuffer;
        pI2CHandle->TxLen = Len;
        pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
        pI2CHandle->DevAddr = SlaveAddr;
        pI2CHandle->Sr = Sr;

        /* Generate start condition */
        I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

        /* enable ITBUFEN Control Bit */
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);

        /* enable ITEVFEN Control Bit */
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);

        /* enable ITERREN Control Bit */
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);
    }
    return busystate;
}
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t Sr)
{
    uint8_t busystate = pI2CHandle->TxRxState;

    if ((busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
    {
        pI2CHandle->pRxBuffer = pRxBuffer;
        pI2CHandle->TxLen = Len;
        pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
        pI2CHandle->DevAddr = SlaveAddr;
        pI2CHandle->Sr = Sr;

        /* Generate start condition */
        I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

        /* enable ITBUFEN Control Bit */
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);

        /* enable ITEVFEN Control Bit */
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);

        /* enable ITERREN Control Bit */
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);
    }
    return busystate;
}

void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle)
{
    /* Interrupt handling for both master and slave */
    uint32_t temp1, temp2, temp3;
    temp1 = pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITEVTEN);
    temp2 = pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITBUFEN);
    temp3 = pI2CHandle->pI2Cx->CR2 & (1 << I2C_SR1_SB);

    /* handle interrupt generated by SB event */
    if (temp1 && temp3)
    {
        /* It'll only executed in master mode because for slave mode SB is always 0 */
        if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
        {
            I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
        }
        else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
        {
            I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
        }
    }

    temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_ADDR);
    /* handle interrupt generated by ADDR event */
    if (temp1 && temp3)
    {
        I2C_ClearADDRFlag(pI2CHandle->pI2Cx);
    }

    temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_BTF);
    /* handle interrupt generated by BTF event */
    if (temp1 && temp3)
    {
        if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
        {
            /* check TXE is also set */
            if (pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_TXE))
            {
                /* BTF = 1 & TXE = 1 */
                if (pI2CHandle->TxLen == 0)
                {
                    /* generate stop condition */
                    if (pI2CHandle->Sr == I2C_DISABLE_SR)
                        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

                    /* reset all members in handle struct */
                    I2C_CloseSendData(pI2CHandle);

                    /* notify the application about transmission complete */
                    I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);
                }
            }
        }
        else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
        {
        }
    }

    temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_STOPF);
    /* handle interrupt generated by STOPF event */
    if (temp1 && temp3)
    {
        /* Clear STOPF, read SR1, write CR1 */
        pI2CHandle->pI2Cx->CR1 |= 0x0000;

        /* Notify the application that STOP is detected */
        I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_STOP);
    }

    temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_TXE);
    /* handle interrupt generated by TXE event */
    if (temp1 && temp2 && temp3)
    {
        /* check device mode */
        if (pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_MSL))
        {
            /* master mode */
            if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
            {
                if (pI2CHandle->TxLen > 0)
                {
                    /* load data into DR */
                    pI2CHandle->pI2Cx->DR = *(pI2CHandle->pTxBuffer);

                    /* decrement TxLen */
                    pI2CHandle->TxLen--;

                    /* Increment buffer address */
                    pI2CHandle->pTxBuffer++;
                }
            }
        }
        else
        {
            /* slave mode*/
            if (pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_TRA))
            {
                I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_REQ);
            }
        }
    }

    temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_RXNE);
    /* handle interrupt generated by RXNE event */
    if (temp1 && temp2 && temp3)
    {
        if (pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_MSL))
        {
            /* only master device will do the following actions */
            if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
            {
                if (pI2CHandle->RxSize == 1)
                {
                    *pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->DR;
                    pI2CHandle->RxLen--;
                }

                /* read more than 1 byte */
                if (pI2CHandle->RxSize > 1)
                {
                    if (pI2CHandle->RxLen == 2)
                    {
                        I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);
                    }
                    *pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->DR;
                    pI2CHandle->pRxBuffer++;
                    pI2CHandle->RxLen--;
                }

                if (pI2CHandle->RxLen == 0)
                {
                    if (pI2CHandle->Sr == I2C_DISABLE_SR)
                        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

                    I2C_CloseReceiveData(pI2CHandle);

                    I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_RX_CMPLT);
                }
            }
        }
        else
        {
            /* slave mode*/
            if (!(pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_TRA)))
            {
                I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_RCV);
            }
        }
    }
}

void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle)
{
    uint32_t temp1, temp2;

    /* Know the status of  ITERREN control bit in the CR2 */
    temp2 = (pI2CHandle->pI2Cx->CR2) & (1 << I2C_CR2_ITERREN);

    /* Check for Bus error */
    temp1 = (pI2CHandle->pI2Cx->SR1) & (1 << I2C_SR1_BERR);
    if (temp1 && temp2)
    {
        /* Clear the buss error flag */
        pI2CHandle->pI2Cx->SR1 &= ~(1 << I2C_SR1_BERR);

        /* Notify the application about the error */
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_BERR);
    }

    /* Check for arbitration lost error */
    temp1 = (pI2CHandle->pI2Cx->SR1) & (1 << I2C_SR1_ARLO);
    if (temp1 && temp2)
    {
        /* Clear the arbitration lost error flag */
        pI2CHandle->pI2Cx->SR1 &= ~(1 << I2C_SR1_ARLO);

        /* Notify the application about the error */
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_ARLO);
    }

    /* Check for ACK failure  error */
    temp1 = (pI2CHandle->pI2Cx->SR1) & (1 << I2C_SR1_AF);
    if (temp1 && temp2)
    {
        /* Clear the ACK failure error flag */
        pI2CHandle->pI2Cx->SR1 &= ~(1 << I2C_SR1_AF);

        /* Notify the application about the error */
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_AF);
    }

    /* Check for Overrun/underrun error */
    temp1 = (pI2CHandle->pI2Cx->SR1) & (1 << I2C_SR1_OVR);
    if (temp1 && temp2)
    {
        /* Clear the Overrun/underrun error flag */
        pI2CHandle->pI2Cx->SR1 &= ~(1 << I2C_SR1_OVR);

        /* Notify the application about the error */
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_OVR);
    }

    /* Check for Time out error */
    temp1 = (pI2CHandle->pI2Cx->SR1) & (1 << I2C_SR1_TIMEOUT);
    if (temp1 && temp2)
    {
        /* Implement the code to clear the Time out error flag */
        pI2CHandle->pI2Cx->SR1 &= ~(1 << I2C_SR1_TIMEOUT);

        /* Implement the code to notify the application about the error */
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_TIMEOUT);
    }
}