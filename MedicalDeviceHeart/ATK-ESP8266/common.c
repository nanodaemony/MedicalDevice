#include "common.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// �û�������
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

// ���Ӷ˿ں�,�������޸�Ϊ�����˿�.
const u8* portnum="10086";		 

// WIFI STAģʽ,����Ҫȥ���ӵ�·�������߲���,�����Լ���·��������.
// ·����SSID��(SSID����WIFI����)
const u8* wifista_ssid="DataCollector";		
// wpa/wpa2 aes���ܷ�ʽ
const u8* wifista_encryption="wpawpa2_aes";	
// ��������
const u8* wifista_password="1357924680"; 	  

// WIFI APģʽ,ģ���������߲���,�������޸�.
// ���WIFIģ�����WIFI������
const u8* wifiap_ssid="MedicalData";		
// wpa/wpa2 aes���ܷ�ʽ
const u8* wifiap_encryption="wpawpa2_aes";	
// ��������
const u8* wifiap_password="12345678"; 		 
// ����ģʽ(0,TCP������; 0X01,TCP�ͻ���; 0X02,UDPģʽ)
u8 netpro = 0X01;

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// 4������ģʽ
// ATK-ESP8266,3������ģʽ,Ĭ��Ϊ·����(ROUTER)ģʽ 
const u8 *ATK_ESP8266_CWMODE_TBL[3] = {"STAģʽ ","APģʽ ","AP&STAģʽ "};	
// 4�ֹ���ģʽ 
// ATK-ESP8266,4�ֹ���ģʽ
const u8 *ATK_ESP8266_WORKMODE_TBL[3] = {"TCP������", "TCP�ͻ���", "UDPģʽ"};	
// 5�ּ��ܷ�ʽ
const u8 *ATK_ESP8266_ECN_TBL[5] = {"OPEN","WEP","WPA_PSK","WPA2_PSK","WPA_WAP2_PSK"};
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

// usmart֧�ֲ���
// ���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
// mode: 0: ������USART2_RX_STA;
//       1: ����USART2_RX_STA;
void atk_8266_at_response(u8 mode) {
	// ���յ�һ��������
	if(USART2_RX_STA & 0X8000) { 
		// ��ӽ�����
		USART2_RX_BUF[USART2_RX_STA & 0X7FFF] = 0;
		// ���͵�����
		printf("%s", USART2_RX_BUF);	
		if(mode) USART2_RX_STA = 0;
	} 
}

// ATK-ESP8266���������,�����յ���Ӧ��
// str:�ڴ���Ӧ����
// ����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* atk_8266_check_cmd(u8 *str) {
	
	char *strx = 0;
	// ���յ�һ��������
	if(USART2_RX_STA & 0X8000) { 
		// ��ӽ�����
		USART2_RX_BUF[USART2_RX_STA & 0X7FFF] = 0;
		strx = strstr((const char*)USART2_RX_BUF, (const char*)str);
	} 
	return (u8*)strx;
}


/**
 * ��ESP8266���Ϳ�������
 * 
 * @param: cmd:���͵������ַ���
 * @param: ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
 * @param: waittime:�ȴ�ʱ��(��λ:10ms)
 * @return ����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����) 1,����ʧ��
 */ 
u8 atk_8266_send_cmd(u8 *cmd, u8 *ack, u16 waittime) {
	u8 res = 0; 
	USART2_RX_STA = 0;
	// ��������
	u2_printf("%s\r\n", cmd);	
	// ��Ҫ�ȴ�Ӧ��
	if(ack&&waittime) {
		// �ȴ�����ʱ
		while(--waittime) {
			delay_ms(10);
			// ���յ��ڴ���Ӧ����
			if(USART2_RX_STA & 0X8000) {
				if(atk_8266_check_cmd(ack)) {
					// ���͸���λ��
					printf("Ack:%s\r\n", (u8*)ack);
					// �õ���Ч���� 
					break;
				}
				USART2_RX_STA = 0;
			} 
		}
		// �ȴ�ʱ�䵽��,����ʧ��
		if(waittime == 0) res = 1; 
	}
	return res;
} 


/**
 * ��ATK-ESP8266������ͨ����
 * 
 * @param: data:���͵�����(����Ҫ��ӻس���)
 * @param: ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
 * @param: waittime:�ȴ�ʱ��(��λ:10ms)
 * @return ����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����) 1,����ʧ��
 */ 
