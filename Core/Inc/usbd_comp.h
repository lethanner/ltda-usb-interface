/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_COMP_H_
#define __USBD_COMP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_ioreq.h"
#include "usbd_desc.h"
#include "stm32f4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
typedef struct
{
} USBD_COMP_HandleTypeDef;

typedef struct
{
} USBD_COMP_ItfTypeDef;

typedef struct
{
  USBD_ClassTypeDef *class;
  void *classData;
  void *userData;
  uint16_t ctrlIf;
  uint16_t minIf;
  uint16_t maxIf;
} USBD_ClassCompInfo;


/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables---------------------------------------------------------*/
extern USBD_ClassTypeDef    USBD_COMP;
extern USBD_COMP_ItfTypeDef USBD_COMP_fops_FS;

/* Exported functions prototypes ---------------------------------------------*/
uint8_t USBD_COMP_RegisterInterface (USBD_HandleTypeDef *pdev, USBD_COMP_ItfTypeDef *fops);
uint8_t COMP_CDC_Transmit_FS (uint8_t* Buf, uint16_t  Len);


/* Private defines -----------------------------------------------------------*/
#define CLASS_NUM                    2U
#define VCP                          0U
#define UAC                          1U
#define USBD_COMP_CLASS              &USBD_COMP
#define USB_COMP_CONFIG_DESC_SIZ     16U + (USB_AUDIO_CONFIG_DESC_SIZ - 9U) + \
											(USB_CDC_CONFIG_DESC_SIZ - 9U) + 9U

#endif /* ST_STM32_USB_DEVICE_LIBRARY_CLASS_COMP_INC_USBD_COMP_H_ */
