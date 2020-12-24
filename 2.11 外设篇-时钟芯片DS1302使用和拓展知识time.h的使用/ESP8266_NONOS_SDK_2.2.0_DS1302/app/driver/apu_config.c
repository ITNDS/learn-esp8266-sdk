/*
 * apu_config.c
 *
 *  Created on: 2020��4��9��
 *      Author: yang
 */

#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "mem.h"
#include "driver/apu_config.h"
#include "user_json.h"


/*�洢���յ���·�������ƺ�����*/
char RecvSSID[64];
char RecvPWD[64];
char RecvWifiInfoFlag=0;

uint8_t ThisMAC[22];//�洢ģ���MAC��ַ
uint8_t ThisIP[22];//�洢ģ������·�����ֵõ�IP��ַ

uint8_t UDPSendData[100];
uint8_t UDPSendDataLen=0;
uint8_t UDPSendDataCnt=0;//�������ݵĴ���,����3��Ĭ�Ϸ��ͳɹ�
uint8_t UDPSendDataTime=0;

uint8_t TimerOutCnt=0;//��ʱ����
uint8_t TimerOutValue=0;//��ʱʱ��

struct softap_config soft_ap_Config;	//APģʽ����     ����AP�����ṹ��
struct espconn espconn_udp;//UDP�������õĽṹ��



static ETSTimer WiFiLinker;//��ʱ�������·����
static ETSTimer MainTime;//MainTime


apuconfig_callback_t wifiCb = NULL;



static void ICACHE_FLASH_ATTR main_time(void *arg)
{
	if(TimerOutCnt<TimerOutValue){
		TimerOutCnt++;

		if(UDPSendDataCnt>=1){//�Ѿ���APP����������
			UDPSendDataTime++;
		}

		if(UDPSendDataCnt>=3 || UDPSendDataTime>=3){//�������ݵĴ���,����3��Ĭ�Ϸ��ͳɹ�(�����󶨽���),������һ���Ժ�ʱ
			os_timer_disarm(&MainTime);//ֹͣ��ʱ��
			os_timer_disarm(&WiFiLinker);//ֹͣ��ʱ��
			wifi_set_opmode(STATION_MODE);//stationģʽ
			UDPSendDataCnt=0;
			UDPSendDataTime=0;
			TimerOutCnt = 0;
			if(wifiCb){//��������
				wifiCb(APU_STATUS_LINK_OVER,NULL);
			}
		}
	}
	else{//��ʱ
		os_timer_disarm(&MainTime);//ֹͣ��ʱ��
		os_timer_disarm(&WiFiLinker);//ֹͣ��ʱ��
		wifi_set_opmode(STATION_MODE);//stationģʽ
		UDPSendDataCnt=0;
		TimerOutCnt = 0;
		if(wifiCb){//��������
			wifiCb(APU_STATUS_LINK_OVER,NULL);
		}
	}

}


//��ʱ�������·������״̬
static uint8_t wifiStatus = STATION_IDLE;
static void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{
	struct ip_info ipConfig;

	wifi_get_ip_info(STATION_IF, &ipConfig);
	wifiStatus = wifi_station_get_connect_status();
	if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0)//���ӳɹ�
	{
		os_bzero(ThisIP, 22);
//		os_printf("self:"IPSTR"\n",IP2STR(&ipConfig.ip));

		os_sprintf(ThisIP,"%d.%d.%d.%d",IP2STR(&ipConfig.ip));

//		os_printf("\n ThisIP:%s\n",ThisIP);
		os_timer_disarm(&WiFiLinker);//ֹͣ��ѵ

		if(wifiCb){//��������·����
			wifiCb(APU_STATUS_LINKED,ThisIP);
		}
	}
	else
	{
		if(wifiStatus == STATION_WRONG_PASSWORD)
		{
//			os_printf("STATION_WRONG_PASSWORD\r\n");
			RecvWifiInfoFlag=0;
		}
		else if(wifiStatus == STATION_NO_AP_FOUND)
		{
//			os_printf("STATION_NO_AP_FOUND\r\n");
			RecvWifiInfoFlag=0;
		}
		else if(wifiStatus == STATION_CONNECT_FAIL)
		{
//			os_printf("STATION_CONNECT_FAIL\r\n");
			RecvWifiInfoFlag=0;
		}
		else
		{
//			os_printf("STATION_IDLE\r\n");
		}
	}
}


