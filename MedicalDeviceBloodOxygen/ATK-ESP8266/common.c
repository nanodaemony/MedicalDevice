#include "common.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// 用户配置区
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

// 连接端口号,可自行修改为其他端口.
const u8* portnum="10086";		 

// WIFI STA模式,设置要去连接的路由器无线参数,根据自己的路由器设置.
// 路由器SSID号(SSID就是WIFI名称)
const u8* wifista_ssid="DataCollector";		
// wpa/wpa2 aes加密方式
const u8* wifista_encryption="wpawpa2_aes";	
// 连接密码
const u8* wifista_password="1357924680"; 	  

// WIFI AP模式,模块对外的无线参数,可自行修改.
// 这个WIFI模块产生WIFI的名称
const u8* wifiap_ssid="MedicalData";		
// wpa/wpa2 aes加密方式
const u8* wifiap_encryption="wpawpa2_aes";	
// 连接密码
const u8* wifiap_password="12345678"; 		 
// 网络模式(0,TCP服务器; 0X01,TCP客户端; 0X02,UDP模式)
u8 netpro = 0X01;

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// 4个网络模式
// ATK-ESP8266,3种网络模式,默认为路由器(ROUTER)模式 
const u8 *ATK_ESP8266_CWMODE_TBL[3] = {"STA模式 ","AP模式 ","AP&STA模式 "};	
// 4种工作模式 
// ATK-ESP8266,4种工作模式
const u8 *ATK_ESP8266_WORKMODE_TBL[3] = {"TCP服务器", "TCP客户端", "UDP模式"};	
// 5种加密方式
const u8 *ATK_ESP8266_ECN_TBL[5] = {"OPEN","WEP","WPA_PSK","WPA2_PSK","WPA_WAP2_PSK"};
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

// usmart支持部分
// 将收到的AT指令应答数据返回给电脑串口
// mode: 0: 不清零USART2_RX_STA;
//       1: 清零USART2_RX_STA;
void atk_8266_at_response(u8 mode) {
	// 接收到一次数据了
	if(USART2_RX_STA & 0X8000) { 
		// 添加结束符
		USART2_RX_BUF[USART2_RX_STA & 0X7FFF] = 0;
		// 发送到串口
		printf("%s", USART2_RX_BUF);	
		if(mode) USART2_RX_STA = 0;
	} 
}

// ATK-ESP8266发送命令后,检测接收到的应答
// str:期待的应答结果
// 返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* atk_8266_check_cmd(u8 *str) {
	
	char *strx = 0;
	// 接收到一次数据了
	if(USART2_RX_STA & 0X8000) { 
		// 添加结束符
		USART2_RX_BUF[USART2_RX_STA & 0X7FFF] = 0;
		strx = strstr((const char*)USART2_RX_BUF, (const char*)str);
	} 
	return (u8*)strx;
}


/**
 * 向ESP8266发送控制命令
 * 
 * @param: cmd:发送的命令字符串
 * @param: ack:期待的应答结果,如果为空,则表示不需要等待应答
 * @param: waittime:等待时间(单位:10ms)
 * @return 返回值:0,发送成功(得到了期待的应答结果) 1,发送失败
 */ 
u8 atk_8266_send_cmd(u8 *cmd, u8 *ack, u16 waittime) {
	u8 res = 0; 
	USART2_RX_STA = 0;
	// 发送命令
	u2_printf("%s\r\n", cmd);	
	// 需要等待应答
	if(ack&&waittime) {
		// 等待倒计时
		while(--waittime) {
			delay_ms(10);
			// 接收到期待的应答结果
			if(USART2_RX_STA & 0X8000) {
				if(atk_8266_check_cmd(ack)) {
					// 发送给上位机
					printf("Ack:%s\r\n", (u8*)ack);
					// 得到有效数据 
					break;
				}
				USART2_RX_STA = 0;
			} 
		}
		// 等待时间到了,返回失败
		if(waittime == 0) res = 1; 
	}
	return res;
} 


