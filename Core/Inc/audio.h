#include "stm32f4xx_hal.h"

#define AUDIO_OK                            ((uint8_t)0)
#define AUDIO_ERROR                         ((uint8_t)1)
#define AUDIO_TIMEOUT                       ((uint8_t)2)

#define DMA_MAX_SZE                     0xFFFF
#define DMA_MAX(_X_)                (((_X_) <= DMA_MAX_SZE)? (_X_):DMA_MAX_SZE)
#define AUDIODATA_SIZE                  2   /* 16-bits audio data size */

void Audio_HalfTransfer_Callback(void);
void Audio_TransferComplete_Callback(void);

uint8_t Audio_Init(uint8_t Volume, uint32_t AudioFreq);
uint8_t Audio_Play(uint16_t *pBuffer, uint32_t Size);
void Audio_ChangeBuffer(uint16_t *pData, uint16_t Size);
uint8_t Audio_Stop(void);
void Audio_Pause(void);