void ConnectWifi(uint8_t* ssid, uint8_t* pass){
	struct station_config stationConf;
	os_memset(&stationConf, 0, sizeof(struct station_config));

	os_sprintf(stationConf.ssid, "%s", ssid);
	os_sprintf(stationConf.password, "%s", pass);

//	os_printf("stationConf.ssid : %s \r\n",stationConf.ssid);
//	os_printf("stationConf.pwd  : %s \r\n",stationConf.password);

	if(wifiCb){
		wifiCb(APU_STATUS_LINK_SSID_PSWD,&stationConf);
	}

	wifi_station_set_config(&stationConf);

	//��ʱ����ѵ���wifi����״̬
	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 200, 1);

	//wifi_station_set_auto_connect(TRUE);
	wifi_station_connect();//����wifi
}


LOCAL int ICACHE_FLASH_ATTR
msg_set(struct jsontree_context *js_ctx,struct jsonparse_state *parser)
{
	int type;
	while( (type = jsonparse_next(parser)) != 0)
	{
		if(jsonparse_strcmp_value(parser,"ssid") == 0)
		{
			os_bzero(RecvSSID, 64);
			jsonparse_next(parser);
			jsonparse_next(parser);
			jsonparse_copy_value(parser, RecvSSID, sizeof(RecvSSID));
//			os_printf("ssid : %s \r\n",RecvSSID);
		}
		else if(jsonparse_strcmp_value(parser,"pwd") == 0){
			os_bzero(RecvPWD, 64);
			jsonparse_next(parser);
			jsonparse_next(parser);
			jsonparse_copy_value(parser, RecvPWD, sizeof(RecvPWD));
//			os_printf("pwd : %s \r\n",RecvPWD);

			if(RecvWifiInfoFlag==0){
				RecvWifiInfoFlag = 1;

				ConnectWifi(RecvSSID,RecvPWD);//����·����
			}
		}
	}
	return 0;
}


struct jsontree_callback msg_callback =
JSONTREE_CALLBACK(NULL,msg_set);
JSONTREE_OBJECT(msg_tree,JSONTREE_PAIR("ssid",&msg_callback),JSONTREE_PAIR("pwd",&msg_callback));



//�ص�����    �ȴ����տͻ��˵���Ϣ. ��Ϣ��ʽ{"ssid":"qqqqq","pwd":"11223344"}
void ICACHE_FLASH_ATTR udpclient_recv(void *arg, char *pdata, unsigned short len)
{
	struct espconn *T_arg=arg;
	remot_info *P_port_info=NULL;

	//���յ�΢���ֻ���������Ϣ���ֻ���¼��ESP8266��AP�ϣ��ֻ���IP��ַ���磺192.168.4.2
//	os_printf("Receive message:%s\n",pdata);							//{"ssid":"qqqqq","pwd":"11223344"}����������·����


	if(wifiCb){
		wifiCb(APU_STATUS_GETING_DATA,pdata);
	}

	//����json����
	struct jsontree_context js;
	jsontree_setup(&js,(struct jsontree_value *)&msg_tree,json_putchar);
	json_parse(&js,pdata);

	if(espconn_get_connection_info(T_arg,&P_port_info,0)==ESPCONN_OK)	//��ȡ�������UDP���ݵ�Զ��������Ϣ
	{
		T_arg->proto.udp->remote_port=P_port_info->remote_port;
		T_arg->proto.udp->remote_ip[0]=P_port_info->remote_ip[0];
		T_arg->proto.udp->remote_ip[1]=P_port_info->remote_ip[1];
		T_arg->proto.udp->remote_ip[2]=P_port_info->remote_ip[2];
		T_arg->proto.udp->remote_ip[3]=P_port_info->remote_ip[3];

		if(ThisIP[0] !=0){
			UDPSendDataLen = os_sprintf(UDPSendData,"{\"mac\":\"%s\",\"ip\":\"%s\"}",ThisMAC,ThisIP);
//			os_printf("\nUDPSendData:%s\n",UDPSendData);
			espconn_send(T_arg,UDPSendData,UDPSendDataLen);//��Է�����Ӧ��
			UDPSendDataCnt++;

			if(wifiCb){
				wifiCb(APU_STATUS_UDPSEND,UDPSendData);
			}
		}
	}
}


