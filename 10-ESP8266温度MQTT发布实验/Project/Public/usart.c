#include "usart.h"
#include "string.h"  // 添加字符串处理头文件	
#include "stm32f10x.h"  // 包含STM32F10x的头文件
#include "stm32f10x_usart.h"  // 包含USART相关的头文件
#include "stm32f10x_gpio.h"  // 包含GPIO相关的头文件
 

int fputc(int ch,FILE *p)  //函数默认的，在使用printf函数时自动调用
{
	USART_SendData(USART1,(u8)ch);	
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	return ch;
}

//使用scanf时需要关闭USART1中断功能
//重定向c库函数scanf到USART1
int fgetc(FILE *f)
{
	/* 等待串口1输入数据 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

	return (int)USART_ReceiveData(USART1);
}

//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART1_RX_BUF[USART1_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART1_RX_STA=0;       //接收状态标记


/*******************************************************************************
* 函 数 名         : USART1_Init
* 函数功能		   : USART1初始化函数
* 输    入         : bound:波特率
* 输    出         : 无
*******************************************************************************/ 
void USART1_Init(u32 bound)
{
   //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
 
	
	/*  配置GPIO的模式和IO口 */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;//TX			   //串口输出PA9
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;	    //复用推挽输出
	GPIO_Init(GPIOA,&GPIO_InitStructure);  /* 初始化串口输入IO */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;//RX			 //串口输入PA10
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;		  //模拟输入
	GPIO_Init(GPIOA,&GPIO_InitStructure); /* 初始化GPIO */
	

	//USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
	USART_Cmd(USART1, ENABLE);  //使能串口1 
	
	USART_ClearFlag(USART1, USART_FLAG_TC);
		
	// 启用接收中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、	
}
/*******************************************************************************
* 函 数 名         : USART1_IRQHandler
* 函数功能		   : USART1中断函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/ 
// 注意: 该函数会在USART1接收到数据时被调用



// 假设这些宏定义已经存在于合适的地方

// extern vu16 USART1_RX_STA; // 接收状态标记
// extern u8 USART1_RX_BUF[]; // 接收缓冲区


void USART1_IRQHandler(void)
{
    u8 r;
    const char prefix[] = "response text:";
    uint16_t i, len;

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        r = USART_ReceiveData(USART1);

        if ((USART1_RX_STA & 0x8000) == 0) // 接收未完成
        {
            if (USART1_RX_STA & 0x4000) // 已收到0x0D
            {
                if (r != 0x0A) // 未收到0x0A
                {
                    USART1_RX_STA = 0; // 重置接收状态
                }
                else // 收到完整帧（0x0D 0x0A结尾）
                {
                    USART1_RX_STA |= 0x8000; // 标记接收完成
                    len = USART1_RX_STA & 0x3FFF; // 计算数据长度

                    // 发送前缀
                    for (i = 0; i < strlen(prefix); i++)
                    {
                        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
                        USART_SendData(USART1, prefix[i]);
                    }

                    // 发送空格分隔符
                    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
                    USART_SendData(USART1, ' ');

                    // 发送接收到的数据（修复索引偏移）
                    for (i = 0; i < len; i++)
                    {
                        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
                        USART_SendData(USART1, USART1_RX_BUF[i]);
                    }

                    // 重置状态和缓冲区
                    USART1_RX_STA = 0;
                    memset(USART1_RX_BUF, 0, USART1_REC_LEN);
                }
            }
            else // 未收到0x0D
            {
                if (r == 0x0D) // 检测到回车符
                {
                    USART1_RX_STA |= 0x4000; // 标记已收到0x0D
                }
                else // 存储普通数据
                {
                    // 修复点：先存入数据，再递增索引
                    if ((USART1_RX_STA & 0x3FFF) < USART1_REC_LEN)
                    {
                        USART1_RX_BUF[USART1_RX_STA & 0x3FFF] = r; // 先存数据
                        USART1_RX_STA++; // 后递增索引
                    }
                    else // 缓冲区溢出处理
                    {
                        USART1_RX_STA = 0;
                        memset(USART1_RX_BUF, 0, USART1_REC_LEN);
                    }
                }
            }
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE); // 清除中断标志
    }
}