# PC Camera To STM32 TFTLCD

## Wiring

- USART1 RX: PA10, connect to USB-TTL TX
- USART1 TX: PA9, connect to USB-TTL RX
- GND: common ground
- TFTLCD uses FSMC Bank1/NE4 from the added reference project:
  PD0, PD1, PD4, PD5, PD8, PD9, PD10, PD14, PD15;
  PE7, PE8, PE9, PE10, PE11, PE12, PE13, PE14, PE15;
  PG0, PG12; PB0 for backlight.
- The default LCD driver is `TFTLCD_HX8357DN` in `APP/tftlcd/tftlcd.h`.

## Run

Install PC dependencies:

```powershell
pip install -r PC\requirements.txt
```

After flashing the STM32 project, run:

```powershell
python PC\send_camera_to_oled.py --port COM3 --baud 115200 --preview
```

Change `COM3` to your USB-TTL serial port. Use `--camera 1` if the default camera is not the one you want.

## Packet Format

The PC sends:

```text
A5 5A + 2-byte little-endian payload length + 16384 payload bytes + 1-byte XOR checksum
```

The payload is 128x64 RGB565 color data, high byte first for each pixel. The MCU verifies the frame and scales it to the TFTLCD.

At 115200 baud, one full color frame is about 16 KB, so the practical frame rate is low. Use a higher baud rate only if your USB-TTL and STM32 wiring are stable at that speed.
The PC script defaults to `--fps 1.0` for this reason.