u8 atk_8266_send_data(u8 *data, u8 *ack, u16 waittime) {
	u8 res = 0;
	USART2_RX_STA = 0;
	// ��������
	u2_printf("%s", data);	
	// ��Ҫ�ȴ�Ӧ��
	if(ack && waittime) {
		// �ȴ�����ʱ
		while(--waittime) {
			delay_ms(10);
			// ���յ��ڴ���Ӧ����
			if(USART2_RX_STA & 0X8000) {
				// ����Ƿ�õ���Ч���� 
				if(atk_8266_check_cmd(ack)) break;
				USART2_RX_STA = 0;
			}
		}
		// �ȴ�ʱ�䵽��,����ʧ��
		if(waittime == 0) res = 1; 
	}
	return res;
}

/**
 * ATK-ESP8266�˳�͸��ģʽ(Ȼ�����ATָ��ģʽ)
 * @return ����ֵ:0,�˳��ɹ�; 1,�˳�ʧ��
 */ 
u8 atk_8266_quit_trans(void) {
	while((USART2->SR&0X40) == 0);	// �ȴ����Ϳ�
	USART2->DR='+';      
	delay_ms(15);					// ���ڴ�����֡ʱ��(10ms)
	while((USART2->SR&0X40)==0);	// �ȴ����Ϳ�
	USART2->DR='+';      
	delay_ms(15);					// ���ڴ�����֡ʱ��(10ms)
	while((USART2->SR&0X40)==0);	// �ȴ����Ϳ�
	USART2->DR='+';      
	delay_ms(500);					// �ȴ�500ms
	return atk_8266_send_cmd("AT", "OK", 20);// �˳�͸���ж�.
}


///**
// * ��ȡATK-ESP8266ģ���AP+STA����״̬
// * @return ����ֵ:0��δ����;1,���ӳɹ�
// */ 
//u8 atk_8266_apsta_check(void) {
//	if(atk_8266_quit_trans())return 0;			// �˳�͸�� 
//	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	// ����AT+CIPSTATUSָ��,��ѯ����״̬
//	if(atk_8266_check_cmd("+CIPSTATUS:0") 
//		&& atk_8266_check_cmd("+CIPSTATUS:1") 
//		&& atk_8266_check_cmd("+CIPSTATUS:2") 
//		&& atk_8266_check_cmd("+CIPSTATUS:4"))
//		return 0;
//	else return 1;
//}

/**
 * ��ȡATK-ESP8266ģ�������״̬
 * @return ����ֵ:0,δ����; 1,���ӳɹ�
 */ 
u8 atk_8266_consta_check(void) {
	u8 *p;
	u8 res;
	// ��ѯ״̬��Ҫ���˳�͸�� ���뵽ATָ��ģʽ
	if(atk_8266_quit_trans()) return 0;			
	// ����AT+CIPSTATUSָ��,��ѯ����״̬
	atk_8266_send_cmd("AT+CIPSTATUS", ":", 50);
	p = atk_8266_check_cmd("+CIPSTATUS:"); 
	res=*p;		
	return res;
}

/**
 * ��ȡClient ip��ַ
 * ipbuf:ip��ַ���������
 */ 
void atk_8266_get_wanip(u8* ipbuf) {
	u8 *p, *p1;
	// ��ȡWAN IP��ַʧ��
	if(atk_8266_send_cmd("AT+CIFSR", "OK", 50)) {
		ipbuf[0] = 0;
		return;
	}		
	p = atk_8266_check_cmd("\"");
	p1 = (u8*) strstr((const char*)(p + 1), "\"");
	*p1 = 0;
	sprintf((char*)ipbuf, "%s", p + 1);	
}




/**
 * STAģʽ�µ�AP��TCP��UDP����ģʽ����
 */  
