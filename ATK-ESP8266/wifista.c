#include "common.h"
#include "stdlib.h"

// ����ģʽ
// u8 netpro = 0;	

/**
 * �Զ���ģʽѡ�񷽷�
 */
u8 wifista_test(void) {
//	// u8 netpro = 0;	// ����ģʽ
//	u8 key;
//	u8 timex = 0; 
//	// IP��ַ������
//	u8 ipbuf[16];	
//	u8 *p;
//	// ���ٵ�һ�λ�ȡ����״̬
//	u16 t = 999;		
//	u8 res = 0;
//	u16 rlen = 0;
//	// ����״̬
//	u8 constate = 0;	
//	// ����32�ֽ��ڴ�
//	p = mymalloc(32);							
//	// ����WIFIΪSTAģʽ
//	atk_8266_send_cmd("AT+CWMODE=1", "OK", 50);		
//	// DHCP�������ر�(��APģʽ��Ч) 
//	atk_8266_send_cmd("AT+RST", "OK", 20);		
//	// ��ʱ3S�ȴ������ɹ�
//	delay_ms(1000);         
//	delay_ms(1000);
//	delay_ms(1000);
//	// �������ӵ���WIFI��������/���ܷ�ʽ/����,�⼸��������·�������ý����޸�!! 
//	// �������߲���:ssid,����
//	// sprintf((char*)p, "AT+CWJAP=\"%s\",\"%s\"", wifista_ssid, wifista_password);
//	atk_8266_send_cmd("AT+CWJAP=\"DataCollector\",\"1357924680\"", "OK", 100);	
//	// ����Ŀ��·����,���һ��IP
//	while(atk_8266_send_cmd(p, "WIFI GOT IP", 300));					
//	
//	printf("��������ATK-ESPģ��,���Ե�...");
//	// ����Ҫʱ��һ�µȴ�����
//	delay_ms(1000);
//	// 0�������ӣ�1��������
//	atk_8266_send_cmd("AT+CIPMUX=0", "OK", 20);   
//	// ����Ŀ��TCP������ 
//	while(atk_8266_send_cmd("AT+CIPSTART=\"TCP\",\"192.168.8.100\",10086", "OK", 200)) {
//		printf("WK_UP:������ѡ");
//		printf("ATK-ESP:����TCPʧ��"); 
//	}
//	// ����ģʽΪ��͸��
//	atk_8266_send_cmd("AT+CIPMODE=1", "OK", 200);
//	
//	// �ȴ��������...
//	delay_ms(1000);
//	delay_ms(1000);
//	// ������ģʽ,��ȡ�����IP��ַ���洢��ipbuf��
//	atk_8266_get_wanip(ipbuf);
//	// ��ʾ����
//	// atk_8266_wificonf_show(30,180,"������·�������߲���Ϊ:",(u8*) wifista_ssid,(u8*) wifista_encryption,(u8*) wifista_password);
//	
//	// ��ձ�־λ
//	USART2_RX_STA = 0;
//	// ������뵽�������ݵ��߼���
//	while(1) {
//		key = KEY_Scan(0);
//		// WK_UP �˳�����		 
//		if(key == WKUP_PRES) { 
//			res = 0;					
//			atk_8266_quit_trans();	// �˳�͸��
//			// �ر�͸��ģʽ(���뵽ATָ��ģʽ)
//			atk_8266_send_cmd("AT+CIPMODE=0","OK",20);   
//			break;												 
//		}
//		// KEY0 �������� 
//		else if(key == KEY0_PRES)	{
////			// UDP
////			if((netpro == 3)||(netpro == 2)) {
////				sprintf((char*)p,"ATK-8266%s����%02d\r\n",ATK_ESP8266_WORKMODE_TBL[netpro],t/10);//��������
////				Show_Str(30+54,100,200,12,p,12,0);
////				atk_8266_send_cmd("AT+CIPSEND=25","OK",200);  //����ָ�����ȵ�����
////				delay_ms(200);
////				atk_8266_send_data(p,"OK",100);  //����ָ�����ȵ�����
////				timex=100;
////				
////				// TCP Client
////			} else if((netpro == 1)) {
////				// �˳�͸��
////				atk_8266_quit_trans();
////				// ��ʼ͸��
////				atk_8266_send_cmd("AT+CIPSEND", "OK", 20);                   
////				// ��������
////				sprintf((char*)p, "This is from ESP826612980192834378923784927304890234890184901823918290381902801489023849028349089012839012839012");
////				
////				// Show_Str(30 + 54, 100, 200, 12, p, 12, 0);
////				u2_printf("%s", p);
////				timex = 100;
////				
////			} 
//		} else;
//		// �и�ʱ�ӵȴ���Ӧ????
//		if(timex) timex--;
//		if(timex == 1) {
//			// LCD_Fill(30 + 54, 100, 239, 112, WHITE);
//		}
//		t++;
//		delay_ms(10);
//		// ���յ�һ������
//		if(USART2_RX_STA & 0X8000) { 
//			// �õ����ν��յ������ݳ���
//			rlen = USART2_RX_STA & 0X7FFF;	
//			// ��ӽ����� 
//			USART2_RX_BUF[rlen] = 0;		
//			// ���͵�����  
//			printf("%s", USART2_RX_BUF);	 
//			// ��Ļ��ʾ���յ����ֽ��� 
//			// sprintf((char*)p, "�յ�%d�ֽ�,��������", rlen);
//			
//			// ����״̬��λ
//			USART2_RX_STA = 0;
//			// ״̬Ϊ��δ����,������������״̬
//			if(constate != '+') t = 1000; 
//			else t = 0;   // ״̬Ϊ�Ѿ�������,10����ټ��
//		}  
//		// ����10����û���յ��κ�����,��������ǲ��ǻ�����.
//		if(t == 1000) {
//			// �õ�����״̬
//			constate = atk_8266_consta_check(); 
//			// ����״̬
//			if(constate == '+') {
//				// Show_Str(30 + 30, 80, 200, 12, "���ӳɹ�", 12, 0);  
//			}
//			else {
//				// Show_Str(30 + 30, 80, 200, 12, "����ʧ��", 12, 0); 
//			}	 
//			t = 0;
//		} 
//		if((t % 200) == 0) LED0 =! LED0;
//		atk_8266_at_response(1);
//	}
//	myfree(p);		// �ͷ��ڴ� 
//	return res;		
	return 0;
} 
























