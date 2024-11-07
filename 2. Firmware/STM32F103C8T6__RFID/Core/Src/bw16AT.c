/*
 * bw16AT.c
 *
 *  Created on: Oct 5, 2023
 *      Author: Minh Tuan
 */


#include<bw16AT.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



BW16_StatusTypeDef Bw16_Response(uint8_t *recv_buf, uint8_t *content, uint16_t timeout)
{
	while(1)
	{
		uint8_t *_pStr = (uint8_t*)strstr((char*)recv_buf, (char*)content);
		if(_pStr != 0) return BW16_OK;

		HAL_Delay(1);
		timeout--;
		if(timeout == 0) return BW16_ERROR;
	}
}
void Bw16_Echo(UART_HandleTypeDef huart, FunctionalState ctrl)
{
	//ATE1 là bật chế độ echo. echo là chế độ phản hồi lại dữ liệu hoặc lệnh cho thiết bị gửi
	//ATE0 là tắt chế độ
	switch(ctrl)
	{
		case ENABLE:
			HAL_UART_Transmit(&huart, (uint8_t*)"ATE1\r\n", strlen("ATE1\r\n"), 100);
			HAL_Delay(100);
			break;

		case DISABLE:
			HAL_UART_Transmit(&huart, (uint8_t*)"ATE0\r\n", strlen("ATE0\r\n"), 100);
			HAL_Delay(100);
			break;
	}
}

void Bw16_Configure_Mode(UART_HandleTypeDef huart, BW16_Mode_HandleTypeDef mode)
{
	/*
	AT+WMODE=<mode>,<flash>
	<mode> là giá trị số nguyên chỉ định chế độ kết nối Wi-Fi. Có ba giá trị chế độ thông thường được sử dụng:
	0: Vô hiệu hóa Wifi
	1 (Station mode): Chế độ kết nối Wi-Fi dạng Client, module sẽ kết nối vào một Access Point (AP) để truy cập vào mạng.
	2 (SoftAP mode): Chế độ kết nối Wi-Fi dạng Soft Access Point (AP), module sẽ tạo một Access Point (AP) và cho phép các thiết bị khác kết nối vào nó.
	3 (Station + SoftAP mode): Chế độ kết nối Wi-Fi dạng Client và Soft Access Point (AP) kết hợp, module có thể kết nối vào một AP và cũng tạo ra một AP.
	<flash> giá trị số nguyên để lưu bộ nhớ hay không.
	0 Không lưu
	1 Lưu
	*/
	switch(mode)
	{
		case No_Flash_NULL_Mode:
			HAL_UART_Transmit(&huart, (uint8_t*)"AT+WMODE=0,0\r\n", strlen("AT+WMODE=0,0\r\n"), 100);
			HAL_Delay(100);
			break;

		case No_Flash_STA_Mode:
			HAL_UART_Transmit(&huart, (uint8_t*)"AT+WMODE=1,0\r\n", strlen("AT+WMODE=1,0\r\n"), 100);
			HAL_Delay(100);
			break;

		case No_Flash_AP_Mode:
			HAL_UART_Transmit(&huart, (uint8_t*)"AT+WMODE=2,0\r\n", strlen("AT+WMODE=2,0\r\n"), 100);
			HAL_Delay(100);
			break;

		case No_Flash_AP_and_STA_Mode:
			HAL_UART_Transmit(&huart, (uint8_t*)"AT+WMODE=3,0\r\n", strlen("AT+WMODE=3,0\r\n"), 100);
			HAL_Delay(100);
			break;

		case Flash_NULL_Mode:
			HAL_UART_Transmit(&huart, (uint8_t*)"AT+WMODE=0,1\r\n", strlen("AT+WMODE=0,1\r\n"), 100);
			HAL_Delay(100);
			break;

		case Flash_STA_Mode:
			HAL_UART_Transmit(&huart, (uint8_t*)"AT+WMODE=1,1\r\n", strlen("AT+WMODE=1,1\r\n"), 100);
			HAL_Delay(100);
			break;

		case Flash_AP_Mode:
			HAL_UART_Transmit(&huart, (uint8_t*)"AT+WMODE=2,1\r\n", strlen("AT+WMODE=2,1\r\n"), 100);
			HAL_Delay(100);
			break;

		case FLASH_AP_and_STA_Mode:
			HAL_UART_Transmit(&huart, (uint8_t*)"AT+WMODE=3,1\r\n", strlen("AT+WMODE=3,1\r\n"), 100);
			HAL_Delay(100);
			break;
	}
}