//u8 atk_8266_mode_cofig(u8 netpro) {
//	// u8 netpro;
//	u8 ipbuf[16]; 	// IP����
//	u8 *p;
//	u8 key;
//	p=mymalloc(32); // ����32���ֽڵ��ڴ�
//PRESTA:		
//	netpro|=(atk_8266_netpro_sel(50,30,(u8*)ATK_ESP8266_CWMODE_TBL[1]))<<4;	// ����ģʽѡ��
//	if(netpro&0X20) {
//		LCD_Clear(WHITE);
//		if(atk_8266_ip_set("WIFI-APԶ��UDP IP����",(u8*)ATK_ESP8266_WORKMODE_TBL[netpro>>4],(u8*)portnum,ipbuf))goto PRESTA;	//IP����
//		if(netpro&0X03)sprintf((char*)p,"AT+CIPSTART=1,\"UDP\",\"%s\",%s",ipbuf,(u8*)portnum);    //����Ŀ��UDP������,��ID�ţ�STAģʽ��Ϊ0
//		else sprintf((char*)p,"AT+CIPSTART=0,\"UDP\",\"%s\",%s",ipbuf,(u8*)portnum);    //����Ŀ��UDP������,��ID�ţ�STAģʽ��Ϊ0
//		delay_ms(200);
//		LCD_Clear(WHITE);
//		atk_8266_send_cmd(p,"OK",200);
//		
//		// AP TCP Client    ͸��ģʽ����
//	} else if(netpro&0X10) {
//		LCD_Clear(WHITE);
//		POINT_COLOR=RED;
//		Show_Str_Mid(0,30,"ATK-ESP WIFI-STA ����",16,240); 
//		Show_Str(30,50,200,16,"��������ATK-ESPģ��,���Ե�...",12,0);
//		if(atk_8266_ip_set("WIFI-AP Զ��IP����",(u8*)ATK_ESP8266_WORKMODE_TBL[netpro>>4],(u8*)portnum,ipbuf))goto PRESTA;	//IP����
//		if(netpro&0X03)sprintf((char*)p,"AT+CIPSTART=1,\"TCP\",\"%s\",%s",ipbuf,(u8*)portnum);    //����Ŀ��TCP������,��ID�ţ�STAģʽΪclientʱ��Ϊ1
//		else sprintf((char*)p,"AT+CIPSTART=0,\"TCP\",\"%s\",%s",ipbuf,(u8*)portnum);    //����Ŀ��TCP������,��ID�ţ�STAģʽΪserverʱ��Ϊ0
//		while(atk_8266_send_cmd(p,"OK",200)) {
//			LCD_Clear(WHITE);
//			POINT_COLOR=RED;
//			Show_Str_Mid(0,40,"WK_UP:������ѡ",16,240);
//			Show_Str(30,80,200,12,"ATK-ESP ����TCP SERVERʧ��",12,0); //����ʧ��	 
//			key=KEY_Scan(0);
//			if(key==3)goto PRESTA;
//		}				
//	}
//	else;   // ������ģʽ��������
//	myfree(p);
//	return netpro;
//}


/**
 * ����IP����
 * 
 * title:ip���ñ���
 * mode:����ģʽ
 * port:�˿ں�
 * ip:ip������(����IP���ϲ㺯��) ����ֵ:0,ȷ������;1,ȡ������.
 */ 
//u8 atk_8266_ip_set(u8* title, u8* mode, u8* port, u8* ip) {
//	u8 res = 0;
// 	u8 key;
//	u8 timex = 0;  
//	u8 iplen = 0;	// IP���� 
//	LCD_Clear(WHITE);  
//	POINT_COLOR = RED; 
//	Show_Str_Mid(0, 30, title, 16, 240);		    // ��ʾ����	
//	Show_Str(30, 90, 200, 16, "����ģʽ:", 16, 0);	// ����ģʽ��ʾ
//	Show_Str(30, 110, 200, 16, "IP��ַ:", 16, 0);	// IP��ַ���Լ�������
//	Show_Str(30, 130, 200, 16, "�˿�:", 16, 0);	    // �˿ں�
//	kbd_fn_tbl[0] = "����";
//	kbd_fn_tbl[1] = "����"; 
//	// ��ʾ����
//	atk_8266_load_keyboard(0, 180);
//	POINT_COLOR = BLUE;
// 	Show_Str(30 + 72, 90, 200, 16, mode, 16, 0);	// ��ʾ����ģʽ
// 	Show_Str(30 + 40, 130, 200, 16, port, 16, 0);	// ��ʾ�˿�
//	// ipBuf
//	ip[0] = 0;
//	while(1) {
//		key = atk_8266_get_keynum(0, 180);
//		if(key) {
//			if(key < 12) {
//				if(iplen < 15) { 
//					ip[iplen++] = kbd_tbl[key - 1][0];
//				}
//			} else {
//				if(key == 13) if (iplen) iplen--;	// ɾ��  
//				if(key == 14 && iplen) break; 		// ȷ������
//				if(key == 15){res = 1; break;}		// ȡ������
//			}
//			ip[iplen] = 0; 
//			LCD_Fill(30 + 56, 110, 239, 110 + 16, WHITE);
//			Show_Str(30 + 56, 110, 200, 16, ip, 16, 0);			// ��ʾIP��ַ 	
//		} 
//		timex++;
//		if(timex == 20) {
//			timex = 0;
//			LED0 =! LED0;
//		}
//		delay_ms(10);
//		// WIFIģ�鷢����������,��ʱ�ϴ�������
//		atk_8266_at_response(1);
//	} 
//	return res;
//}