/**
 * 向ATK-ESP8266发送普通数据
 * 
 * @param: data:发送的数据(不需要添加回车了)
 * @param: ack:期待的应答结果,如果为空,则表示不需要等待应答
 * @param: waittime:等待时间(单位:10ms)
 * @return 返回值:0,发送成功(得到了期待的应答结果) 1,发送失败
 */ 
u8 atk_8266_send_data(u8 *data, u8 *ack, u16 waittime) {
	u8 res = 0;
	USART2_RX_STA = 0;
	// 发送命令
	u2_printf("%s", data);	
	// 需要等待应答
	if(ack && waittime) {
		// 等待倒计时
		while(--waittime) {
			delay_ms(10);
			// 接收到期待的应答结果
			if(USART2_RX_STA & 0X8000) {
				// 检查是否得到有效数据 
				if(atk_8266_check_cmd(ack)) break;
				USART2_RX_STA = 0;
			}
		}
		// 等待时间到了,返回失败
		if(waittime == 0) res = 1; 
	}
	return res;
}

/**
 * ATK-ESP8266退出透传模式(然后进入AT指令模式)
 * @return 返回值:0,退出成功; 1,退出失败
 */ 
u8 atk_8266_quit_trans(void) {
	while((USART2->SR&0X40) == 0);	// 等待发送空
	USART2->DR='+';      
	delay_ms(15);					// 大于串口组帧时间(10ms)
	while((USART2->SR&0X40)==0);	// 等待发送空
	USART2->DR='+';      
	delay_ms(15);					// 大于串口组帧时间(10ms)
	while((USART2->SR&0X40)==0);	// 等待发送空
	USART2->DR='+';      
	delay_ms(500);					// 等待500ms
	return atk_8266_send_cmd("AT", "OK", 20);// 退出透传判断.
}


///**
// * 获取ATK-ESP8266模块的AP+STA连接状态
// * @return 返回值:0，未连接;1,连接成功
// */ 
//u8 atk_8266_apsta_check(void) {
//	if(atk_8266_quit_trans())return 0;			// 退出透传 
//	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	// 发送AT+CIPSTATUS指令,查询连接状态
//	if(atk_8266_check_cmd("+CIPSTATUS:0") 
//		&& atk_8266_check_cmd("+CIPSTATUS:1") 
//		&& atk_8266_check_cmd("+CIPSTATUS:2") 
//		&& atk_8266_check_cmd("+CIPSTATUS:4"))
//		return 0;
//	else return 1;
//}

/**
 * 获取ATK-ESP8266模块的连接状态
 * @return 返回值:0,未连接; 1,连接成功
 */ 
u8 atk_8266_consta_check(void) {
	u8 *p;
	u8 res;
	// 查询状态需要先退出透传 进入到AT指令模式
	if(atk_8266_quit_trans()) return 0;			
	// 发送AT+CIPSTATUS指令,查询连接状态
	atk_8266_send_cmd("AT+CIPSTATUS", ":", 50);
	p = atk_8266_check_cmd("+CIPSTATUS:"); 
	res=*p;		
	return res;
}

/**
 * 获取Client ip地址
 * ipbuf:ip地址输出缓存区
 */ 
