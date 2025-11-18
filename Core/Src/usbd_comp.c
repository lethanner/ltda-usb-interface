/* Includes ------------------------------------------------------------------*/
#include "usbd_comp.h"
#include "usbd_audio.h"
#include "usbd_audio_if.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "usbd_ctlreq.h"

#include <stdio.h>
#include "main.h"


/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define AUDIO_SAMPLE_FREQ(frq)  (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

/*#define AUDIO_PACKET_SZE(frq)   (uint8_t)(((frq * 2U * 2U)/1000U) & 0xFFU), \
                                (uint8_t)((((frq * 2U * 2U)/1000U) >> 8) & 0xFFU)*/

#define AUDIO_PACKET_SZE_24B(frq) (uint8_t)(((frq / 1000U + 1) * 2U * 3U) & 0xFFU), \
                                  (uint8_t)((((frq / 1000U + 1) * 2U * 3U) >> 8) & 0xFFU)

/* Private macro -------------------------------------------------------------*/

/* Private function prototypes------------------------------------------------*/
static uint8_t USBD_COMP_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx);

static uint8_t USBD_COMP_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx);

static uint8_t USBD_COMP_Setup (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static uint8_t* USBD_COMP_GetCfgDesc (uint16_t *length);

static uint8_t* USBD_COMP_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t USBD_COMP_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t USBD_COMP_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t USBD_COMP_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t USBD_COMP_EP0_TxReady (USBD_HandleTypeDef *pdev);

static uint8_t USBD_COMP_SOF (USBD_HandleTypeDef *pdev);

static uint8_t USBD_COMP_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t USBD_COMP_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

/* Private variables ---------------------------------------------------------*/

USBD_ClassTypeDef  USBD_COMP =
{
  USBD_COMP_Init,
  USBD_COMP_DeInit,
  USBD_COMP_Setup,
  USBD_COMP_EP0_TxReady,
  USBD_COMP_EP0_RxReady,
  USBD_COMP_DataIn,
  USBD_COMP_DataOut,
  USBD_COMP_SOF,
  USBD_COMP_IsoINIncomplete,
  USBD_COMP_IsoOutIncomplete,
  USBD_COMP_GetCfgDesc,
  USBD_COMP_GetCfgDesc,
  USBD_COMP_GetCfgDesc,
  USBD_COMP_GetDeviceQualifierDesc,
};

extern USBD_HandleTypeDef USBD_Device;

USBD_COMP_ItfTypeDef USBD_COMP_fops_FS;
USBD_ClassCompInfo comp_dev [CLASS_NUM];


/* USB COMP device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_COMP_CfgDesc[USB_COMP_CONFIG_DESC_SIZ] __ALIGN_END =
{
	// Configuration 1
	0x09,                              /* bLength */
	USB_DESC_TYPE_CONFIGURATION,       /* bDescriptorType */
	LOBYTE(USB_COMP_CONFIG_DESC_SIZ),  /* wTotalLength bytes*/
	HIBYTE(USB_COMP_CONFIG_DESC_SIZ),
	0x04, /* bNumInterfaces */
	0x01, /* bConfigurationValue */
	0x00, /* iConfiguration */
	0xC0, /* bmAttributes  SELF Powered */
	0x32, /* bMaxPower = 50*2mA = 100 mA*/
	// 09 byte
	/* CDC IAD */
	0x08, /* bLength: Interface Descriptor size */
	0x0B, /* bDescriptorType: IAD */
	CDC_CTRL_IF,  /* bFirstInterface */
	0x02, /* bInterfaceCount */
	0x02, /* bFunctionClass: CDC */
	0x02, /* bFunctionSubClass */
	0x01, /* bFunctionProtocol */
	0x00, /* iFunction */
	/* 08 bytes */
	/* Interface Descriptor */
	  0x09,                                       /* bLength: Interface Descriptor size */
	  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: Interface */
	  /* Interface descriptor type */
	  0x00,                                       /* bInterfaceNumber: Number of Interface */
	  0x00,                                       /* bAlternateSetting: Alternate setting */
	  0x01,                                       /* bNumEndpoints: One endpoint used */
	  0x02,                                       /* bInterfaceClass: Communication Interface Class */
	  0x02,                                       /* bInterfaceSubClass: Abstract Control Model */
	  0x01,                                       /* bInterfaceProtocol: Common AT commands */
	  0x00,                                       /* iInterface */

	  /* Header Functional Descriptor */
	  0x05,                                       /* bLength: Endpoint Descriptor size */
	  0x24,                                       /* bDescriptorType: CS_INTERFACE */
	  0x00,                                       /* bDescriptorSubtype: Header Func Desc */
	  0x10,                                       /* bcdCDC: spec release number */
	  0x01,

	  /* Call Management Functional Descriptor */
	  0x05,                                       /* bFunctionLength */
	  0x24,                                       /* bDescriptorType: CS_INTERFACE */
	  0x01,                                       /* bDescriptorSubtype: Call Management Func Desc */
	  0x00,                                       /* bmCapabilities: D0+D1 */
	  0x01,                                       /* bDataInterface */

	  /* ACM Functional Descriptor */
	  0x04,                                       /* bFunctionLength */
	  0x24,                                       /* bDescriptorType: CS_INTERFACE */
	  0x02,                                       /* bDescriptorSubtype: Abstract Control Management desc */
	  0x02,                                       /* bmCapabilities */

	  /* Union Functional Descriptor */
	  0x05,                                       /* bFunctionLength */
	  0x24,                                       /* bDescriptorType: CS_INTERFACE */
	  0x06,                                       /* bDescriptorSubtype: Union func desc */
	  CDC_CTRL_IF,                                /* bMasterInterface: Communication class interface */
	  CDC_DATA_IF,                                /* bSlaveInterface0: Data Class Interface */

	  /* Endpoint 2 Descriptor */
	  0x07,                                       /* bLength: Endpoint Descriptor size */
	  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
	  CDC_CMD_EP,                                 /* bEndpointAddress */
	  0x03,                                       /* bmAttributes: Interrupt */
	  LOBYTE(CDC_CMD_PACKET_SIZE),                /* wMaxPacketSize */
	  HIBYTE(CDC_CMD_PACKET_SIZE),
	  CDC_FS_BINTERVAL,                           /* bInterval */
	  /*---------------------------------------------------------------------------*/

	  /* Data class interface descriptor */
	  0x09,                                       /* bLength: Endpoint Descriptor size */
	  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: */
	  CDC_DATA_IF,                                /* bInterfaceNumber: Number of Interface */
	  0x00,                                       /* bAlternateSetting: Alternate setting */
	  0x02,                                       /* bNumEndpoints: Two endpoints used */
	  0x0A,                                       /* bInterfaceClass: CDC */
	  0x00,                                       /* bInterfaceSubClass */
	  0x00,                                       /* bInterfaceProtocol */
	  0x00,                                       /* iInterface */

	  /* Endpoint OUT Descriptor */
	  0x07,                                       /* bLength: Endpoint Descriptor size */
	  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
	  CDC_OUT_EP,                                 /* bEndpointAddress */
	  0x02,                                       /* bmAttributes: Bulk */
	  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),        /* wMaxPacketSize */
	  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
	  0x00,                                       /* bInterval */

	  /* Endpoint IN Descriptor */
	  0x07,                                       /* bLength: Endpoint Descriptor size */
	  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
	  CDC_IN_EP,                                  /* bEndpointAddress */
	  0x02,                                       /* bmAttributes: Bulk */
	  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),        /* wMaxPacketSize */
	  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
	  0x00,                                        /* bInterval */
	/* AUDIO IAD */
	0x08, /* bLength: Interface Descriptor size */
	0x0B, /* bDescriptorType: IAD */
	AUDIO_CTRL_IF, /* bFirstInterface */
	0x02, /* bInterfaceCount */
	0x01, /* bFunctionClass  = USB_DEVICE_CLASS_AUDIO = 0x01 */
	0x01, /* bFunctionSubClass = AUDIO_SUBCLASS_AUDIOCONTROL = 0x01 */
	0x00, /* bFunctionProtocol = AUDIO_PROTOCOL_UNDEFINED = Non-Basic Audio Device */
	0x00, /* iFunction */
	/* 08 bytes */
	// USB Speaker Standard interface descriptor
	    AUDIO_INTERFACE_DESC_SIZE,   /* bLength */
	    USB_DESC_TYPE_INTERFACE,     /* bDescriptorType */
	    AUDIO_CTRL_IF,               /* bInterfaceNumber */
	    0x00,                        /* bAlternateSetting */
	    0x00,                        /* bNumEndpoints */
	    USB_DEVICE_CLASS_AUDIO,      /* bInterfaceClass */
	    AUDIO_SUBCLASS_AUDIOCONTROL, /* bInterfaceSubClass */
	    AUDIO_PROTOCOL_UNDEFINED,    /* bInterfaceProtocol */
	    0x00,                        /* iInterface */
	    // 09 byte

	    // USB Speaker Class-specific AC Interface Descriptor
	    AUDIO_INTERFACE_DESC_SIZE,       /* bLength */
	    AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	    AUDIO_CONTROL_HEADER,            /* bDescriptorSubtype */
	    0x00, /* 1.00 */                 /* bcdADC */
	    0x01,
	    0x27, /* wTotalLength = 39*/
	    0x00,
	    0x01, /* bInCollection */
	    AUDIO_OUT_IF, /* baInterfaceNr */
	    // 09 byte

	    // USB Speaker Input Terminal Descriptor
	    AUDIO_INPUT_TERMINAL_DESC_SIZE,  /* bLength */
	    AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	    AUDIO_CONTROL_INPUT_TERMINAL,    /* bDescriptorSubtype */
	    0x01,                            /* bTerminalID */
	    0x01,                            /* wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 */
	    0x01,
	    0x00, /* bAssocTerminal */
	    0x02, /* bNrChannels */
	    0x03, /* wChannelConfig 0x0003  FL FR */
	    0x00,
	    0x00, /* iChannelNames */
	    0x00, /* iTerminal */
	    // 12 byte

	    // USB Speaker Audio Feature Unit Descriptor
	    0x09,                            /* bLength */
	    AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	    AUDIO_CONTROL_FEATURE_UNIT,      /* bDescriptorSubtype */
	    AUDIO_OUT_STREAMING_CTRL,        /* bUnitID */
	    0x01,                            /* bSourceID */
	    0x01,                            /* bControlSize */
		(AUDIO_CONTROL_MUTE | AUDIO_CONTROL_VOL),          /* bmaControls(0) */
	    0,                               /* bmaControls(1) */
	    0x00,                            /* iTerminal */
	    // 09 byte

	    // USB Speaker Output Terminal Descriptor
	    0x09,                            /* bLength */
	    AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	    AUDIO_CONTROL_OUTPUT_TERMINAL,   /* bDescriptorSubtype */
	    0x03,                            /* bTerminalID */
	    0x01,                            /* wTerminalType  0x0301*/
	    0x03,
	    0x00, /* bAssocTerminal */
	    0x02, /* bSourceID */
	    0x00, /* iTerminal */
	    // 09 byte

	    // USB Speaker Standard AS Interface Descriptor
	    // Interface 1, Alternate Setting 0
		// Zero Bandwidth with zero endpoints, used to relinquish bandwidth
		// when audio not used.
	    AUDIO_INTERFACE_DESC_SIZE,     /* bLength */
	    USB_DESC_TYPE_INTERFACE,       /* bDescriptorType */
		AUDIO_OUT_IF,                  /* bInterfaceNumber */
	    0x00,                          /* bAlternateSetting */
	    0x00,                          /* bNumEndpoints */
	    USB_DEVICE_CLASS_AUDIO,        /* bInterfaceClass */
	    AUDIO_SUBCLASS_AUDIOSTREAMING, /* bInterfaceSubClass */
	    AUDIO_PROTOCOL_UNDEFINED,      /* bInterfaceProtocol */
	    0x00,                          /* iInterface */
	    // 09 byte

	    // USB Speaker Standard AS Interface Descriptor
	    // Interface 1, Alternate Setting 1
		// Used when Audio Streaming is in operation
	    AUDIO_INTERFACE_DESC_SIZE,     /* bLength */
	    USB_DESC_TYPE_INTERFACE,       /* bDescriptorType */
		AUDIO_OUT_IF,                   /* bInterfaceNumber */
	    0x01,                          /* bAlternateSetting */
	    0x02,                          /* bNumEndpoints - 1 output & 1 feedback (???) */
	    USB_DEVICE_CLASS_AUDIO,        /* bInterfaceClass */
	    AUDIO_SUBCLASS_AUDIOSTREAMING, /* bInterfaceSubClass */
	    AUDIO_PROTOCOL_UNDEFINED,      /* bInterfaceProtocol */
	    0x00,                          /* iInterface */
	    // 09 byte

	    // USB Speaker Audio Streaming Interface Descriptor
	    AUDIO_STREAMING_INTERFACE_DESC_SIZE, /* bLength */
	    AUDIO_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
	    AUDIO_STREAMING_GENERAL,             /* bDescriptorSubtype */
	    0x01,                                /* bTerminalLink */
	    0x01,                                /* bDelay */
	    0x01,                                /* wFormatTag AUDIO_FORMAT_PCM  0x0001*/
	    0x00,
	    // 07 byte

	    // USB Speaker Audio Type I Format Interface Descriptor
	    17,                            /* bLength */
	    AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
	    AUDIO_STREAMING_FORMAT_TYPE,     /* bDescriptorSubtype */
	    AUDIO_FORMAT_TYPE_I,             /* bFormatType */
	    2,                            /* bNrChannels */
	    3,                            /* bSubFrameSize :  3 Bytes per frame (24bits) */
	    24,                            /* bBitResolution (24-bits per sample) */
	    3,                            /* bSamFreqType 3 frequencies supported */
	    AUDIO_SAMPLE_FREQ(44100),        /* Audio sampling frequency coded on 3 bytes */
	    AUDIO_SAMPLE_FREQ(48000),        /* Audio sampling frequency coded on 3 bytes */
	    AUDIO_SAMPLE_FREQ(96000),        /* Audio sampling frequency coded on 3 bytes */
	    // 17 byte

	    // Endpoint 1 - Standard Descriptor
		// Isochronous Async endpoint for audio packets
	    AUDIO_STANDARD_ENDPOINT_DESC_SIZE,         /* bLength */
	    USB_DESC_TYPE_ENDPOINT,                    /* bDescriptorType */
	    AUDIO_OUT_EP,                              /* bEndpointAddress 1 out endpoint*/
	    USBD_EP_TYPE_ISOC_ASYNC,                   /* bmAttributes */
	    AUDIO_PACKET_SZE_24B(USBD_AUDIO_FREQ_MAX), /* wMaxPacketSize in Bytes (freq / 1000 + extra_samples) * channels * bytes_per_sample */
	    0x01,                                      /* bInterval */
	    0x00,                                      /* bRefresh */
	    AUDIO_IN_EP,                               /* bSynchAddress */
	    // 09 byte

	    // Endpoint - Audio Streaming Descriptor
	    AUDIO_STREAMING_ENDPOINT_DESC_SIZE, /* bLength */
	    AUDIO_ENDPOINT_DESCRIPTOR_TYPE,     /* bDescriptorType */
	    AUDIO_ENDPOINT_GENERAL,             /* bDescriptor */
	    0x01,                               /* bmAttributes - Sampling Frequency control is supported. See UAC Spec 1.0 p.62 */
	    0x00,                               /* bLockDelayUnits */
	    0x00,                               /* wLockDelay */
	    0x00,
	    // 07 byte

	    // Endpoint 2 - Standard Descriptor - See UAC Spec 1.0 p.63 4.6.2.1 Standard AS Isochronous Synch Endpoint Descriptor
		// 3byte 10.14 sampling frequency feedback to host
	    AUDIO_STANDARD_ENDPOINT_DESC_SIZE, /* bLength */
	    USB_DESC_TYPE_ENDPOINT,            /* bDescriptorType */
	    AUDIO_IN_EP,                       /* bEndpointAddress */
	    0x11,                              /* bmAttributes */
	    0x03, 0x00,                        /* wMaxPacketSize in Bytes */
	    0x01,                              /* bInterval 1ms */
	    SOF_RATE,                          /* bRefresh 4ms = 2^2 */
	    0x00                               /* bSynchAddress */
	    // 09 byte
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_COMP_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END=
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/* Private function prototypes -----------------------------------------------*/


