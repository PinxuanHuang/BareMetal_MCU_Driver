#ifndef INC_STM32F407XX_GPIO_DRIVER_H_
#define INC_STM32F407XX_GPIO_DRIVER_H_

#include "stm32f407xx.h"

/* Configuration structure for GPIO pin */
typedef struct {
	uint8_t GPIO_PinNumber;
	uint8_t GPIO_PinMode;
	uint8_t GPIO_PinSpeed;
	uint8_t GPIO_PinPuPdControl;
	uint8_t GPIO_PinOPType;
	uint8_t GPIO_PinAltFunMode;
} GPIO_PinConfig_t;

/* Handle structure for GPIO pin */
typedef struct {
	GPIO_RegDef_t *pGPIOBaseAddr;      /* Address of GPIO pin */
	GPIO_PinConfig_t GPIO_PinConfig;   /* Hold GPIO pin configuration Settings */
} GPIO_Handle_t;

/* Pin number */
#define GPIO_PIN_NO_0  				0
#define GPIO_PIN_NO_1  				1
#define GPIO_PIN_NO_2  				2
#define GPIO_PIN_NO_3  				3
#define GPIO_PIN_NO_4  				4
#define GPIO_PIN_NO_5  				5
#define GPIO_PIN_NO_6  				6
#define GPIO_PIN_NO_7  				7
#define GPIO_PIN_NO_8  				8
#define GPIO_PIN_NO_9  				9
#define GPIO_PIN_NO_10  			10
#define GPIO_PIN_NO_11 				11
#define GPIO_PIN_NO_12  			12
#define GPIO_PIN_NO_13 				13
#define GPIO_PIN_NO_14 				14
#define GPIO_PIN_NO_15 				15

/* Pin Mode */
#define GPIO_MODE_IN 		0
#define GPIO_MODE_OUT 		1
#define GPIO_MODE_ALTFN 	2
#define GPIO_MODE_ANALOG 	3
#define GPIO_MODE_IT_FT     4
#define GPIO_MODE_IT_RT     5
#define GPIO_MODE_IT_RFT    6

/* Pin output type */
#define GPIO_OP_TYPE_PP   0       /* Push Pull  */
#define GPIO_OP_TYPE_OD   1       /* Open Drain */

/* Pin output speeds */
#define GPIO_SPEED_LOW			0
#define GPIO_SPEED_MEDIUM		1
#define GPIO_SPEED_FAST			2
#define GPOI_SPEED_HIGH			3

/* Pin pull up / pull down configuration macros */
#define GPIO_NO_PUPD   		0
#define GPIO_PIN_PU			1
#define GPIO_PIN_PD			2

/* Peripheral Clock setup */
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnorDi);

/* Init and De-init */
void GPIO_Init(GPIO_Handle_t *pGPIOHandle);
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx);

#endif /* INC_STM32F407XX_GPIO_DRIVER_H_ */
