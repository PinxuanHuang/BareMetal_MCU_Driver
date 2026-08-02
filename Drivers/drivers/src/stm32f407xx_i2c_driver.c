#include "stm32f407xx_i2c_driver.h"

void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        pI2Cx->CR1 |= (1 << I2C_CR1_PE);
        // pI2cBaseAddress->CR1 |= I2C_CR1_PE_Bit_Mask;
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

void I2C_Init(I2C_Handle_t *pI2CHandle)
{
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

uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName)
{
    if (pI2Cx->SR1 & FlagName)
    {
        return FLAG_SET;
    }
    return FLAG_RESET;
}