/* Private user code ---------------------------------------------------------*/


/* External variables --------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/

static inline void switchToClass (USBD_HandleTypeDef *pdev, USBD_ClassCompInfo *class)
{
  pdev->pClassData = class->classData;
  pdev->pUserData  = class->userData;
}

static inline void saveClass (USBD_HandleTypeDef *pdev, USBD_ClassCompInfo *class)
{
  class->classData = pdev->pClassData;
  class->userData  = pdev->pUserData;
}

/**
 * @brief  USBD_COMP_Init
 *         Initialize the COMP interface
 * @param  pdev:   device instance
 * @param  cfgidx: configuration index
 * @retval status
 */

static uint8_t USBD_COMP_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  uint8_t retval = USBD_OK;

  USBD_CDC_LineCodingTypeDef line_coding =
  {
    /* 115200 8n1 */
    .bitrate    = 115200U, /* Data terminal rate, in bits per second */
    .format     = 0U,    /* Stop bits: 0 - 1 Stop bit */
    .paritytype = 0U,    /* Parity:    0 - None */
    .datatype   = 8U,    /* Data bits */
  };

  comp_dev[VCP].class    = &USBD_CDC;
  comp_dev[VCP].userData = &USBD_Interface_fops_FS;
  comp_dev[VCP].ctrlIf   = CDC_CTRL_IF;
  comp_dev[VCP].minIf    = CDC_CTRL_IF;
  comp_dev[VCP].maxIf    = CDC_DATA_IF;

  switchToClass (pdev, &comp_dev[VCP]);
  retval = comp_dev[VCP].class->Init (pdev, cfgidx);
  saveClass (pdev, &comp_dev[VCP]);
  memcpy ((uint8_t*) comp_dev[VCP].classData, &line_coding, sizeof (line_coding));

  comp_dev[UAC].class    = &USBD_AUDIO;
  comp_dev[UAC].userData = &USBD_AUDIO_fops;
  comp_dev[UAC].ctrlIf   = AUDIO_CTRL_IF;
  comp_dev[UAC].minIf    = AUDIO_CTRL_IF;
  comp_dev[UAC].maxIf    = AUDIO_OUT_IF;

  switchToClass (pdev, &comp_dev[UAC]);
  retval = comp_dev[UAC].class->Init (pdev, cfgidx);
  saveClass (pdev, &comp_dev[UAC]);

  return retval;
}

