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

// UCOSIII���������ȼ��û�������ʹ�ã�ALIENTEK
// ����Щ���ȼ��������UCOSIII��5��ϵͳ�ڲ�����
// ���ȼ�0���жϷ������������� OS_IntQTask()
// ���ȼ�1��ʱ�ӽ������� OS_TickTask()
// ���ȼ�2����ʱ���� OS_TmrTask()
// ���ȼ�OS_CFG_PRIO_MAX-2��ͳ������ OS_StatTask()
// ���ȼ�OS_CFG_PRIO_MAX-1���������� OS_IdleTask()

// �������ȼ�
#define START_TASK_PRIO		3
// �����ջ��С
#define START_STK_SIZE 		512
// ������ƿ�
OS_TCB StartTaskTCB;
// �����ջ	
CPU_STK START_TASK_STK[START_STK_SIZE];
// ������
void start_task(void *p_arg);

// �������ȼ�
#define LED0_TASK_PRIO		4
// �����ջ��С	
#define LED0_STK_SIZE 		128
// ������ƿ�
OS_TCB Led0TaskTCB;
// �����ջ	
CPU_STK LED0_TASK_STK[LED0_STK_SIZE];
// ������
void led0_task(void *p_arg);

// �������ȼ�
#define LED1_TASK_PRIO		5
// �����ջ��С	
#define LED1_STK_SIZE 		128
// ������ƿ�
OS_TCB Led1TaskTCB;
// �����ջ	
CPU_STK LED1_TASK_STK[LED1_STK_SIZE];
// ������
void led1_task(void *p_arg);

// �������ȼ�
#define FLOAT_TASK_PRIO		6
// �����ջ��С
#define FLOAT_STK_SIZE		128
// ������ƿ�
OS_TCB	FloatTaskTCB;
// �����ջ
__align(8) CPU_STK	FLOAT_TASK_STK[FLOAT_STK_SIZE];
// ������
void float_task(void *p_arg);


// �������ȼ�
#define WIFI_TASK_PRIO		7
// �����ջ��С	
#define WIFI_STK_SIZE 		128
// ������ƿ�
OS_TCB WifiTaskTCB;
// �����ջ	
CPU_STK WIFI_TASK_STK[WIFI_STK_SIZE];
// ������
void wifi_task(void *p_arg);

// �������ȼ�
#define WIFI_GET_TASK_PRIO		8
// �����ջ��С	
#define WIFI_GET_STK_SIZE 		128
// ������ƿ�
OS_TCB WifiGetTaskTCB;
// �����ջ	
CPU_STK WIFI_GET_TASK_STK[WIFI_GET_STK_SIZE];
// ������
void wifi_get_task(void *p_arg);


// �������ȼ�
#define HEART_GET_TASK_PRIO		7
// �����ջ��С	
#define HEART_GET_STK_SIZE 		128
// ������ƿ�
OS_TCB HeartGetTaskTCB;
// �����ջ	
CPU_STK HEART_GET_TASK_STK[HEART_GET_STK_SIZE];
// ������
void heart_get_task(void *p_arg);


// TCP�������ݳ���
u16 rlen = 0;
int i;
// �������ݽ������ݳ���
u16 dlen = 0;

// �ĵ����
u16 adc1;
int voltage1;

// �洢�ĵ����ݵ�Buff
u8 heartBuff[1024]; 
int h;

int main(void) {
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init();       // ��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // �жϷ�������
	uart_init(115200);  // ���ڲ���������
	// ��ʼ������2������Ϊ115200(��)
	USART2_Init(115200);
	// Init the USART3
	// USART3_Init(115200);  	
	LED_Init();         // LED��ʼ��
	Adc_Init();
	
	OSInit(&err);		// ��ʼ��UCOSIII
	OS_CRITICAL_ENTER();// �����ٽ���
	// ������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		// ������ƿ�
				 (CPU_CHAR	* )"start task", 		// ��������
                 (OS_TASK_PTR )start_task, 			// ������
                 (void		* )0,					// ���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     // �������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	// �����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	// �����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		// �����ջ��С
                 (OS_MSG_QTY  )0,					// �����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					// ��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					// �û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, // ����ѡ��
                 (OS_ERR 	* )&err);				// ��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	// �˳��ٽ���	 
	OSStart(&err);  // ����UCOSIII
	while(1);
}

