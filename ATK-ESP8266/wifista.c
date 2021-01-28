#include "common.h"
#include "stdlib.h"

// 网络模式
// u8 netpro = 0;	

/**
 * 自定义模式选择方法
 */
u8 wifista_test(void) {
//	// u8 netpro = 0;	// 网络模式
//	u8 key;
//	u8 timex = 0; 
//	// IP地址的数组
//	u8 ipbuf[16];	
//	u8 *p;
//	// 加速第一次获取链接状态
//	u16 t = 999;		
//	u8 res = 0;
//	u16 rlen = 0;
//	// 连接状态
//	u8 constate = 0;	
//	// 申请32字节内存
//	p = mymalloc(32);							
//	// 设置WIFI为STA模式
//	atk_8266_send_cmd("AT+CWMODE=1", "OK", 50);		
//	// DHCP服务器关闭(仅AP模式有效) 
//	atk_8266_send_cmd("AT+RST", "OK", 20);		
//	// 延时3S等待重启成功
//	delay_ms(1000);         
//	delay_ms(1000);
//	delay_ms(1000);
//	// 设置连接到的WIFI网络名称/加密方式/密码,这几个参数由路由器设置进行修改!! 
//	// 设置无线参数:ssid,密码
//	// sprintf((char*)p, "AT+CWJAP=\"%s\",\"%s\"", wifista_ssid, wifista_password);
//	atk_8266_send_cmd("AT+CWJAP=\"DataCollector\",\"1357924680\"", "OK", 100);	
//	// 连接目标路由器,并且获得IP
//	while(atk_8266_send_cmd(p, "WIFI GOT IP", 300));					
//	
//	printf("正在配置ATK-ESP模块,请稍等...");
//	// 必须要时延一下等待缓缓
//	delay_ms(1000);
//	// 0：单连接，1：多连接
//	atk_8266_send_cmd("AT+CIPMUX=0", "OK", 20);   
//	// 配置目标TCP服务器 
//	while(atk_8266_send_cmd("AT+CIPSTART=\"TCP\",\"192.168.8.100\",10086", "OK", 200)) {
//		printf("WK_UP:返回重选");
//		printf("ATK-ESP:连接TCP失败"); 
//	}
//	// 传输模式为：透传
//	atk_8266_send_cmd("AT+CIPMODE=1", "OK", 200);
//	
//	// 等待连接完成...
//	delay_ms(1000);
//	delay_ms(1000);
//	// 服务器模式,获取分配的IP地址并存储到ipbuf中
//	atk_8266_get_wanip(ipbuf);
//	// 显示参数
//	// atk_8266_wificonf_show(30,180,"请设置路由器无线参数为:",(u8*) wifista_ssid,(u8*) wifista_encryption,(u8*) wifista_password);
//	
//	// 清空标志位
//	USART2_RX_STA = 0;
//	// 下面进入到发送数据的逻辑了
//	while(1) {
//		key = KEY_Scan(0);
//		// WK_UP 退出测试		 
//		if(key == WKUP_PRES) { 
//			res = 0;					
//			atk_8266_quit_trans();	// 退出透传
//			// 关闭透传模式(进入到AT指令模式)
//			atk_8266_send_cmd("AT+CIPMODE=0","OK",20);   
//			break;												 
//		}
//		// KEY0 发送数据 
//		else if(key == KEY0_PRES)	{
////			// UDP
////			if((netpro == 3)||(netpro == 2)) {
////				sprintf((char*)p,"ATK-8266%s测试%02d\r\n",ATK_ESP8266_WORKMODE_TBL[netpro],t/10);//测试数据
////				Show_Str(30+54,100,200,12,p,12,0);
////				atk_8266_send_cmd("AT+CIPSEND=25","OK",200);  //发送指定长度的数据
////				delay_ms(200);
////				atk_8266_send_data(p,"OK",100);  //发送指定长度的数据
////				timex=100;
////				
////				// TCP Client
////			} else if((netpro == 1)) {
////				// 退出透传
////				atk_8266_quit_trans();
////				// 开始透传
////				atk_8266_send_cmd("AT+CIPSEND", "OK", 20);                   
////				// 测试数据
////				sprintf((char*)p, "This is from ESP826612980192834378923784927304890234890184901823918290381902801489023849028349089012839012839012");
////				
////				// Show_Str(30 + 54, 100, 200, 12, p, 12, 0);
////				u2_printf("%s", p);
////				timex = 100;
////				
////			} 
//		} else;
//		// 有个时延等待回应????
//		if(timex) timex--;
//		if(timex == 1) {
//			// LCD_Fill(30 + 54, 100, 239, 112, WHITE);
//		}
//		t++;
//		delay_ms(10);
//		// 接收到一次数据
//		if(USART2_RX_STA & 0X8000) { 
//			// 得到本次接收到的数据长度
//			rlen = USART2_RX_STA & 0X7FFF;	
//			// 添加结束符 
//			USART2_RX_BUF[rlen] = 0;		
//			// 发送到串口  
//			printf("%s", USART2_RX_BUF);	 
//			// 屏幕显示接收到的字节数 
//			// sprintf((char*)p, "收到%d字节,内容如下", rlen);
//			
//			// 串口状态置位
//			USART2_RX_STA = 0;
//			// 状态为还未连接,立即更新连接状态
//			if(constate != '+') t = 1000; 
//			else t = 0;   // 状态为已经连接了,10秒后再检查
//		}  
//		// 连续10秒钟没有收到任何数据,检查连接是不是还存在.
//		if(t == 1000) {
//			// 得到连接状态
//			constate = atk_8266_consta_check(); 
//			// 连接状态
//			if(constate == '+') {
//				// Show_Str(30 + 30, 80, 200, 12, "连接成功", 12, 0);  
//			}
//			else {
//				// Show_Str(30 + 30, 80, 200, 12, "连接失败", 12, 0); 
//			}	 
//			t = 0;
//		} 
//		if((t % 200) == 0) LED0 =! LED0;
//		atk_8266_at_response(1);
//	}
//	myfree(p);		// 释放内存 
//	return res;		
	return 0;
} 
