/**
 * @brief  USBD_COMP_DeInit
 *         DeInitialize the COMP layer
 * @param  pdev:   device instance
 * @param  cfgidx: configuration index
 * @retval status
 */

static uint8_t USBD_COMP_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  uint8_t retval = USBD_OK;

  for (uint8_t i = 0U; i < CLASS_NUM; i++)
  {
    switchToClass (pdev, &comp_dev[i]);
    retval = comp_dev[i].class->DeInit (pdev, cfgidx);
    saveClass (pdev, &comp_dev[i]);
  }
  return retval;
}

/**
 * @brief  USBD_COMP_Setup
 *         Handle the AUDIO specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */

static uint8_t USBD_COMP_Setup (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  uint16_t len;
  uint8_t  *pbuf;
  uint8_t  retval = USBD_OK;
  uint8_t  done = 0U;
  uint8_t  i;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
      switch (req->bRequest)
      {
        case AUDIO_REQ_GET_CUR:
        case AUDIO_REQ_SET_CUR:
        case AUDIO_REQ_GET_MIN:
        case AUDIO_REQ_GET_MAX:
        case AUDIO_REQ_GET_RES:

          switchToClass (pdev, &comp_dev[UAC]);
          retval = comp_dev[UAC].class->Setup (pdev, req);
          if (!retval) done = 1U;
          break;

        case CDC_SET_LINE_CODING:
        case CDC_GET_LINE_CODING:
        case CDC_SET_CONTROL_LINE_STATE:

          switchToClass (pdev, &comp_dev[VCP]);
          retval = comp_dev[VCP].class->Setup (pdev, req);
          if (!retval) done = 1U;
          break;
      }

      if (done != 1U)
      {
        USBD_CtlError (pdev, req);
        retval = USBD_FAIL;
      }

      break;

    case USB_REQ_TYPE_STANDARD:

      switch (req->bRequest)
      {
        case USB_REQ_GET_DESCRIPTOR:

          if( (req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
          {
            pbuf = USBD_COMP_CfgDesc + 92;
            len  = MIN (pbuf[0] , req->wLength);

            USBD_CtlSendData (pdev, pbuf, len);
          }
          break;

        case USB_REQ_GET_INTERFACE:
        case USB_REQ_SET_INTERFACE:

          if ((uint8_t) (req->wIndex) <= USBD_MAX_NUM_INTERFACES)
          {
            for (i = 0U; i < CLASS_NUM; i++)
            {
              if ((req->wIndex >= comp_dev[i].minIf) && (req->wIndex <= comp_dev[i].maxIf))
              {
                switchToClass (pdev, &comp_dev[i]);
                retval = comp_dev[i].class->Setup (pdev, req);
                done = 1U;
                break;
              }
            }
          }
          else
          {
            /* Call the error management function (command will be nacked */
            USBD_CtlError (pdev, req);
          }
          break;

        default:
          USBD_CtlError (pdev, req);
          retval = USBD_FAIL;
      }
  }
  return retval;
}

/**
 * @brief  USBD_COMP_DataIn
 *         handle data IN Stage
 * @param  pdev:  device instance
 * @param  epnum: endpoint index
 * @retval status
 */

static uint8_t  USBD_COMP_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  uint8_t retval = USBD_OK;
  uint8_t i;

  if (epnum == (CDC_IN_EP & 0x7F))   i = VCP;
  if (epnum == (AUDIO_IN_EP & 0x7F)) i = UAC;

  if (comp_dev[i].class->DataIn != NULL)
  {
    switchToClass (pdev, &comp_dev[i]);
    retval = comp_dev[i].class->DataIn (pdev, epnum);
  }

  return retval;
}

