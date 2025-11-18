#ifndef __BSP_MISC_H
#define __BSP_MISC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"

#define ACTIVITY_LED_PORT				GPIOC
#define ACTIVITY_LED_DEBUG_PIN			GPIO_PIN_14
#define ACTIVITY_LED_AUDIO_PIN			GPIO_PIN_13
#define ACTIVITY_LED_GPIO_CLK_ENABLE()	__HAL_RCC_GPIOC_CLK_ENABLE()

void bsp_init(void);
void BSP_LED_Init(void);
void BSP_LED_DeInit(void);
void BSP_OnboardLED_On(void);
void BSP_OnboardLED_Off(void);
void BSP_OnboardLED_Toggle(void);
void BSP_AudioLED_On(void);
void BSP_AudioLED_Off(void);
void BSP_AudioLED_Toggle(void);


#ifdef __cplusplus
}
#endif

#endif 


