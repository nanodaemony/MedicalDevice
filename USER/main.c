#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "includes.h"
#include "usart2.h"
#include "usart3.h"
#include "common.h"
#include "adc.h"

#include<stdio.h>
#include<stdlib.h>

// UCOSIII中以下优先级用户程序不能使用，ALIENTEK
// 将这些优先级分配给了UCOSIII的5个系统内部任务
// 优先级0：中断服务服务管理任务 OS_IntQTask()
// 优先级1：时钟节拍任务 OS_TickTask()
// 优先级2：定时任务 OS_TmrTask()
// 优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
// 优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()

// 任务优先级
#define START_TASK_PRIO		3
// 任务堆栈大小
#define START_STK_SIZE 		512
// 任务控制块
OS_TCB StartTaskTCB;
// 任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
// 任务函数
void start_task(void *p_arg);

// 任务优先级
#define LED0_TASK_PRIO		4
// 任务堆栈大小	
#define LED0_STK_SIZE 		128
// 任务控制块
OS_TCB Led0TaskTCB;
// 任务堆栈	
CPU_STK LED0_TASK_STK[LED0_STK_SIZE];
// 任务函数
void led0_task(void *p_arg);

// 任务优先级
#define LED1_TASK_PRIO		5
// 任务堆栈大小	
#define LED1_STK_SIZE 		128
// 任务控制块
OS_TCB Led1TaskTCB;
// 任务堆栈	
CPU_STK LED1_TASK_STK[LED1_STK_SIZE];
// 任务函数
void led1_task(void *p_arg);

// 任务优先级
#define FLOAT_TASK_PRIO		6
// 任务堆栈大小
#define FLOAT_STK_SIZE		128
// 任务控制块
OS_TCB	FloatTaskTCB;
// 任务堆栈
__align(8) CPU_STK	FLOAT_TASK_STK[FLOAT_STK_SIZE];
// 任务函数
void float_task(void *p_arg);


// 任务优先级
#define WIFI_TASK_PRIO		7
// 任务堆栈大小	
#define WIFI_STK_SIZE 		128
// 任务控制块
OS_TCB WifiTaskTCB;
// 任务堆栈	
CPU_STK WIFI_TASK_STK[WIFI_STK_SIZE];
// 任务函数
void wifi_task(void *p_arg);

// 任务优先级
#define WIFI_GET_TASK_PRIO		8
// 任务堆栈大小	
#define WIFI_GET_STK_SIZE 		128
// 任务控制块
OS_TCB WifiGetTaskTCB;
// 任务堆栈	
CPU_STK WIFI_GET_TASK_STK[WIFI_GET_STK_SIZE];
// 任务函数
void wifi_get_task(void *p_arg);


// 任务优先级
#define HEART_GET_TASK_PRIO		7
// 任务堆栈大小	
#define HEART_GET_STK_SIZE 		128
// 任务控制块
OS_TCB HeartGetTaskTCB;
// 任务堆栈	
CPU_STK HEART_GET_TASK_STK[HEART_GET_STK_SIZE];
// 任务函数
void heart_get_task(void *p_arg);


// TCP接收数据长度
u16 rlen = 0;
int i;
// 仪器数据接收数据长度
u16 dlen = 0;

// 心电参数
u16 adc1;
int voltage1;

// 存储心电数据的Buff
u8 heartBuff[1024]; 
int h;

int main(void) {
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init();       // 延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 中断分组配置
	uart_init(115200);  // 串口波特率设置
	// 初始化串口2波特率为115200(☆)
	USART2_Init(115200);
	// Init the USART3
	// USART3_Init(115200);  	
	LED_Init();         // LED初始化
	Adc_Init();
	
	OSInit(&err);		// 初始化UCOSIII
	OS_CRITICAL_ENTER();// 进入临界区
	// 创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		// 任务控制块
				 (CPU_CHAR	* )"start task", 		// 任务名字
                 (OS_TASK_PTR )start_task, 			// 任务函数
                 (void		* )0,					// 传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     // 任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	// 任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	// 任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		// 任务堆栈大小
                 (OS_MSG_QTY  )0,					// 任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					// 当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					// 用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, // 任务选项
                 (OS_ERR 	* )&err);				// 存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	// 退出临界区	 
	OSStart(&err);  // 开启UCOSIII
	while(1);
}