void atk_8266_get_wanip(u8* ipbuf) {
	u8 *p, *p1;
	// 获取WAN IP地址失败
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
 * STA模式下的AP的TCP、UDP工作模式配置
 */  
//u8 atk_8266_mode_cofig(u8 netpro) {
//	// u8 netpro;
//	u8 ipbuf[16]; 	// IP缓存
//	u8 *p;
//	u8 key;
//	p=mymalloc(32); // 申请32个字节的内存
//PRESTA:		
//	netpro|=(atk_8266_netpro_sel(50,30,(u8*)ATK_ESP8266_CWMODE_TBL[1]))<<4;	// 网络模式选择
//	if(netpro&0X20) {
//		LCD_Clear(WHITE);
//		if(atk_8266_ip_set("WIFI-AP远端UDP IP设置",(u8*)ATK_ESP8266_WORKMODE_TBL[netpro>>4],(u8*)portnum,ipbuf))goto PRESTA;	//IP输入
//		if(netpro&0X03)sprintf((char*)p,"AT+CIPSTART=1,\"UDP\",\"%s\",%s",ipbuf,(u8*)portnum);    //配置目标UDP服务器,及ID号，STA模式下为0
//		else sprintf((char*)p,"AT+CIPSTART=0,\"UDP\",\"%s\",%s",ipbuf,(u8*)portnum);    //配置目标UDP服务器,及ID号，STA模式下为0
//		delay_ms(200);
//		LCD_Clear(WHITE);
//		atk_8266_send_cmd(p,"OK",200);
//		
//		// AP TCP Client    透传模式测试
//	} else if(netpro&0X10) {
//		LCD_Clear(WHITE);
//		POINT_COLOR=RED;
//		Show_Str_Mid(0,30,"ATK-ESP WIFI-STA 测试",16,240); 
//		Show_Str(30,50,200,16,"正在配置ATK-ESP模块,请稍等...",12,0);
//		if(atk_8266_ip_set("WIFI-AP 远端IP设置",(u8*)ATK_ESP8266_WORKMODE_TBL[netpro>>4],(u8*)portnum,ipbuf))goto PRESTA;	//IP输入
//		if(netpro&0X03)sprintf((char*)p,"AT+CIPSTART=1,\"TCP\",\"%s\",%s",ipbuf,(u8*)portnum);    //配置目标TCP服务器,及ID号，STA模式为client时，为1
//		else sprintf((char*)p,"AT+CIPSTART=0,\"TCP\",\"%s\",%s",ipbuf,(u8*)portnum);    //配置目标TCP服务器,及ID号，STA模式为server时，为0
//		while(atk_8266_send_cmd(p,"OK",200)) {
//			LCD_Clear(WHITE);
//			POINT_COLOR=RED;
//			Show_Str_Mid(0,40,"WK_UP:返回重选",16,240);
//			Show_Str(30,80,200,12,"ATK-ESP 连接TCP SERVER失败",12,0); //连接失败	 
//			key=KEY_Scan(0);
//			if(key==3)goto PRESTA;
//		}				
//	}
//	else;   // 服务器模式不用配置
//	myfree(p);
//	return netpro;
//}


/**
 * 进行IP设置
 * 
 * title:ip设置标题
 * mode:工作模式
 * port:端口号
 * ip:ip缓存区(返回IP给上层函数) 返回值:0,确认连接;1,取消连接.
 */ 
//u8 atk_8266_ip_set(u8* title, u8* mode, u8* port, u8* ip) {
//	u8 res = 0;
// 	u8 key;
//	u8 timex = 0;  
//	u8 iplen = 0;	// IP长度 
//	LCD_Clear(WHITE);  
//	POINT_COLOR = RED; 
//	Show_Str_Mid(0, 30, title, 16, 240);		    // 显示标题	
//	Show_Str(30, 90, 200, 16, "工作模式:", 16, 0);	// 工作模式显示
//	Show_Str(30, 110, 200, 16, "IP地址:", 16, 0);	// IP地址可以键盘设置
//	Show_Str(30, 130, 200, 16, "端口:", 16, 0);	    // 端口号
//	kbd_fn_tbl[0] = "连接";
//	kbd_fn_tbl[1] = "返回"; 
//	// 显示键盘
//	atk_8266_load_keyboard(0, 180);
//	POINT_COLOR = BLUE;
// 	Show_Str(30 + 72, 90, 200, 16, mode, 16, 0);	// 显示工作模式
// 	Show_Str(30 + 40, 130, 200, 16, port, 16, 0);	// 显示端口
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
//				if(key == 13) if (iplen) iplen--;	// 删除  
//				if(key == 14 && iplen) break; 		// 确认连接
//				if(key == 15){res = 1; break;}		// 取消连接
//			}
//			ip[iplen] = 0; 
//			LCD_Fill(30 + 56, 110, 239, 110 + 16, WHITE);
//			Show_Str(30 + 56, 110, 200, 16, ip, 16, 0);			// 显示IP地址 	
//		} 
//		timex++;
//		if(timex == 20) {
//			timex = 0;
//			LED0 =! LED0;
//		}
//		delay_ms(10);
//		// WIFI模块发过来的数据,及时上传给电脑
//		atk_8266_at_response(1);
//	} 
//	return res;
//}

// 测试界面主UI布局
//void atk_8266_mtest_ui(u16 x,u16 y) { 
//	LCD_Clear(WHITE);
//	POINT_COLOR = RED;
//	Show_Str(x, y + 25, 200, 16, "Configuring the ESP8266.", 16, 0);
//	atk_8266_msg_show(x, y + 125, 0);
//}


/**
 * ATK-ESP8266模块测试主函数
 */
//void atk_8266_test(void) {
//    // u16 rlen=0;
//	u8 key;
//	u8 timex;
//	// 颜色为红色
//	POINT_COLOR = RED;
//	Show_Str_Mid(0, 30, "ATK-ESP8266 WIFI模块测试", 16, 240); 
//	// 检查WIFI模块是否在线
//	while(atk_8266_send_cmd("AT", "OK", 20)) {
//		// 退出透传
//		atk_8266_quit_trans();
//		// 关闭透传模式	
//		atk_8266_send_cmd("AT+CIPMODE=0", "OK", 200);  
//		Show_Str(40, 55, 200, 16, "未检测到模块!!!", 16, 0);
//		delay_ms(800);
//		LCD_Fill(40, 55, 200, 55 + 16, WHITE);
//		Show_Str(40, 55, 200, 16, "尝试连接模块...", 16, 0); 
//	} 
//	// 关闭回显
//	while(atk_8266_send_cmd("ATE0", "OK", 20));
//	// 显示测试界面主UI布局
//	atk_8266_mtest_ui(32, 30);

//	// 不断循环
//	while(1) {
//		delay_ms(10); 
//		// 检查ATK-ESP8266模块发送过来的数据,及时上传给电脑
//		atk_8266_at_response(1);
//		// 不断进行按键扫描获取结果
//		key = KEY_Scan(0); 
//		if(key) {
//			LCD_Clear(WHITE);
//			POINT_COLOR = RED;
//			switch(key) {
//				// KEY0
//				case 1:
//					Show_Str_Mid(0, 30, "ATK-ESP WIFI-AP+STA 测试", 16, 240);
//					Show_Str_Mid(0, 50, "正在配置ATK-ESP8266模块，请稍等...", 12, 240);
//					// 串口以太网测试
//					atk_8266_apsta_test();	
//					break;
//				// KEY1(重点)
//				case 2:
//					Show_Str_Mid(0, 30, "ATK-ESP WIFI-STA 测试", 16, 240);
//					Show_Str_Mid(0, 50, "正在配置ATK-ESP8266模块，请稍等...", 12, 240);
//					// WIFI STA测试
//					atk_8266_wifista_test();
//					break;
//				// WK_UP(不用)
//				case 4:
//					// WIFI AP测试
//					atk_8266_wifiap_test();	
//					break;
//			}
//			atk_8266_mtest_ui(32,30);
//			timex=0;
//		} 	 
//		// 200ms闪烁 
//		if((timex%20)==0)LED0=!LED0;
//		timex++;	 
//	} 
//}



/**
 * 等待开始采集的主函数
 */
void startCollection(void) {
    // u16 rlen=0;
	u8 key;
	u8 timex;
	// 颜色为红色
	//POINT_COLOR = RED;
	//Show_Str_Mid(0, 30, "ATK-ESP8266 WIFI模块测试", 16, 240); 
	// 检查WIFI模块是否在线
	while(atk_8266_send_cmd("AT", "OK", 20)) {
		// 退出透传(然后进入AT指令模式)
		atk_8266_quit_trans();
		// 关闭透传模式	
		atk_8266_send_cmd("AT+CIPMODE=0", "OK", 200);  
		//Show_Str(40, 55, 200, 16, "未检测到模块!!!", 16, 0);
		delay_ms(800);
		//LCD_Fill(40, 55, 200, 55 + 16, WHITE);
		//Show_Str(40, 55, 200, 16, "尝试连接模块...", 16, 0); 
	} 
	// 关闭回显
	while(atk_8266_send_cmd("ATE0", "OK", 20));
	// 显示测试界面主UI布局
	// atk_8266_mtest_ui(32, 30);
	delay_ms(1000);
	// WIFI STA测试
	wifista_test();
}



