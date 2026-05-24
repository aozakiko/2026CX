#include "system.h"
#include "SysTick.h"
#include "usart.h"
#include "led.h"
#include "tftlcd.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usbd_ctlreq.h"
#include "hw_config.h"
#include "usbd_core.h"



extern USBD_HandleTypeDef hUsbDeviceFS;
extern PCD_HandleTypeDef hpcd_USB_FS;

/*******************************************************************************
* 函 数 名         : main
* 函数功能		   : 主函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
int main()
{
	u16 t;
	u16 len;	
	u16 times=0;    
	u8 usbstatus=0;
	
	HAL_Init();                     //初始化HAL库 
	SystemClock_Init(RCC_PLL_MUL9); //设置时钟,72M
	SysTick_Init(72);
	USART1_Init(115200);
	LED_Init();
	TFTLCD_Init();			//LCD初始化
	
	FRONT_COLOR=RED;
	LCD_ShowString(10,10,tftlcd_data.width,tftlcd_data.height,16,"PRECHIN-STM32F1");
	LCD_ShowString(10,30,tftlcd_data.width,tftlcd_data.height,16,"www.prechin.net");
	LCD_ShowString(10,50,tftlcd_data.width,tftlcd_data.height,16,"USB Virtual USART TEST");
	LCD_ShowString(10,130,tftlcd_data.width,tftlcd_data.height,16,"USB Connecting...");
	
	USB_Reset();//USB断开再重连
    MX_USB_DEVICE_Init();//USB设备初始化
	LCD_ShowString(10,70,200,16,16,"USB inited...");

	while(1)
	{
		if(usbstatus!=USB_GetStatus())//USB连接状态发生了改变.
		{
			usbstatus = USB_GetStatus();//记录新的状态
			if(usbstatus==USBD_STATE_CONFIGURED)
			{
				FRONT_COLOR=BLUE;
				LCD_ShowString(10,130,200,16,16,"USB Connected    ");//提示USB连接成功
				LED2=0;//DS1亮
			}else
			{
				FRONT_COLOR=RED;
				LCD_ShowString(10,130,200,16,16,"USB disConnected ");//提示USB断开
				LED2=1;//DS1灭
			}
		}
		if(USB_USART_RX_STA&0x8000)
		{					   
			len=USB_USART_RX_STA&0x3FFF;//得到此次接收到的数据长度
			USB_Printf("\r\n您发送的消息长度为:%d，消息内容为：\r\n",len);
			for(t=0;t<len;t++)
			{
				USB_USART_SendData(&USB_USART_RX_BUF[t]);//以字节方式,发送给USB 
			}
			USB_Printf("\r\n\r\n");//插入换行
			USB_USART_RX_STA=0;
		}else
		{
			times++;
			if(times%200==0)USB_Printf("请输入数据,以回车键结束\r\n");  
			if(times%30==0)LED1=!LED1;//闪烁LED,提示系统正在运行.
			delay_ms(10);   
		}	
	}
}
