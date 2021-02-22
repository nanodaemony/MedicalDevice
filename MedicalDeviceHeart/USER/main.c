#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "includes.h"
#include "usart2.h"
#include "usart3.h"
#include "common.h"
#include "adc.h"
#include "max30102.h" 
#include "myiic.h"
#include "algorithm.h"
#include "oled.h"

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
#define COMMAND_GET_TASK_PRIO		4
// �����ջ��С	
#define COMMAND_GET_STK_SIZE 		128
// ������ƿ�
OS_TCB CommandGetTaskTCB;
// �����ջ	
CPU_STK COMMAND_GET_TASK_STK[COMMAND_GET_STK_SIZE];
// ������
void command_get_task(void *p_arg);


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


// �������ȼ�
#define BLOOD_GET_TASK_PRIO		6
// �����ջ��С	
#define BLOOD_GET_STK_SIZE 		512
// ������ƿ�
OS_TCB BloodGetTaskTCB;
// �����ջ	
CPU_STK BLOOD_GET_TASK_STK[BLOOD_GET_STK_SIZE];
// ������
void blood_get_task(void *p_arg);


// �������ȼ�
#define OLED_TASK_PRIO		5
// �����ջ��С	
#define OLED_STK_SIZE 		512
// ������ƿ�
OS_TCB OLEDTaskTCB;
// �����ջ	
CPU_STK OLED_TASK_STK[OLED_STK_SIZE];
// ������
void oled_task(void *p_arg);


// TCP�������ݳ���
u16 rlen = 0;
int i;
// Ѫ�����ݽ������ݳ���
u16 dlen = 0;

// �ĵ����
u16 adc1;
int voltage1;
// �洢�ĵ����ݵ�Buff
u8 heartBuff[1024];
int h;
// �����ĵ�����
char data[1000];
int indexData;

// ��ǰ�Ƿ���Խ���ָ��
int isCanReceiveCommand = 1;
// �Ƿ����ĵ����� Ĭ�ϲ�����
int isSendHeartData = 1;
// �Ƿ���Ѫ������ Ĭ�ϲ�����
int isSendBloodData = 1;
// �Ƿ���ɳ�ʼ��
int isFinishInit = 0;

u8 *bd;

u8 test = 0;

char hr[7];
char hrValid[2];
char spo2[7];
char spo2Valid[2];
int atCnt;
int k;
int temp;