void InitAPUConfig(){
	uint8_t mac[6];
	wifi_set_opmode(STATIONAP_MODE);//station+ soft-apģʽ
	soft_ap_Config.ssid_len = strlen(USSID);						//�ȵ����Ƴ��ȣ�����ʵ�ʵ����Ƴ���һ�¾ͺ�
	memcpy(soft_ap_Config.ssid,USSID,soft_ap_Config.ssid_len);		//ʵ���ȵ��������ã����Ը��������Ҫ��
	memcpy(soft_ap_Config.password,UPWD,strlen(UPWD));				//�ȵ���������
	soft_ap_Config.authmode = AUTH_WPA2_PSK;						//����ģʽ
	soft_ap_Config.channel = 1;										//�ŵ�����֧��1~13���ŵ�
	soft_ap_Config.max_connection = 4;								//����������������֧���ĸ���Ĭ���ĸ�
	wifi_softap_set_config_current(&soft_ap_Config);				//���� Wi-Fi SoftAP �ӿ����ã������浽 Flash

	os_bzero(ThisMAC, 22);
	wifi_get_macaddr(STATION_IF, mac);//��ȡ�豸MAC��ַ
	os_sprintf(ThisMAC, MACSTR, MAC2STR(mac));
	os_printf("\n MAC:%s\n",ThisMAC);
}


void InitUDP(){
	espconn_init();//"espconn.h"   195��
	espconn_udp.type = ESPCONN_UDP;     //����
	espconn_udp.state = ESPCONN_NONE;   //һ��ʼ��״̬
	espconn_udp.proto.udp = (esp_udp *)os_malloc(sizeof(esp_udp));
	espconn_udp.proto.udp->local_port = 5556;//�����Ķ˿ں�

    //ע��UDP���ݰ����ջص�
    espconn_regist_recvcb(&espconn_udp, udpclient_recv);   //UDP�������ݻص�
    espconn_create(&espconn_udp);							//����UDP����
}


/**
* @brief   ����apuconfig
* @param   time_out:��ʱʱ�� S   Ĭ��60S
* @param   cb:�����ص�����
* @param   None
* @param   None
* @retval  None
* @warning None
* @example
**/
void apuconfig_start(apuconfig_callback_t cb,uint8 time_out){
	os_bzero(ThisIP, 22);
	TimerOutCnt=0;
	UDPSendDataCnt=0;
	UDPSendDataTime=0;
	//������Ҫ����ı���

	RecvWifiInfoFlag = 0;
	InitAPUConfig();
	InitUDP();

	wifiCb = cb;
	if(time_out==0){
		TimerOutValue=60;
	}else{
		TimerOutValue = time_out;
	}
	os_timer_disarm(&MainTime);
	os_timer_setfn(&MainTime, (os_timer_func_t *)main_time, NULL);
	os_timer_arm(&MainTime, 1000, 1);

}


/**
* @brief   ֹͣapuconfig
* @param   None
* @param   None
* @param   None
* @param   None
* @retval  None
* @warning None
* @example
**/
void  apuconfig_stop(void){
	os_timer_disarm(&MainTime);//ֹͣ��ʱ��
	os_timer_disarm(&WiFiLinker);//ֹͣ��ʱ��
	wifi_set_opmode(STATION_MODE);//stationģʽ
}






