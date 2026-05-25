# 手机 MQTT 环境监测前端

本前端通过 MQTT over WebSocket 订阅 STM32 发布的环境数据，适合手机浏览器访问。

## 默认连接

- Broker WebSocket：`ws://192.168.166.177:9001/mqtt`
- 完整数据 Topic：`stm32/env`
- 温度 Topic：`stm32/temperature`
- 湿度 Topic：`stm32/humidity`

STM32 端仍连接 MQTT TCP 端口 `1883` 发布数据；手机浏览器需要通过 Broker 的 WebSocket 端口访问，不能直接连接 `1883`。

## Mosquitto WebSocket 示例配置

```conf
listener 1883
allow_anonymous true

listener 9001
protocol websockets
allow_anonymous true
```

修改配置后重启 Mosquitto，并确认 Windows 防火墙允许 `1883` 和 `9001` 入站连接。手机、PC Broker 和 STM32/ESP8266 需要在同一局域网或同一个热点下。

## 数据格式

`08-温湿度光照环境监测实验/Project/APP/esp8266/wifi_function.c` 中的 `ESP8266_MQTT_Publish_Environment()` 会发布：

```text
stm32/env          uptime=12,temp=25.34,temp_x100=2534,humidity=52,light=63,lux=630
stm32/temperature 25.34
stm32/humidity    52
```

当前页面也兼容串口帧格式：

```text
ENV,uptime_s,temp_x100,humidity,light_percent,lux
```

如果 DHT11 未连接或读取失败，STM32 会发布 `humidity=-1`，页面显示为 `--`。

## 运行

页面使用本地 `mqtt.min.js`，手机只要能访问本目录页面和 Broker WebSocket 端口即可。可直接打开 `index.html`。若要让手机访问电脑上的页面，可在本目录启动静态服务：

```powershell
python -m http.server 8080
```

然后在手机浏览器打开 `http://电脑IP:8080/`。