// 开始任务函数
void start_task(void *p_arg) {
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	// 统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		// 如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  // 当使用时间片轮转的时候
	 // 使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	// 进入临界区
	// 创建LED0任务
	OSTaskCreate((OS_TCB 	* )&Led0TaskTCB,		
				 (CPU_CHAR	* )"led0 task", 		
                 (OS_TASK_PTR )led0_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LED0_TASK_PRIO,     
                 (CPU_STK   * )&LED0_TASK_STK[0],	
                 (CPU_STK_SIZE)LED0_STK_SIZE/10,	
                 (CPU_STK_SIZE)LED0_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	// 创建LED1任务
	OSTaskCreate((OS_TCB 	* )&Led1TaskTCB,		
				 (CPU_CHAR	* )"led1 task", 		
                 (OS_TASK_PTR )led1_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LED1_TASK_PRIO,     	
                 (CPU_STK   * )&LED1_TASK_STK[0],	
                 (CPU_STK_SIZE)LED1_STK_SIZE/10,	
                 (CPU_STK_SIZE)LED1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);	
			 
	// 创建浮点测试任务
	OSTaskCreate((OS_TCB 	* )&FloatTaskTCB,		
				 (CPU_CHAR	* )"float test task", 		
                 (OS_TASK_PTR )float_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )FLOAT_TASK_PRIO,     	
                 (CPU_STK   * )&FLOAT_TASK_STK[0],	
                 (CPU_STK_SIZE)FLOAT_STK_SIZE/10,	
                 (CPU_STK_SIZE)FLOAT_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);							

	// 创建WIFI任务
	OSTaskCreate((OS_TCB 	* )&WifiTaskTCB,		
				 (CPU_CHAR	* )"Wifi task", 		
                 (OS_TASK_PTR )wifi_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )WIFI_TASK_PRIO,     	
                 (CPU_STK   * )&WIFI_TASK_STK[0],	
                 (CPU_STK_SIZE)WIFI_STK_SIZE/10,	
                 (CPU_STK_SIZE)WIFI_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);	
				 
	// 创建WIFI_GET任务
	OSTaskCreate((OS_TCB 	* )&WifiGetTaskTCB,		
				 (CPU_CHAR	* )"Wifi Get task", 		
                 (OS_TASK_PTR )wifi_get_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )WIFI_GET_TASK_PRIO,     	
                 (CPU_STK   * )&WIFI_GET_TASK_STK[0],	
                 (CPU_STK_SIZE)WIFI_GET_STK_SIZE/10,	
                 (CPU_STK_SIZE)WIFI_GET_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);	
				 
	// 创建HEART_GET任务
	OSTaskCreate((OS_TCB 	* )&HeartGetTaskTCB,		
				 (CPU_CHAR	* )"Heart Get task", 		
                 (OS_TASK_PTR )heart_get_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )HEART_GET_TASK_PRIO,     	
                 (CPU_STK   * )&HEART_GET_TASK_STK[0],	
                 (CPU_STK_SIZE)HEART_GET_STK_SIZE/10,	
                 (CPU_STK_SIZE)HEART_GET_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);	
				 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB, &err);		// 挂起开始任务			 
	OS_CRITICAL_EXIT();	// 进入临界区
}

//led0任务函数
void led0_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	while(1) {
		LED0 = 0;
		OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err); // 延时200ms
		LED0 = 1;
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); // 延时500ms
	}
}

//led1任务函数
void led1_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	while(1) {
		LED1 = ~LED1; 
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); // 延时500ms
		
	}
}

