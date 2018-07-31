#include <sys/types.h>          /* See NOTES */
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/time.h>
#include<semaphore.h>

#include <poll.h>
#include <sys/epoll.h>


#include "mb.h"
#include "mbutils.h"
#include "type.h"
#include "modbusTcpSrv.h"
#include "para.h"
#include "queue.h"

#define SERVER_PORT 502

static TProtocolBuf tProtocolBuf = { .bFrameSent = FALSE };
TProtocolBuf* getProtocolBuf() { return &tProtocolBuf; }
static TPowerSetPara tPowerSetPara;
TPowerSetPara* getSetParaBak() { return &tPowerSetPara; }


static volatile struct _TModbusRegs
{
	uint8_t ucRegCoilsBuf[REG_COILS_SIZE/8]; //线圈状态
	uint8_t ucRegDiscreteBuf[REG_DISCRETE_SIZE/8]; //开关输入状态
	uint16_t usRegInputBuf[REG_INPUT_NREGS]; //输入寄存器内容
	uint16_t usRegHoldingBuf[REG_HOLDING_NREGS]; //保持寄存器内容
}tModbusRegs;

uint16_t* getRegInput()
{
	return (uint16_t*)tModbusRegs.usRegInputBuf;
}

uint16_t* getRegHolding()
{
	return (uint16_t*)tModbusRegs.usRegHoldingBuf;
}
/**
  * @brief  输入寄存器处理函数，输入寄存器可读，但不可写。
  * @param  pucRegBuffer  返回数据指针
  *         usAddress     寄存器起始地址
  *         usNRegs       寄存器长度
  * @retval eStatus       寄存器状态
  */
eMBErrorCode 
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
  eMBErrorCode    eStatus = MB_ENOERR;
  int16_t         iRegIndex;
  
  //查询是否在寄存器范围内
  //为了避免警告，修改为有符号整数
  if( ( (int16_t)usAddress >= REG_INPUT_START ) \
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
  {
    //获得操作偏移量，本次操作起始地址-输入寄存器的初始地址
    iRegIndex = ( int16_t )( usAddress - REG_INPUT_START );
    //逐个赋值
    while( usNRegs > 0 )
    {
      //赋值高字节
      *pucRegBuffer++ = ( uint8_t )(tModbusRegs.usRegInputBuf[iRegIndex] >> 8 );
      //赋值低字节
      *pucRegBuffer++ = ( uint8_t )(tModbusRegs.usRegInputBuf[iRegIndex] & 0xFF );
      //偏移量增加
      iRegIndex++;
      //被操作寄存器数量递减
      usNRegs--;
    }
  }
  else
  {
    //返回错误状态，无寄存器  
    eStatus = MB_ENOREG;
  }

  return eStatus;
}

/**
  * @brief  保持寄存器处理函数，保持寄存器可读，可读可写
  * @param  pucRegBuffer  读操作时--返回数据指针，写操作时--输入数据指针
  *         usAddress     寄存器起始地址
  *         usNRegs       寄存器长度
  *         eMode         操作方式，读或者写
  * @retval eStatus       寄存器状态
  */
