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

//输入寄存器起始地址
#define REG_INPUT_START       0x0001
//输入寄存器数量
#define REG_INPUT_NREGS       70
//保持寄存器起始地址
#define REG_HOLDING_START     0x0001
//保持寄存器数量
#define REG_HOLDING_NREGS     120
//线圈起始地址
#define REG_COILS_START       0x0001
//线圈数量
#define REG_COILS_SIZE        16
//开关寄存器起始地址
#define REG_DISCRETE_START    0x0001
//开关寄存器数量
#define REG_DISCRETE_SIZE     16


#define MB_TCP_BUF_SIZE  2048
typedef struct _TProtocolBuf
{
	uint8_t ucTCPRequestFrame[MB_TCP_BUF_SIZE];   //mb协议接收寄存器
	uint16_t ucTCPRequestLen;  //mb接收寄存器协议长度
	uint8_t ucTCPResponseFrame[MB_TCP_BUF_SIZE];   //mb协议发送寄存器
	uint16_t ucTCPResponseLen;   //mb发送协议长度
	uint8_t bFrameSent;   //是否进行发送响应判断
}TProtocolBuf;

TProtocolBuf* getProtocolBuf();
uint16_t* getRegInput();
uint16_t* getRegHolding();
void* modbusServer(void* arg);
TPowerSetPara* getSetParaBak();
#endif /* MODBUSTCPSRV_H_ */
