#include "main.h"
#include "bsp_misc.h"

void Error_Handler(void);

void bsp_init(void) {
	BSP_LED_Init();
}

void BSP_LED_Init(void) {
	GPIO_InitTypeDef  gpio_init_structure = {0};
	gpio_init_structure.Pin   = ACTIVITY_LED_DEBUG_PIN | ACTIVITY_LED_AUDIO_PIN;
	gpio_init_structure.Mode  = GPIO_MODE_OUTPUT_PP;
	gpio_init_structure.Pull  = GPIO_NOPULL;
	gpio_init_structure.Speed = GPIO_SPEED_LOW;

	ACTIVITY_LED_GPIO_CLK_ENABLE();
	HAL_GPIO_Init(ACTIVITY_LED_PORT, &gpio_init_structure);
	HAL_GPIO_WritePin(ACTIVITY_LED_PORT, ACTIVITY_LED_DEBUG_PIN, GPIO_PIN_SET); // onboard led is active low
	HAL_GPIO_WritePin(ACTIVITY_LED_PORT, ACTIVITY_LED_AUDIO_PIN, GPIO_PIN_SET);
}

void BSP_LED_DeInit(void){
	HAL_GPIO_WritePin(ACTIVITY_LED_PORT, ACTIVITY_LED_DEBUG_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ACTIVITY_LED_PORT, ACTIVITY_LED_AUDIO_PIN, GPIO_PIN_RESET);

	GPIO_InitTypeDef  gpio_init_structure = {0};
	gpio_init_structure.Pin   = ACTIVITY_LED_DEBUG_PIN | ACTIVITY_LED_AUDIO_PIN;
	HAL_GPIO_DeInit(ACTIVITY_LED_PORT, gpio_init_structure.Pin);
}


void BSP_OnboardLED_On(void) {
    HAL_GPIO_WritePin(ACTIVITY_LED_PORT, ACTIVITY_LED_DEBUG_PIN, GPIO_PIN_RESET);
}

void BSP_OnboardLED_Off(void) {
    HAL_GPIO_WritePin(ACTIVITY_LED_PORT, ACTIVITY_LED_DEBUG_PIN, GPIO_PIN_SET);
}

void BSP_OnboardLED_Toggle(void) {
	HAL_GPIO_TogglePin(ACTIVITY_LED_PORT, ACTIVITY_LED_DEBUG_PIN);
}

void BSP_AudioLED_On(void) {
	HAL_GPIO_WritePin(ACTIVITY_LED_PORT, ACTIVITY_LED_AUDIO_PIN, GPIO_PIN_RESET);
}

void BSP_AudioLED_Off(void) {
	HAL_GPIO_WritePin(ACTIVITY_LED_PORT, ACTIVITY_LED_AUDIO_PIN, GPIO_PIN_SET);
}

void BSP_AudioLED_Toggle(void) {
	HAL_GPIO_TogglePin(ACTIVITY_LED_PORT, ACTIVITY_LED_AUDIO_PIN);
}