eMBErrorCode 
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode )
{
  //错误状态
  eMBErrorCode    eStatus = MB_ENOERR;
  //偏移量
  int16_t         iRegIndex;
  
  //判断寄存器是不是在范围内
  if( ( (int16_t)usAddress >= REG_HOLDING_START ) \
     && ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
  {
    //计算偏移量
    iRegIndex = ( int16_t )( usAddress - REG_HOLDING_START );
    
    switch ( eMode )
    {
      //读处理函数  
      case MB_REG_READ:
        while( usNRegs > 0 )
        {
          *pucRegBuffer++ = ( uint8_t )( tModbusRegs.usRegHoldingBuf[iRegIndex] >> 8 );
          *pucRegBuffer++ = ( uint8_t )( tModbusRegs.usRegHoldingBuf[iRegIndex] & 0xFF );
          iRegIndex++;
          usNRegs--;
        }
        break;

      //写处理函数 
      case MB_REG_WRITE:
        while( usNRegs > 0 )
        {
        	tModbusRegs.usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
        	tModbusRegs.usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
			iRegIndex++;
			usNRegs--;
        }
        break;
     }
  }
  else
  {
    //返回错误状态
    eStatus = MB_ENOREG;
  }
  
  return eStatus;
}


/**
  * @brief  线圈寄存器处理函数，线圈寄存器可读，可读可写
  * @param  pucRegBuffer  读操作---返回数据指针，写操作--返回数据指针
  *         usAddress     寄存器起始地址
  *         usNRegs       寄存器长度
  *         eMode         操作方式，读或者写
  * @retval eStatus       寄存器状态
  */
eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
  //错误状态
  eMBErrorCode eStatus = MB_ENOERR;
  //寄存器个数
  int16_t iNCoils = ( int16_t )usNCoils;
  //寄存器偏移量
  int16_t usBitOffset;

  //检查寄存器是否在指定范围内
  if( ( (int16_t)usAddress >= REG_COILS_START ) &&
        ( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE ) )
  {
    //计算寄存器偏移量
    usBitOffset = ( int16_t )( usAddress - REG_COILS_START );
    switch ( eMode )
    {
      //读操作
    case MB_REG_READ:
		while( iNCoils > 0 )
		{
			*pucRegBuffer++ = xMBUtilGetBits((UCHAR*)tModbusRegs.ucRegCoilsBuf, usBitOffset,
										  ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ) );
			iNCoils -= 8;
			usBitOffset += 8;
		}
		break;

      //写操作
    case MB_REG_WRITE:
        while( iNCoils > 0 )
        {
          xMBUtilSetBits((UCHAR*)tModbusRegs.ucRegCoilsBuf, usBitOffset,
                        ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ),
                        *pucRegBuffer++ );
          iNCoils -= 8;
        }
        break;
    }

  }
  else
  {
	  eStatus = MB_ENOREG;
  }
  return eStatus;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
  //错误状态
  eMBErrorCode    eStatus = MB_ENOERR;
  //操作寄存器个数
  int16_t         iNDiscrete = ( int16_t )usNDiscrete;
  //偏移量
  uint16_t        usBitOffset;

  //判断寄存器时候再制定范围内
  if( ( (int16_t)usAddress >= REG_DISCRETE_START ) &&
        ( usAddress + usNDiscrete <= REG_DISCRETE_START + REG_DISCRETE_SIZE ) )
  {
    //获得偏移量
    usBitOffset = ( uint16_t )( usAddress - REG_DISCRETE_START );
    
    while( iNDiscrete > 0 )
    {
      *pucRegBuffer++ = xMBUtilGetBits( (UCHAR*)tModbusRegs.ucRegDiscreteBuf, usBitOffset,
                                      ( uint8_t)( iNDiscrete > 8 ? 8 : iNDiscrete ) );
      iNDiscrete -= 8;
      usBitOffset += 8;
    }

  }
  else
  {
    eStatus = MB_ENOREG;
  }
  return eStatus;
}



static int tcpSrvInit()
{
    int nSocketFd = 0;
    struct sockaddr_in stServAddr;
    int nRet = 0;
    int isReuse = 1;

	/* 产生一个套接口的描数字 */
	nSocketFd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&stServAddr,0,sizeof(struct sockaddr_in));

	stServAddr.sin_family = AF_INET;
	stServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	stServAddr.sin_port = htons(SERVER_PORT);

	setsockopt(nSocketFd,SOL_SOCKET,SO_REUSEADDR,(const char*)&isReuse,sizeof(isReuse));

	/* 把这个套接字描述符和本地地址绑定起来 */
	nRet = bind(nSocketFd,(struct sockaddr*)&stServAddr,sizeof(stServAddr));
	if(-1 == nRet)
	{
		perror("bind failed :");
		close(nSocketFd);
		return -1;
	}

	/* 设置该套接口的监听状态， */
	listen(nSocketFd,1024);

    return nSocketFd;
}



static int MBTCPInit()
{
	eMBErrorCode eStatus = MB_ENOERR;
	eStatus = eMBTCPInit(SERVER_PORT);
	if (MB_ENOERR != eStatus)
	{
		DEBUG_PRINT("MBTCP initialization error code is %d.\r\n", eStatus);
		return -1;
	}
	eStatus = eMBEnable();
	if (MB_ENOERR != eStatus)
	{
		DEBUG_PRINT("MBEnbale error code is %d.\r\n", eStatus);
		return -1;
	}
}

#define MAX_CONNECT_CNT (20)
static int curConnectCnt = 0;
static void handleMbTcpMsg(int socketFd, int epollFd)
{
	getProtocolBuf()->ucTCPRequestLen = recv(socketFd, getProtocolBuf()->ucTCPRequestFrame, MB_TCP_BUF_SIZE, 0);


	if (getProtocolBuf()->ucTCPRequestLen <= 0)
	{
		/*
		 * TCP disconnect
		 * 此处只能检测客户主动disconnect 不能检测出断线等异常断开
		 * 可靠方法是检测若某连接单位时间未收到消息，由服务器主动断开该连接，待完成。
		 */
	    struct epoll_event event;
	    event.data.fd = socketFd;
	    event.events =  EPOLLIN|EPOLLET;
	    epoll_ctl(epollFd, EPOLL_CTL_DEL, socketFd, &event);

		close(socketFd);
		DEBUG_PRINT("client %d disconnect.\n",socketFd);

		if (curConnectCnt > 0)
			--curConnectCnt;
	}
	else
	{
		xMBPortEventPost(EV_FRAME_RECEIVED);  //发送EV_FRAME_RECEIVED事件，以驱动eMBpoll()函数中的状态机
		eMBPoll();   //处理EV_FRAME_RECEIVED事件
		eMBPoll();   //处理EV_EXECUTE事件
		 if(getProtocolBuf()->bFrameSent)
		{
			uint16_t iSendLen = 0;
			getProtocolBuf()->bFrameSent = FALSE;
			iSendLen = send(socketFd, getProtocolBuf()->ucTCPResponseFrame, getProtocolBuf()->ucTCPResponseLen, 0);
		}
	}
}


