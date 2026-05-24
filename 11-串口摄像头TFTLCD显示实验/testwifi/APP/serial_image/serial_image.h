#ifndef _serial_image_H
#define _serial_image_H

#include "system.h"

#define SERIAL_IMAGE_MAGIC_0       0xA5
#define SERIAL_IMAGE_MAGIC_1       0x5A
#define SERIAL_IMAGE_WIDTH         128
#define SERIAL_IMAGE_HEIGHT        64
#define SERIAL_IMAGE_BYTES_PER_PIXEL 2
#define SERIAL_IMAGE_PAYLOAD_SIZE  (SERIAL_IMAGE_WIDTH * SERIAL_IMAGE_HEIGHT * SERIAL_IMAGE_BYTES_PER_PIXEL)
#define SERIAL_IMAGE_ACK           'K'
#define SERIAL_IMAGE_DEBUG         0

void SerialImage_Reset(void);
void SerialImage_InputByte(u8 byte);
u8 SerialImage_FrameReady(void);
const u8 *SerialImage_GetFrame(void);
u8 SerialImage_FetchFrame(u8 *dst);
void SerialImage_ReleaseFrame(void);
u32 SerialImage_GetFrameCount(void);
u32 SerialImage_GetChecksumErrorCount(void);
u32 SerialImage_GetDroppedFrameCount(void);

#endif