/**
 * @brief  USBD_COMP_DataOut
 *         handle data OUT Stage
 * @param  pdev:  device instance
 * @param  epnum: endpoint index
 * @retval status
 */

static uint8_t  USBD_COMP_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  uint8_t retval = USBD_OK;
  uint8_t i;

  if (epnum == CDC_OUT_EP)   i = VCP;
  if (epnum == AUDIO_OUT_EP) i = UAC;

  if (comp_dev[i].class->DataOut != NULL)
  {
    switchToClass (pdev, &comp_dev[i]);
    retval = comp_dev[i].class->DataOut (pdev, epnum);
  }

  return retval;
}

/**
 * @brief  USBD_COMP_SOF
 *         handle SOF event
 * @param  pdev: device instance
 * @retval status
 */

static uint8_t  USBD_COMP_SOF (USBD_HandleTypeDef *pdev)
{
  uint8_t retval = USBD_OK;

  for (uint8_t i = 0U; i < CLASS_NUM; i++)
  {
    if (comp_dev[i].class->SOF != NULL)
    {
      switchToClass (pdev, &comp_dev[i]);
      retval = comp_dev[i].class->SOF (pdev);
    }
  }
  return retval;
}

/**
 * @brief  USBD_COMP_EP0_RxReady
 *         handle EP0 Rx Ready event
 * @param  pdev: device instance
 * @retval status
 */