extern TQueue tQueue;
static int checkParaChange(TQueue* ptQ)
{
	int ret = 0;
	uint16* pBak = (uint16*)getSetParaBak();
	uint16* pHold = getRegHolding();
	int i = 0;
	for (i=0; i<sizeof(TPowerSetPara)/2; i++)
	{
		if ((pBak[i]!=pHold[i]) && !Queue_bIsFull(ptQ))
		{
			Queue_bInsert(ptQ, i);
			ret = 1;
		}
	}

	return ret;
}



void acceptTcpConnect(int srvfd, int epollFd)
{
    struct sockaddr_in sin;
    socklen_t len = sizeof(struct sockaddr_in);
    bzero(&sin, len);

    int confd = accept(srvfd, (struct sockaddr*)&sin, &len);

    if (confd < 0)
    {
       DEBUG_PRINT("bad accept\n");
       return;
    }
    else
    {
		if (curConnectCnt < MAX_CONNECT_CNT)
		{
			++curConnectCnt;
			DEBUG_PRINT("accept connection: %d", confd);
		}
		else
		{
			close(confd);
			DEBUG_PRINT("exceed max connect count to reject connection: %d", confd);
		}
    }
    //add the new connect to epoll fd.
    struct epoll_event event;
    event.data.fd = confd;
    event.events =  EPOLLIN|EPOLLET;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, confd, &event);
}



extern sem_t semtSetCmd;
extern pthread_mutex_t mutexSetPara;
void handleTcpMsg(int socketFd, int epollFd)
{
	pthread_mutex_lock(&mutexSetPara);
	memcpy((char*)getSetParaBak(), (char*)(tModbusRegs.usRegHoldingBuf), sizeof(TPowerSetPara));//备份当前保持寄存器
	handleMbTcpMsg(socketFd, epollFd);
	if (checkParaChange(&tQueue) > 0)
		sem_post(&semtSetCmd);
	pthread_mutex_unlock(&mutexSetPara);
}



#define MAX_EPOLL_EVENTS (500)
void* modbusServer(void* arg)
{
    int i = 0;
    int sockListen;
    int epollFd; //epoll描述符
    struct epoll_event eventList[MAX_EPOLL_EVENTS];//事件数组
    const int TIME_OUT_MS = 3000;
    sockListen = tcpSrvInit();

    if (MBTCPInit()<0)
    {
    	DEBUG_PRINT("MBTCP initialization error\r\n");
    	return (void*)NULL;
    }

    // epoll 初始化
    epollFd = epoll_create(MAX_EPOLL_EVENTS);
    struct epoll_event event;
    event.events = EPOLLIN|EPOLLET;
    event.data.fd = sockListen;

    //add Event
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, sockListen, &event) < 0)
    {
        DEBUG_PRINT("epoll add fail : fd = %d\n", sockListen);
        return -1;
    }

    //epoll
    while(1)
    {
        //epoll_wait
        int ret = epoll_wait(epollFd, eventList, MAX_EPOLL_EVENTS, TIME_OUT_MS);

        if (ret < 0)
        {
            DEBUG_PRINT("epoll error\n");
            break;
        }
        else if (ret == 0)
        {
            DEBUG_PRINT("timeout ...\n");
            continue;
        }

        //直接获取了活动事件数量,给出了活动的流,这里是和poll区别的关键
        for(i=0; i<ret; ++i)
        {
            //错误退出
            if ((eventList[i].events & EPOLLERR) ||
            	(eventList[i].events & EPOLLHUP) ||
                !(eventList[i].events & EPOLLIN))
            {
				DEBUG_PRINT("epoll error\n");
				close (eventList[i].data.fd);
				return -1;
            }

            if (eventList[i].data.fd == sockListen)
            	acceptTcpConnect(sockListen, epollFd);
            else
            	handleTcpMsg(eventList[i].data.fd, epollFd);
        }
    }

    close(epollFd);
    close(sockListen);

    return 0;
}



