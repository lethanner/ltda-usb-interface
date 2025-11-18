#include "audio.h"
#include "usbd_audio_if.h"

extern I2S_HandleTypeDef hi2s2;
uint32_t sample_counter = 0;

void Audio_HalfTransfer_Callback(void)
{
	HalfTransfer_CallBack_FS();
}

void Audio_TransferComplete_Callback(void)
{
	TransferComplete_CallBack_FS();

	/*if (sample_counter++ > 5) {
		sample_counter = 0;
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	}*/
}

uint8_t Audio_Init(uint8_t Volume, uint32_t AudioFreq)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

	// TODO?
	UNUSED(Volume);
	UNUSED(AudioFreq);

	return AUDIO_OK;
}

uint8_t Audio_Play(uint16_t *pBuffer, uint32_t Size)
{
	if(HAL_I2S_Transmit_DMA(&hi2s2,(uint16_t*)pBuffer,
							DMA_MAX(Size/AUDIODATA_SIZE))!=HAL_OK)
		return AUDIO_ERROR;

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	return AUDIO_OK;
}

void Audio_ChangeBuffer(uint16_t *pData, uint16_t Size)
{
	HAL_I2S_Transmit_DMA(&hi2s2,(uint16_t*)pData,Size);
}

uint8_t Audio_Stop(void)
{
	//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	if(HAL_I2S_DMAStop(&hi2s2)!=HAL_OK)
		return AUDIO_ERROR;

	return AUDIO_OK;
}

void Audio_Pause(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}