// Wifi任务函数
void wifi_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	
	// 显示测试界面主UI布局
	delay_ms(1000);
	printf("Start wifi test.\r\n");
	// 检查WIFI模块是否在线
	while(atk_8266_send_cmd("AT", "OK", 20)) {
		// 退出透传(然后进入AT指令模式)
		atk_8266_quit_trans();
		// 关闭透传模式	
		atk_8266_send_cmd("AT+CIPMODE=0", "OK", 200);  
		printf("未检测到模块!!!");
		delay_ms(800);
		printf("尝试连接模块..."); 
	} 
	printf("Wifi module is connected.\r\n");
	// 关闭回显
	while(atk_8266_send_cmd("ATE0", "OK", 20));
	
	// WIFI STA测试
	// wifista_test();
	delay_ms(1000);
	atk_8266_send_cmd("AT+CWMODE=1", "OK", 50);
	delay_ms(100);
	atk_8266_send_cmd("AT+RST", "OK", 20);	
	// Wait to reboot.
	delay_ms(1000);         
	delay_ms(1000);
	delay_ms(1000);
	printf("Start to connect to Hotspot.\r\n");
	atk_8266_send_cmd("AT+CWJAP=\"DataCollector\",\"1357924680\"", "OK", 100);	
	printf("Finish to connect to Hotspot.\r\n");
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	atk_8266_send_cmd("AT+CIPMUX=0", "OK", 20);   
	delay_ms(5000);
	printf("Start connect to server.\r\n");
	atk_8266_send_cmd("AT+CIPSTART=\"TCP\",\"192.168.8.100\",10086", "OK", 200);
	printf("Finish connect to server.\r\n");
	delay_ms(5000);
	atk_8266_send_cmd("AT+CIPMODE=1", "OK", 200); 
	delay_ms(1000);	
	// 开始传输数据
	atk_8266_send_cmd("AT+CIPSEND", "OK", 20); 
	while(1) {
		delay_ms(1000);
		// atk_8266_quit_trans();
		// atk_8266_send_cmd("AT+CIPSEND", "OK", 20); 
		// atk_8266_send_data("This is from ESP82661298012", "OK", 10);
	}
}


// Wifi任务函数
void wifi_get_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	// 防止影响开始的初始化工作
	for(i = 0; i < 30; i++) {
		delay_ms(1000);
	}
	
	while(1) {
		if(USART2_RX_STA & 0X8000) { 
			// 得到本次接收到的数据长度
			rlen = USART2_RX_STA & 0X7FFF;	
			// 添加结束符 
			USART2_RX_BUF[rlen] = 0;		
			// 发送到串口  
			printf("%s\r\n", USART2_RX_BUF);				
			// 串口状态置位
			USART2_RX_STA = 0;
			
		}
	}
}


// Heart心电任务函数
void heart_get_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	// 防止影响开始的初始化工作
	for(i = 0; i < 40; i++) {
		delay_ms(1000);
	}
	while(1) {
		// delay_ms(10);
		adc1 = Get_Adc_Average(10, 10);
		voltage1 = adc1 / 4;
		printf("%d\r\n", voltage1);
		u2_printf("#%d", voltage1);
	}
}


// 仪器数据获取任务函数
void device_get_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	while(1) {
		// delay_ms(1000);
		// u3_printf("123123\r\n");
		if(USART3_RX_STA & 0X8000) { 
			// 得到本次接收到的数据长度
			dlen = USART3_RX_STA & 0X7FFF;	
			// 添加结束符 
			USART3_RX_BUF[dlen] = 0;		
			// 发送到串口  
			atk_8266_send_data(USART3_RX_BUF, "OK", 10);
			u3_printf("Device Data: %s\r\n", USART3_RX_BUF);	 
			// 串口状态置位
			USART3_RX_STA = 0;
		}
	}
}

// 浮点测试任务
void float_task(void *p_arg) {
	CPU_SR_ALLOC();
	static float float_num=0.01;
	while(1) {
		float_num += 0.01f;
//		OS_CRITICAL_ENTER();	// 进入临界区
//		printf("float_num的值为: %.4f\r\n", float_num);
//		OS_CRITICAL_EXIT();		// 退出临界区
		delay_ms(500);			// 延时500ms
	}
}







