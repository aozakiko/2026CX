#include "serial_image.h"
#include "usart.h"
#include "stm32f10x_usart.h"

#if SERIAL_IMAGE_DEBUG
static void SerialImage_DebugByte(u8 byte)
{
	if (USART_GetFlagStatus(USART1, USART_FLAG_TXE) != RESET)
	{
		USART_SendData(USART1, byte);
	}
}
#define SERIAL_IMAGE_DBG(ch) SerialImage_DebugByte((u8)(ch))
#else
#define SERIAL_IMAGE_DBG(ch)
#endif

typedef enum
{
	SERIAL_IMAGE_WAIT_MAGIC0 = 0,
	SERIAL_IMAGE_WAIT_MAGIC1,
	SERIAL_IMAGE_WAIT_LEN_L,
	SERIAL_IMAGE_WAIT_LEN_H,
	SERIAL_IMAGE_WAIT_PAYLOAD,
	SERIAL_IMAGE_WAIT_CHECKSUM
} SerialImageState;

static volatile SerialImageState g_state = SERIAL_IMAGE_WAIT_MAGIC0;
static volatile u16 g_expected_len = 0;
static volatile u16 g_index = 0;
static volatile u8 g_checksum = 0;
static volatile u8 g_frame_ready = 0;
static u8 g_rx_frame[SERIAL_IMAGE_PAYLOAD_SIZE];
static u8 g_ready_frame[SERIAL_IMAGE_PAYLOAD_SIZE];
static volatile u32 g_frame_count = 0;
static volatile u32 g_checksum_error_count = 0;
static volatile u32 g_dropped_frame_count = 0;

void SerialImage_Reset(void)
{
	g_state = SERIAL_IMAGE_WAIT_MAGIC0;
	g_expected_len = 0;
	g_index = 0;
	g_checksum = 0;
}

void SerialImage_InputByte(u8 byte)
{
	u16 i;

	switch (g_state)
	{
	case SERIAL_IMAGE_WAIT_MAGIC0:
		if (byte == SERIAL_IMAGE_MAGIC_0)
		{
			g_state = SERIAL_IMAGE_WAIT_MAGIC1;
		}
		break;

	case SERIAL_IMAGE_WAIT_MAGIC1:
		if (byte == SERIAL_IMAGE_MAGIC_1)
		{
			SERIAL_IMAGE_DBG('H');
			g_state = SERIAL_IMAGE_WAIT_LEN_L;
		}
		else if (byte != SERIAL_IMAGE_MAGIC_0)
		{
			g_state = SERIAL_IMAGE_WAIT_MAGIC0;
		}
		break;

	case SERIAL_IMAGE_WAIT_LEN_L:
		g_expected_len = byte;
		g_state = SERIAL_IMAGE_WAIT_LEN_H;
		break;

	case SERIAL_IMAGE_WAIT_LEN_H:
		g_expected_len |= ((u16)byte << 8);
		if (g_expected_len == SERIAL_IMAGE_PAYLOAD_SIZE)
		{
			SERIAL_IMAGE_DBG('L');
			g_index = 0;
			g_checksum = 0;
			g_state = SERIAL_IMAGE_WAIT_PAYLOAD;
		}
		else
		{
			SERIAL_IMAGE_DBG('N');
			SerialImage_Reset();
		}
		break;

	case SERIAL_IMAGE_WAIT_PAYLOAD:
		g_rx_frame[g_index++] = byte;
		g_checksum ^= byte;
		if (g_index >= SERIAL_IMAGE_PAYLOAD_SIZE)
		{
			g_state = SERIAL_IMAGE_WAIT_CHECKSUM;
		}
		break;

	case SERIAL_IMAGE_WAIT_CHECKSUM:
		if (byte == g_checksum)
		{
			if (g_frame_ready == 0)
			{
				for (i = 0; i < SERIAL_IMAGE_PAYLOAD_SIZE; i++)
				{
					g_ready_frame[i] = g_rx_frame[i];
				}
				g_frame_ready = 1;
				g_frame_count++;
				USART1_SendByte(SERIAL_IMAGE_ACK);
			}
			else
			{
				g_dropped_frame_count++;
				USART1_SendByte(SERIAL_IMAGE_ACK);
			}
		}
		else
		{
			g_checksum_error_count++;
			SERIAL_IMAGE_DBG('E');
		}
		SerialImage_Reset();
		break;

	default:
		SerialImage_Reset();
		break;
	}
}

u8 SerialImage_FrameReady(void)
{
	return g_frame_ready;
}

const u8 *SerialImage_GetFrame(void)
{
	return g_ready_frame;
}

u8 SerialImage_FetchFrame(u8 *dst)
{
	u16 i;

	if (g_frame_ready == 0)
	{
		return 0;
	}

	for (i = 0; i < SERIAL_IMAGE_PAYLOAD_SIZE; i++)
	{
		dst[i] = g_ready_frame[i];
	}
	g_frame_ready = 0;

	return 1;
}

void SerialImage_ReleaseFrame(void)
{
	g_frame_ready = 0;
}

u32 SerialImage_GetFrameCount(void)
{
	return g_frame_count;
}

u32 SerialImage_GetChecksumErrorCount(void)
{
	return g_checksum_error_count;
}

u32 SerialImage_GetDroppedFrameCount(void)
{
	return g_dropped_frame_count;
}
