/*
 * modbusTcpSrv.h
 *
 *  Created on: 2018-7-22
 *      Author: Administrator
 */

#ifndef MODBUSTCPSRV_H_
#define MODBUSTCPSRV_H_
#include "type.h"
#include "port.h"
#include "para.h"

#define MB_TCP_BUF_SIZE  2048
typedef struct _TProtocolBuf
{
	uint8_t ucTCPRequestFrame[MB_TCP_BUF_SIZE];   //mbЭ����ռĴ���
	uint16_t ucTCPRequestLen;  //mb���ռĴ���Э�鳤��
	uint8_t ucTCPResponseFrame[MB_TCP_BUF_SIZE];   //mbЭ�鷢�ͼĴ���
	uint16_t ucTCPResponseLen;   //mb����Э�鳤��
	uint8_t bFrameSent;   //�Ƿ���з�����Ӧ�ж�
}TProtocolBuf;

TProtocolBuf* getProtocolBuf();
uint16_t* getRegInput();
uint16_t* getRegHolding();
void* modbusServer(void* arg);
TPowerSetPara* getSetParaBak();
#endif /* MODBUSTCPSRV_H_ */