// ��ʼ������
void start_task(void *p_arg) {
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	// ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		// ���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  // ��ʹ��ʱ��Ƭ��ת��ʱ��
	 // ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	// �����ٽ���
	// ����LED0����
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
				 
	// ����LED1����
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
			 
	// ���������������
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

	// ����WIFI����
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
				 
	// ����WIFI_GET����
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
				 
	// ����HEART_GET����
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
				 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB, &err);		// ����ʼ����			 
	OS_CRITICAL_EXIT();	// �����ٽ���
}

//led0������
void led0_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	while(1) {
		LED0 = 0;
		OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err); // ��ʱ200ms
		LED0 = 1;
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); // ��ʱ500ms
	}
}

//led1������
void led1_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	while(1) {
		LED1 = ~LED1; 
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); // ��ʱ500ms
		
	}
}

// Wifi������
void wifi_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	
	// ��ʾ���Խ�����UI����
	delay_ms(1000);
	printf("Start wifi test.\r\n");
	// ���WIFIģ���Ƿ�����
	while(atk_8266_send_cmd("AT", "OK", 20)) {
		// �˳�͸��(Ȼ�����ATָ��ģʽ)
		atk_8266_quit_trans();
		// �ر�͸��ģʽ	
		atk_8266_send_cmd("AT+CIPMODE=0", "OK", 200);  
		printf("δ��⵽ģ��!!!");
		delay_ms(800);
		printf("��������ģ��..."); 
	} 
	printf("Wifi module is connected.\r\n");
	// �رջ���
	while(atk_8266_send_cmd("ATE0", "OK", 20));
	
	// WIFI STA����
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
	// ��ʼ��������
	atk_8266_send_cmd("AT+CIPSEND", "OK", 20); 
	while(1) {
		delay_ms(1000);
		// atk_8266_quit_trans();
		// atk_8266_send_cmd("AT+CIPSEND", "OK", 20); 
		// atk_8266_send_data("This is from ESP82661298012", "OK", 10);
	}
}


// Wifi������
void wifi_get_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	// ��ֹӰ�쿪ʼ�ĳ�ʼ������
	for(i = 0; i < 30; i++) {
		delay_ms(1000);
	}
	
	while(1) {
		if(USART2_RX_STA & 0X8000) { 
			// �õ����ν��յ������ݳ���
			rlen = USART2_RX_STA & 0X7FFF;	
			// ��ӽ����� 
			USART2_RX_BUF[rlen] = 0;		
			// ���͵�����  
			printf("%s\r\n", USART2_RX_BUF);				
			// ����״̬��λ
			USART2_RX_STA = 0;
			
		}
	}
}


// Heart�ĵ�������
void heart_get_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	// ��ֹӰ�쿪ʼ�ĳ�ʼ������
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


// �������ݻ�ȡ������
void device_get_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	while(1) {
		// delay_ms(1000);
		// u3_printf("123123\r\n");
		if(USART3_RX_STA & 0X8000) { 
			// �õ����ν��յ������ݳ���
			dlen = USART3_RX_STA & 0X7FFF;	
			// ��ӽ����� 
			USART3_RX_BUF[dlen] = 0;		
			// ���͵�����  
			atk_8266_send_data(USART3_RX_BUF, "OK", 10);
			u3_printf("Device Data: %s\r\n", USART3_RX_BUF);	 
			// ����״̬��λ
			USART3_RX_STA = 0;
		}
	}
}

// �����������
void float_task(void *p_arg) {
	CPU_SR_ALLOC();
	static float float_num=0.01;
	while(1) {
		float_num += 0.01f;
//		OS_CRITICAL_ENTER();	// �����ٽ���
//		printf("float_num��ֵΪ: %.4f\r\n", float_num);
//		OS_CRITICAL_EXIT();		// �˳��ٽ���
		delay_ms(500);			// ��ʱ500ms
	}
}







