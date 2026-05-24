# PC Dashboard

Open `index.html` with Chrome or Edge, click `连接串口`, then choose the STM32 USART1 port.

Serial settings:

- Baud rate: `115200`
- Data bits: `8`
- Stop bits: `1`
- Parity: `none`

The firmware sends a machine-readable frame once per second:

```text
ENV,uptime_s,temp_x100,humidity,light_percent,lux
```

Example:

```text
ENV,5,2530,52,63,630
```

The humidity field is read from DHT11 on `PG11`. If DHT11 is not detected or a read fails, the firmware sends `humidity=-1` and the dashboard shows `--`.