void Bw16_Check_Info_WiFi(UART_HandleTypeDef huart)
{
	/*
		AT+WJAP? dùng để kiểm tra thông tin wifi đã được kết nối
	*/
	HAL_UART_Transmit(&huart, (uint8_t*)"AT+WJAP?\r\n", strlen("AT+WJAP?\r\n"), 100);
	HAL_Delay(100);
}
void Bw16_Reset(UART_HandleTypeDef huart)
{
	/*
		AT+RST reset chip
	*/
	HAL_UART_Transmit(&huart, (uint8_t*)"AT+RST\r\n", strlen("AT+RST\r\n"), 100);
	HAL_Delay(100);
}
void Bw16_AT(UART_HandleTypeDef huart)
{
	HAL_UART_Transmit(&huart, (uint8_t*)"AT\r\n", strlen("AT\r\n"), 100);
	HAL_Delay(100);
}



void Bw16_Ping_MQTT(UART_HandleTypeDef huart,uint8_t *ip)
{
	uint8_t messages[200];
	sprintf((char*)messages, "AT+PING=%s\r\n",  ip);
	HAL_UART_Transmit(&huart, messages,strlen((char*)messages), 100);
	HAL_Delay(100);
}


void Bw16_Set_DHCP(UART_HandleTypeDef huart,char *mode,char *ip,char *mark,char *gateway)
{
	/*
	    AT+WSDHCP=0,192.168.41.201,255.255.255.0,192.168.41.254
		AT+WSHCP=<mode>,<ip>,<mark>,<gateway>
		<mode> có 2 chế độ 0,1.
			- 0: cài ip tĩnh
			- 1: cài ip động
		<ip> địa chỉ ip tĩnh để đặt cho bw16
		<mark> địa chỉ subnetmark
		<gateway> địa chỉ gateway
	*/
	char messages[200];
	sprintf(messages, "AT+WSDHCP=%s,%s,%s,%s\r\n", mode, ip, mark, gateway);
	HAL_UART_Transmit(&huart, messages,strlen((char*)messages), 100);
	HAL_Delay(100);
}

void Bw16_Check_Info_DHCP(UART_HandleTypeDef huart)
{
	/*
		AT+WSDHCP?
		Dùng để kiểm tra các thông số ip,mark,gateway đã được set
	*/
	HAL_UART_Transmit(&huart, (uint8_t*)"AT+WSDHCP?\r\n", strlen("AT+WSDHCP?\r\n"), 100);
	HAL_Delay(100);
}

void Bw16_Auto_Connect_Wifi(UART_HandleTypeDef huart,FunctionalState ctrl)
{
	/*
		AT+AUTOCONN = 0 : tắt chế độ tự động kết nối wifi được lưu trong flash
		AT+AUTOCONN = 1 : bật chế độ tự động kết nối wifi được lưu trong flash
	*/
	switch (ctrl)
	{
	case ENABLE:
		HAL_UART_Transmit(&huart, (uint8_t*)"AT+WAUTOCONN=1\r\n", strlen("AT+WAUTOCONN=1\r\n"), 100);
		HAL_Delay(100);
		break;
	case DISABLE:
		HAL_UART_Transmit(&huart, (uint8_t*)"AT+WAUTOCONN=0\r\n", strlen("AT+WAUTOCONN=0\r\n"), 100);
		HAL_Delay(100);
		break;
	}
}


void BW16_Start_SmartConfig(UART_HandleTypeDef huart, BW16_App_HandleTypeDef app)
{
	/*
		AT+WCONFIG = 0 : tắt các chế độ config
		AT+WCONFIG = 1 : dùng app simple config để config wifi
		AT+WCONFIG = 2 : dùng app wifi config để config wifi
	*/
	switch (app)
	{
	case OFF:
		HAL_UART_Transmit(&huart,(uint8_t*)"AT+WCONFIG=0\r\n",strlen("AT+WCONFIG=0\r\n"),100);
		HAL_Delay(100);
		break;
	case Simple_Config:
		HAL_UART_Transmit(&huart,(uint8_t*)"AT+WCONFIG=1\r\n",strlen("AT+WCONFIG=1\r\n"),100);
		HAL_Delay(100);
		break;
	case Wifi_Config:
		HAL_UART_Transmit(&huart,(uint8_t*)"AT+WCONFIG=2\r\n",strlen("AT+WCONFIG=2\r\n"),100);
		HAL_Delay(100);
		break;
	}
}

void Bw16_Setup_Default(UART_HandleTypeDef huart)
{
	/*
		AT+RESTORE 
		dùng để xóa toàn bộ dữ liệu trong bộ nhớ
	*/
	HAL_UART_Transmit(&huart, (uint8_t*)"AT+RESTORE\r\n", strlen("AT+RESTORE\r\n"),100);
	HAL_Delay(100);
}

void Bw16_RSSI(UART_HandleTypeDef huart)
{
	/*
		AT+WRSSI 
		dùng để kiểm tra tốc độ wifi
	*/
	HAL_UART_Transmit(&huart, (uint8_t*)"AT+WRSSI\r\n", strlen("AT+WRSSI\r\n"),100);

	HAL_Delay(100);
}