int main(void) {
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init();       // ��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // �жϷ�������
    
    // ��ʼ��OLED 
    OLED_Init();
    OLED_ShowString(0, 0, "NanoDevice", 24);     
    OLED_ShowString(0, 52, "   By CZ(msc 206)", 8);      
    // ������ʾ��OLED
    OLED_Refresh_Gram();
	uart_init(115200);  // ���ڲ���������
	// ��ʼ������2������Ϊ115200(��)
	USART2_Init(115200);
	// Init the USART3
	USART3_Init(115200);  	
	LED_Init();         // LED��ʼ��
	Adc_Init();
	
	// ��ʼ��Ѫ��ģ��
	max30102_init();

	// ��ʾ���Խ�����UI����
	delay_ms(1000);
	printf("Device init...\r\n");
    OLED_ShowString(0, 30, "Initializing...", 12);  
    OLED_Refresh_Gram();
	// ���WIFIģ���Ƿ�����
	while(atk_8266_send_cmd("AT", "OK", 20)) {
		// �˳�͸��(Ȼ�����ATָ��ģʽ)
		atk_8266_quit_trans();
		// �ر�͸��ģʽ	
		atk_8266_send_cmd("AT+CIPMODE=0", "OK", 200);  
		printf("δ��⵽ģ��!!!");
        OLED_ShowString(0, 30, "No WiFi Module.", 12);
        OLED_Refresh_Gram();
		delay_ms(800);
		printf("��������ģ��..."); 
	} 
    OLED_ShowString(0, 30, "ESP8266 Connected!", 12);
    OLED_Refresh_Gram();
	printf("Wifi module is connected.\r\n");
	// �رջ���
	while(atk_8266_send_cmd("ATE0", "OK", 20));
    
	delay_ms(1000);
	atk_8266_send_cmd("AT+CWMODE=1", "OK", 50);
	delay_ms(100);
	atk_8266_send_cmd("AT+RST", "OK", 20);	
	// Wait to reboot.
	delay_ms(1000);         
	delay_ms(1000);
    // ����WiFi�ȵ�
    OLED_ShowString(0, 30, "Connecting WiFi...", 12);
    OLED_Refresh_Gram();
	printf("Start to connect to Hotspot.\r\n");
    // ��������ֱ���ɹ�
    while(atk_8266_send_cmd("AT+CWJAP=\"DataCollector\",\"1357924680\"", "OK", 100)) {
        delay_ms(1000);
        printf("Trying to connect to Wifi...\r\n");
        OLED_ShowString(0, 30, "Retry Connect WiFi...", 12);
        OLED_Refresh_Gram();
    }
	printf("�ɹ�����WiFi.\r\n");
    OLED_ShowString(0, 30, "WiFi connected!!!     ", 12);   
    OLED_Refresh_Gram();
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	atk_8266_send_cmd("AT+CIPMUX=0", "OK", 20);   
	delay_ms(5000);
    
	printf("��ʼ���ӷ�����.\r\n");
    OLED_ShowString(0, 30, "Connecting server...", 12);   
    OLED_Refresh_Gram();
    // ��Ŀ��������ַ: 39.98.122.209
    // �ʼǱ����Ե�ַ (�����ֻ�): 192.168.43.204
    while(atk_8266_send_cmd("AT+CIPSTART=\"TCP\",\"192.168.43.204\",10087", "OK", 200)) {
        delay_ms(1000);
        printf("Trying to connect to Server...\r\n");
        OLED_ShowString(0, 30, "Retry Connect server...", 12);
        OLED_Refresh_Gram();
    }
	printf("������������.\r\n");
    OLED_ShowString(0, 30, "Server Connected!!!        ", 12);   
    OLED_Refresh_Gram();
	delay_ms(3000);
	atk_8266_send_cmd("AT+CIPMODE=1", "OK", 200); 
	delay_ms(1000);	
	// ��ʼ��������
	atk_8266_send_cmd("AT+CIPSEND", "OK", 20); 
	printf("��ʼ��UCOS...");
    // ���浱ǰWIFIģ�����͸��,��ʱ���Խ���ָ��������
    isCanReceiveCommand = 0;
    isFinishInit = 1;
    OLED_ShowString(0, 30, "                     ", 12);
    OLED_ShowString(0, 30, "HR: ", 12);   
    OLED_ShowString(84, 30, "Valid: ", 12); 
    OLED_ShowString(0, 45, "SpO2: ", 12);   
    OLED_ShowString(84, 45, "Valid: ", 12); 
    OLED_Refresh_Gram();
    delay_ms(500);
    
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
				 				
 
	// ����WIFI_GET����
	OSTaskCreate((OS_TCB 	* )&CommandGetTaskTCB,		
				 (CPU_CHAR	* )"Command Get task", 		
                 (OS_TASK_PTR )command_get_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )COMMAND_GET_TASK_PRIO,     	
                 (CPU_STK   * )&COMMAND_GET_TASK_STK[0],	
                 (CPU_STK_SIZE)COMMAND_GET_STK_SIZE/10,	
                 (CPU_STK_SIZE)COMMAND_GET_STK_SIZE,		
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
				 
	// ����BLOOD_GET����
	OSTaskCreate((OS_TCB 	* )&BloodGetTaskTCB,		
				 (CPU_CHAR	* )"Blood Get task", 		
                 (OS_TASK_PTR )blood_get_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )BLOOD_GET_TASK_PRIO,     	
                 (CPU_STK   * )&BLOOD_GET_TASK_STK[0],	
                 (CPU_STK_SIZE)BLOOD_GET_STK_SIZE/10,	
                 (CPU_STK_SIZE)BLOOD_GET_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);	
                 
	// ����OLED����
	OSTaskCreate((OS_TCB 	* )&OLEDTaskTCB,		
				 (CPU_CHAR	* )"OLED task", 		
                 (OS_TASK_PTR )oled_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )OLED_TASK_PRIO,     	
                 (CPU_STK   * )&OLED_TASK_STK[0],	
                 (CPU_STK_SIZE)OLED_STK_SIZE/10,	
                 (CPU_STK_SIZE)OLED_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);	                 
				             
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB, &err);		// ����ʼ����			 
	OS_CRITICAL_EXIT();	// �����ٽ���
}

// led0������
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

// oled������
void oled_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
    // ����������Ϣ
	while(1) {
		// ������Ϣ
		bd = OSTaskQPend((OS_TICK		)0,
                      (OS_OPT		)OS_OPT_PEND_BLOCKING,
                      (OS_MSG_SIZE*	)10,
                      (CPU_TS*		)0,
                      (OS_ERR*      )&err);
		delay_ms(100);
        
        OLED_ShowString(30, 30, hr, 12);   
        OLED_ShowString(120, 30, hrValid, 12); 
        OLED_ShowString(42, 45, spo2, 12);   
        OLED_ShowString(120, 45, spo2Valid, 12);
        
        // OLED_ShowString(0, 30, hr, 12);
        // test = test + 3;
        OLED_Refresh_Gram();
	}
}


