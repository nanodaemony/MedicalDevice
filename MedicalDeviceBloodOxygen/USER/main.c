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
#define COMMAND_TASK_PRIO		4
// �����ջ��С	
#define COMMAND_STK_SIZE 		128
// ������ƿ�
OS_TCB CommandTaskTCB;
// �����ջ	
CPU_STK COMMAND_TASK_STK[COMMAND_STK_SIZE];
// ������
void command_task(void *p_arg);


// �������ȼ�
#define BLOOD_GET_TASK_PRIO		5
// �����ջ��С	
#define BLOOD_GET_STK_SIZE 		512
// ������ƿ�
OS_TCB BloodGetTaskTCB;
// �����ջ	
CPU_STK BLOOD_GET_TASK_STK[BLOOD_GET_STK_SIZE];
// ������
void blood_get_task(void *p_arg);


// USART2�������ݳ���
u16 rlen = 0;
int i;

// Ѫ����صĶ���
#define MAX_BRIGHTNESS 255
// IR LED sensor data
uint32_t aun_ir_buffer[500];   
// data length
int32_t n_ir_buffer_length;    
// Red LED sensor data
uint32_t aun_red_buffer[500];  
// SPO2 value
int32_t n_sp02;    
// indicator to show if the SP02 calculation is valid
int8_t ch_spo2_valid;   
// heart rate value
int32_t n_heart_rate;   
// indicator to show if the heart rate calculation is valid
int8_t  ch_hr_valid;    
uint8_t uch_dummy;

// variables to calculate the on-board LED brightness that reflects the heartbeats
uint32_t un_min, un_max, un_prev_data;  
int i;
int32_t n_brightness;
float f_temp;
u8 temp_num = 0;
u8 temp[6];
u8 str[100];
u8 dis_hr = 0, dis_spo2 = 0;


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
	
	// ��ʼ��Ѫ��ģ��
	max30102_init();
	
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


	// ����Command��������
	OSTaskCreate((OS_TCB 	* )&CommandTaskTCB,		
				 (CPU_CHAR	* )"Command task", 		
                 (OS_TASK_PTR )command_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )COMMAND_TASK_PRIO,     
                 (CPU_STK   * )&COMMAND_TASK_STK[0],	
                 (CPU_STK_SIZE)COMMAND_STK_SIZE/10,	
                 (CPU_STK_SIZE)COMMAND_STK_SIZE,		
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


// Usart2������
void command_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	
	while(1) {
		delay_ms(1000);
		if(USART2_RX_STA & 0X8000) { 
			// �õ����ν��յ������ݳ���
			rlen = USART2_RX_STA & 0X7FFF;	
			// ��ӽ����� 
			USART2_RX_BUF[rlen] = 0;		
			// ���͵�����  
			printf("%s\r\n", USART2_RX_BUF);	
			
			// ˵�����յ�������,����������

			
			// ����״̬��λ
			USART2_RX_STA = 0;
			
		}
	}
}


// BloodѪ��������
void blood_get_task(void *p_arg) {
	OS_ERR err;
	p_arg = p_arg;
	printf("Wait Start.");
	// ��ֹӰ�쿪ʼ�ĳ�ʼ������
	for(i = 0; i < 30; i++) {
		delay_ms(1000);
	}
	un_min = 0x3FFFF;
	un_max = 0;
	// buffer length of 100 stores 5 seconds of samples running at 100sps
	n_ir_buffer_length = 500; 
	// read the first 500 samples, and determine the signal range
    for(i = 0; i < n_ir_buffer_length; i++) {
		// wait until the interrupt pin asserts
        while(MAX30102_INT == 1);   
		max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);
		// Combine values to get the actual number
		aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    
		// Combine values to get the actual number
		aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) | (long)temp[4]<<8 | (long)temp[5];   
        if(un_min>aun_red_buffer[i])
			// update signal min
            un_min=aun_red_buffer[i];    
        if(un_max<aun_red_buffer[i])
			// update signal max
            un_max=aun_red_buffer[i];    
    }
	un_prev_data=aun_red_buffer[i];
	// calculate heart rate and SpO2 after first 500 samples (first 5 seconds of samples)
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 
	
	while(1) {
		// delay_ms(1000);
		i = 0;
        un_min = 0x3FFFF;
        un_max = 0;
		
		// dumping the first 100 sets of samples in the memory and shift the last 400 sets of samples to the top
        for(i = 100; i < 500; i++) {
            aun_red_buffer[i-100]=aun_red_buffer[i];
            aun_ir_buffer[i-100]=aun_ir_buffer[i];
            
            // update the signal min and max
            if(un_min>aun_red_buffer[i])
            un_min=aun_red_buffer[i];
            if(un_max<aun_red_buffer[i])
            un_max=aun_red_buffer[i];
        }
		// take 100 sets of samples before calculating the heart rate.
        for(i = 400; i < 500; i++) {
			// delay_ms(10);
            un_prev_data = aun_red_buffer[i-1];
            while(MAX30102_INT == 1);
            max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
			aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    // Combine values to get the actual number
			aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) | (long)temp[4]<<8 | (long)temp[5];   // Combine values to get the actual number
        
            if(aun_red_buffer[i] > un_prev_data) {
                f_temp=aun_red_buffer[i]-un_prev_data;
                f_temp/=(un_max-un_min);
                f_temp*=MAX_BRIGHTNESS;
                n_brightness-=(int)f_temp;
                if(n_brightness<0)
                    n_brightness=0;
            } else {
                f_temp=un_prev_data-aun_red_buffer[i];
                f_temp/=(un_max-un_min);
                f_temp*=MAX_BRIGHTNESS;
                n_brightness+=(int)f_temp;
                if(n_brightness>MAX_BRIGHTNESS)
                    n_brightness=MAX_BRIGHTNESS;
            }
			// send samples and calculation result to terminal program through UART
			//**/ ch_hr_valid == 1 && ch_spo2_valid ==1 && n_heart_rate<120 && n_sp02<101
			if(ch_hr_valid == 1 && n_heart_rate<120) {
				dis_hr = n_heart_rate;
				dis_spo2 = n_sp02;
			} else {
				dis_hr = 0;
				dis_spo2 = 0;
			}
		}
		u2_printf("@%i!%i!%i!%i", n_heart_rate, ch_hr_valid, n_sp02, ch_spo2_valid);
		printf("HR=%i, ", n_heart_rate); 
		printf("HRvalid=%i, ", ch_hr_valid);
		printf("SpO2=%i, ", n_sp02);
		printf("SPO2Valid=%i\r\n", ch_spo2_valid);
        maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
	}
}