// ���Խ�����UI����
//void atk_8266_mtest_ui(u16 x,u16 y) { 
//	LCD_Clear(WHITE);
//	POINT_COLOR = RED;
//	Show_Str(x, y + 25, 200, 16, "Configuring the ESP8266.", 16, 0);
//	atk_8266_msg_show(x, y + 125, 0);
//}


/**
 * ATK-ESP8266ģ�����������
 */
//void atk_8266_test(void) {
//    // u16 rlen=0;
//	u8 key;
//	u8 timex;
//	// ��ɫΪ��ɫ
//	POINT_COLOR = RED;
//	Show_Str_Mid(0, 30, "ATK-ESP8266 WIFIģ�����", 16, 240); 
//	// ���WIFIģ���Ƿ�����
//	while(atk_8266_send_cmd("AT", "OK", 20)) {
//		// �˳�͸��
//		atk_8266_quit_trans();
//		// �ر�͸��ģʽ	
//		atk_8266_send_cmd("AT+CIPMODE=0", "OK", 200);  
//		Show_Str(40, 55, 200, 16, "δ��⵽ģ��!!!", 16, 0);
//		delay_ms(800);
//		LCD_Fill(40, 55, 200, 55 + 16, WHITE);
//		Show_Str(40, 55, 200, 16, "��������ģ��...", 16, 0); 
//	} 
//	// �رջ���
//	while(atk_8266_send_cmd("ATE0", "OK", 20));
//	// ��ʾ���Խ�����UI����
//	atk_8266_mtest_ui(32, 30);

//	// ����ѭ��
//	while(1) {
//		delay_ms(10); 
//		// ���ATK-ESP8266ģ�鷢�͹���������,��ʱ�ϴ�������
//		atk_8266_at_response(1);
//		// ���Ͻ��а���ɨ���ȡ���
//		key = KEY_Scan(0); 
//		if(key) {
//			LCD_Clear(WHITE);
//			POINT_COLOR = RED;
//			switch(key) {
//				// KEY0
//				case 1:
//					Show_Str_Mid(0, 30, "ATK-ESP WIFI-AP+STA ����", 16, 240);
//					Show_Str_Mid(0, 50, "��������ATK-ESP8266ģ�飬���Ե�...", 12, 240);
//					// ������̫������
//					atk_8266_apsta_test();	
//					break;
//				// KEY1(�ص�)
//				case 2:
//					Show_Str_Mid(0, 30, "ATK-ESP WIFI-STA ����", 16, 240);
//					Show_Str_Mid(0, 50, "��������ATK-ESP8266ģ�飬���Ե�...", 12, 240);
//					// WIFI STA����
//					atk_8266_wifista_test();
//					break;
//				// WK_UP(����)
//				case 4:
//					// WIFI AP����
//					atk_8266_wifiap_test();	
//					break;
//			}
//			atk_8266_mtest_ui(32,30);
//			timex=0;
//		} 	 
//		// 200ms��˸ 
//		if((timex%20)==0)LED0=!LED0;
//		timex++;	 
//	} 
//}



/**
 * �ȴ���ʼ�ɼ���������
 */
void startCollection(void) {
    // u16 rlen=0;
	u8 key;
	u8 timex;
	// ��ɫΪ��ɫ
	//POINT_COLOR = RED;
	//Show_Str_Mid(0, 30, "ATK-ESP8266 WIFIģ�����", 16, 240); 
	// ���WIFIģ���Ƿ�����
	while(atk_8266_send_cmd("AT", "OK", 20)) {
		// �˳�͸��(Ȼ�����ATָ��ģʽ)
		atk_8266_quit_trans();
		// �ر�͸��ģʽ	
		atk_8266_send_cmd("AT+CIPMODE=0", "OK", 200);  
		//Show_Str(40, 55, 200, 16, "δ��⵽ģ��!!!", 16, 0);
		delay_ms(800);
		//LCD_Fill(40, 55, 200, 55 + 16, WHITE);
		//Show_Str(40, 55, 200, 16, "��������ģ��...", 16, 0); 
	} 
	// �رջ���
	while(atk_8266_send_cmd("ATE0", "OK", 20));
	// ��ʾ���Խ�����UI����
	// atk_8266_mtest_ui(32, 30);
	delay_ms(1000);
	// WIFI STA����
	wifista_test();
}