void Bw16_Connect_Wifi_AP(UART_HandleTypeDef huart,uint8_t *ptrSSID, uint8_t *ptrPWD)
{
	/*
		AT+WJAP=BA,Aa@123456
		Dùng để kết nối wifi với tên BA và password Aa@123456
	*/
	uint8_t info[100];
	sprintf((char*)info, "AT+WJAP=%s,%s\r\n",ptrSSID,ptrPWD);
	HAL_UART_Transmit(&huart,info,strlen((char*)info),100);
	HAL_Delay(100);
}

//...... MQTT BW16

void Bw16_Set_Info_MQTT(UART_HandleTypeDef huart,uint8_t *key, uint8_t *data)
{
	/*
		AT+MQTT=1,iot-vtc.nichietsuvn.com    tên miền kết nối hoặc IP
		AT+MQTT=2,1887                       1887 - port
		AT+MQTT=3,1                          1 - kết nối tcp
		AT+MQTT=4,00034                      00034 - id
		AT+MQTT=5,guest                      guest - user
		AT+MQTT=6,123456a@                   123456a@ - password
		AT+MQTT=7,"LWTTOPIC",0,1,"123456"    <LWT_topic>,<LWT_qos>,<LWT_Retained>,<LWTpayload>
											 LWTTOPIC: Chủ đề của thông điệp LWT (Last Will and Testament) 
											 - một thông điệp được gửi đến các khách hàng khi một kết nối MQTT bị mất.
											 LWT_qos: Cấp độ QoS (Quality of Service) của thông điệp LWT.
											 LWT_Retained: Trạng thái retained của thông điệp LWT.
											 LWTpayload: Nội dung của thông điệp LWT.

	*/
	uint8_t info[100];
	sprintf((char*)info, "AT+MQTT=%s,%s\r\n",key,data);
	HAL_Delay(100);
	HAL_UART_Transmit(&huart,info,strlen((char*)info),100);
	HAL_Delay(100);
}

void Bw16_Connect_MQTT(UART_HandleTypeDef huart)
{
	/*
		AT+MQTT dùng để connect mqtt
	*/
	HAL_UART_Transmit(&huart,(uint8_t*)"AT+MQTT\r\n",strlen("AT+MQTT\r\n"),100);
	HAL_Delay(100);
}

void Bw16_Check_Info_MQTT(UART_HandleTypeDef huart)
{
	/*
		Kiểm tra thông tin của MQTT 
		MQTT:<MQTT_status>,<Host_name>,<Port>,<scheme>,<client_id>,<username>,<password>,<LWT_topic>,<LWT_qos>,<LWT_Retained>,<LWTpayload>
		<MQTT_status>: Trạng thái kết nối MQTT.
			0: Trạng thái ban đầu
			1: Kết nối
			2: Đăng ký tin nhắn
			3: Kết nối thành công
	*/
	HAL_UART_Transmit(&huart,(uint8_t*)"AT+MQTT?\r\n",strlen("AT+MQTT?\r\n"),100);
	HAL_Delay(100);
}

void Bw16_MQTT_Publish(UART_HandleTypeDef huart, uint8_t *topic,uint8_t *qos, uint8_t *retained, uint8_t *payload)
{
	/*
		<topic>: Chủ đề cần xuất bản.
		<qos>: Mức QoS (0,1,2).
		<Retained>:
			0: Tin nhắn bình thường
			1: Tin nhắn được giữ lại

		<payload>: Nội dung dữ liệu (độ dài tối đa 1024 byte)
	*/
	uint8_t package[200];
	sprintf((char*)package, "AT+MQTTPUB=%s,%s,%s,%s\r\n",topic,qos,retained,payload);
	HAL_UART_Transmit(&huart,package,strlen((char*)package),100);
}

void Bw16_MQTT_PublishRaw(UART_HandleTypeDef huart, uint8_t *topic,uint8_t *qos, uint8_t *retained, uint8_t *length)
{
	/*
	<topic>: Chủ đề cần xuất bản.
	<qos>: Mức QoS (0,1,2).
	<Retained>:
		0: Tin nhắn bình thường
		1: Tin nhắn được giữ lại
	<length>: Độ dài dữ liệu
	*/
	uint8_t messages[200];
	sprintf((char*)messages, "AT+MQTTPUBRAW=%s,%s,%s,%s\r\n",topic,qos,retained,length);
	HAL_UART_Transmit(&huart,messages,strlen((char*)messages),100);
}

void Bw16_MQTT_Subscribe(UART_HandleTypeDef huart, uint8_t *topic,uint8_t *qos)
{
	uint8_t package[200];
	sprintf((char*)package, "AT+MQTTSUB=%s,%s\r\n",topic,qos);
	HAL_UART_Transmit(&huart,package,strlen((char*)package),100);
}

void Bw16_MQTT_UnSubscribe(UART_HandleTypeDef huart, uint8_t *topic)
{
	uint8_t package[100];
	sprintf((char*)package, "AT+MQTTSUB=%s\r\n",topic);
	HAL_UART_Transmit(&huart,package,strlen((char*)package),100);
}