static uint8_t  USBD_COMP_EP0_RxReady (USBD_HandleTypeDef *pdev)
{
  uint8_t retval = USBD_OK;

  for (uint8_t i = 0U; i < CLASS_NUM; i++)
  {
    if (comp_dev[i].class->EP0_RxReady != NULL)
    {
      switchToClass (pdev, &comp_dev[i]);
      retval = comp_dev[i].class->EP0_RxReady (pdev);
    }
  }
  return retval;
}

/**
  * @brief  USBD_COMP_EP0_TxReady
  *         handle EP0 Tx Ready event
  * @param  pdev: device instance
  * @retval status
  */

static uint8_t  USBD_COMP_EP0_TxReady (USBD_HandleTypeDef *pdev)
{
  /* Only OUT control data are processed */
  return USBD_OK;
}

/**
  * @brief  USBD_COMP_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev:  device instance
  * @param  epnum: endpoint index
  * @retval status
  */

static uint8_t  USBD_COMP_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  return USBD_OK;
}

/**
  * @brief  USBD_COMP_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev:  device instance
  * @param  epnum: endpoint index
  * @retval status
  */

static uint8_t  USBD_COMP_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  return USBD_OK;
}

/**
 * @brief  USBD_COMP_GetCfgDesc
 *         return Configuration descriptor
 * @param  length: pointer data length
 * @retval pointer to descriptor buffer
 */

static uint8_t* USBD_COMP_GetCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_COMP_CfgDesc);
  return USBD_COMP_CfgDesc;
}

/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length: pointer data length
* @retval pointer to descriptor buffer
*/

static uint8_t* USBD_COMP_GetDeviceQualifierDesc (uint16_t *length)
{
  *length = sizeof (USBD_COMP_DeviceQualifierDesc);
  return USBD_COMP_DeviceQualifierDesc;
}

/**
 * @brief  USBD_COMP_RegisterInterface
 * @param  fops: COMP interface callback
 * @retval status
 */

uint8_t USBD_COMP_RegisterInterface (USBD_HandleTypeDef   *pdev,
                                     USBD_COMP_ItfTypeDef *fops)
{
  if (fops != NULL)
  {
    pdev->pUserData = fops;
  }
  return USBD_OK;
}


/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */

uint8_t COMP_CDC_Transmit_FS (uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;

  switchToClass (&USBD_Device, &comp_dev[VCP]);

  return result;
}



/****END OF FILE****/