// Wifi������
void command_get_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	// ��ֹӰ�쿪ʼ�ĳ�ʼ������,������ʱ���Խ��տ���������
    while(isCanReceiveCommand) {
        delay_ms(1000);
    }
    
	while(1) {
		delay_ms(100);
        // ע�⣡������֪��Ϊʲô����������һ�λظ��������յ���������ATָ�����Ż�����������
        // ������Ϊ�������ʱ����̫����!!!!
		if(USART2_RX_STA & 0X8000) { 
			// �õ����ν��յ������ݳ���
			rlen = USART2_RX_STA & 0X7FFF;	
			// ��ӽ����� 
			USART2_RX_BUF[rlen] = 0;
			// ���͵�����  
			printf("%s\r\n", USART2_RX_BUF);			
            printf("%c\r\n", USART2_RX_BUF[0]);
            // #�ſ�ͷ˵���ǺϷ�����
            if(USART2_RX_BUF[0] - '#' == 0) {
                // #1ȫ���ɼ�
                if(USART2_RX_BUF[1] - '1' == 0) {
                    isSendHeartData = 1;
                    isSendBloodData = 1;
                } else if (USART2_RX_BUF[1] - '3' == 0) {
                    isSendHeartData = 1;
                } else if (USART2_RX_BUF[1] - '4' == 0) {
                    isSendHeartData = 0;
                    delay_ms(100);
                } else if (USART2_RX_BUF[1] - '5' == 0) {
                    isSendBloodData = 1;
                } else if (USART2_RX_BUF[1] - '6' == 0) {
                    isSendBloodData = 0;
                    OLED_ShowString(42, 45, "---", 12);   
                    OLED_ShowString(120, 45, "-", 12);
                    OLED_ShowString(30, 30, "---", 12);   
                    OLED_ShowString(120, 30, "-", 12);
                    OLED_Refresh_Gram();
                    delay_ms(100);
                }
            }
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
	for(i = 0; i < 50; i++) {
		delay_ms(1000);
	}
	while(1) {
		// ����������ʱ,�ⲿ��Ҫ�ٴ���ʱ��
		adc1 = Get_Adc_Average(10, 10);
		voltage1 = adc1 >> 2;
		printf("%d\r\n", voltage1);
		//u2_printf("#%d\r\n", voltage1);
		while(voltage1 != 0) {
			data[indexData] = (char)(voltage1 % 10 + 48);
			voltage1 = voltage1 / 10;
			indexData++;
		}
		data[indexData] = '#';
		indexData++;
		if(indexData > 1000) {
            // ��������ĵ�����
            if(isSendHeartData) {
                u2_printf("%s\r\n", data);
            }
			delay_ms(10);
			indexData = 0;
		}
	}
}

// ���ݻ�ȡ������
void blood_get_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	while(1) {
		delay_ms(100);
		if(USART3_RX_STA & 0X8000) { 
			// �õ����ν��յ������ݳ���
			dlen = USART3_RX_STA & 0X7FFF;	
			// ��ӽ����� 
			USART3_RX_BUF[dlen] = 0;
			// ���͵�����
			// atk_8266_send_data(USART3_RX_BUF, "OK", 10);
			delay_ms(10);
            if(isSendBloodData & isFinishInit) {
                u2_printf("%s\r\n", USART3_RX_BUF);
                // OLED_ShowString(0, 30, USART3_RX_BUF, 12);
                
                atCnt = 0;
                temp = 0;
                for(k = 0; k < dlen; k++) { 
                    if(USART3_RX_BUF[k] - '@' == 0) {
                        temp = 0;
                        atCnt++;
                        if(atCnt == 5) break;
                        continue;
                    }
                    if(atCnt == 1) {
                        hr[temp] = USART3_RX_BUF[k];
                        temp++;
                        hr[temp] = ' ';
                        hr[temp + 1] = ' ';
                        hr[temp + 2] = 0;
                    } else if (atCnt == 2) {
                        hrValid[temp] = USART3_RX_BUF[k];
                        temp++;
                        hrValid[temp] = 0;
                    } else if (atCnt == 3) {
                        spo2[temp] = USART3_RX_BUF[k];
                        temp++;
                        spo2[temp] = ' ';
                        spo2[temp + 1] = ' ';
                        spo2[temp + 2] = 0;
                    } else if (atCnt == 4) {
                        spo2Valid[temp] = USART3_RX_BUF[k];
                        temp++;
                        spo2Valid[temp] = 0;
                    }
                }
                printf("++%s", hr);
                printf("++%s", spo2);
                //������Ϣ
                OSTaskQPost((OS_TCB*	)&OLEDTaskTCB,	//������Msgdis������Ϣ
                    (void*		)USART3_RX_BUF,
                    (OS_MSG_SIZE)10,
                    (OS_OPT		)OS_OPT_POST_FIFO,
					(OS_ERR*	)&err);
            }
			printf("Blood:%s\r\n", USART3_RX_BUF);	 
			// ����״̬��λ
			USART3_RX_STA = 0;
		}
	}
}


