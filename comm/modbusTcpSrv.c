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


#include "mb.h"
#include "mbutils.h"
#include "type.h"
#include "modbusTcpSrv.h"
#include "para.h"
#include "queue.h"

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

/* socket
 * bind
 * listen
 * accept
 * send/recv
 */
#define SERVER_PORT 502

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
		printf("MBTCP initialization error code is %d.\r\n", eStatus);
		return -1;
	}
	eStatus = eMBEnable();
	if (MB_ENOERR != eStatus)
	{
		printf("MBEnbale error code is %d.\r\n", eStatus);
		return -1;
	}
}

static void handleMBMsg(int* pfd)
{
	/* 接收客户端发来的数据并显示出来 */
	getProtocolBuf()->ucTCPRequestLen = recv(*pfd, getProtocolBuf()->ucTCPRequestFrame, MB_TCP_BUF_SIZE, 0);
	if (getProtocolBuf()->ucTCPRequestLen <= 0)
	{
		close(*pfd);//网络断开怎么处理 to do
		printf("client %d disconnect.\n",*pfd);
		*pfd = 0;
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
			iSendLen = send(*pfd, getProtocolBuf()->ucTCPResponseFrame, getProtocolBuf()->ucTCPResponseLen, 0);
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



extern sem_t semtSetCmd;
extern pthread_mutex_t mutexSetPara;
void* modbusServer(void* arg)
{
    int nSocketFd = 0;
    socklen_t socketAddrLen;
    struct timeval tv;
    int maxfd = 0;
    int retval = 0;
    fd_set readfds;
    int selectFd[100] = {0};
    int selectCount = 0;
    int index = 0;
    char szIP[100][20] = {{0}};

    nSocketFd = tcpSrvInit();
    if(nSocketFd < 0)
    {
    	printf("MB socket initialization error\r\n");
    	//goto out;
    	return (void*)NULL;
    }

    if (MBTCPInit()<0)
    {
    	printf("MBTCP initialization error\r\n");
    	return (void*)NULL;
    }

    FD_ZERO(&readfds);
    FD_SET(nSocketFd,&readfds);
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    maxfd = nSocketFd;

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(nSocketFd,&readfds);//readfds描述符集中第一个是套接字描述符nSocketFd
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        maxfd = nSocketFd;

        /* 把所有的sock描述符都放入到这个描述符集中去 */
        for (index = 0; index < selectCount; index++)
        {
			if (selectFd[index] != 0)
				FD_SET(selectFd[index],&readfds);
            if(selectFd[index] > maxfd)
            {
                maxfd = selectFd[index];
            }
        }

        retval = select(maxfd+1, &readfds, NULL, NULL, &tv);
        if(retval < 0)   //出错
            perror("select");
		else if(retval == 0)// 当没有响应
        {
			printf("timeout\n");
            continue;
        }

        for (index = 0; index < selectCount; index++)//判断是哪个客户端的响应
        {
            if(FD_ISSET(selectFd[index], &readfds))
            {
            	pthread_mutex_lock(&mutexSetPara);
            	memcpy((char*)getSetParaBak(), (char*)(tModbusRegs.usRegHoldingBuf), sizeof(TPowerSetPara));//备份当前保持寄存器
            	handleMBMsg(&selectFd[index]);
            	if (checkParaChange(&tQueue) > 0)
            		sem_post(&semtSetCmd);
            	pthread_mutex_unlock(&mutexSetPara);
            }
        }

		if (FD_ISSET(nSocketFd, &readfds)) //当有新的客户端连接进来
		{
		    struct sockaddr_in stClientAddr;
		    int clifd = 0;
			socketAddrLen = sizeof(struct sockaddr_in); //监听连接，如果有主机要连接过来，则建立套接口连接
			/*
			 * nSocketFd用来监听有没有新的链接，如果有客户建立连接，accept函数将创建一个新套接字来与该客户进行通信，并且返回新套接字的描述符，即clifd
			 * select 监视nSocketFd 和 selectFd[],通过检查nSocketFd，可以检测有没有新的链接，通过检查 selectFd[]，可以知道是否有数据需要读入。
			 */
			clifd = accept(nSocketFd, (struct sockaddr*)&stClientAddr, &socketAddrLen);//nSocketFd用来监听有没有新的链接，如果
			if(-1 == clifd)
			{
				perror("accept error: ");
				return (void*)NULL;
			}
			else
			{
				selectFd[selectCount] = clifd;
				/* 把每一个ip地址存放起来*/
				strncpy(szIP[selectCount],inet_ntoa(stClientAddr.sin_addr),20);
				selectCount++;
				printf("commect %s %d successful\n",inet_ntoa(stClientAddr.sin_addr),ntohs(stClientAddr.sin_port));//ntohs(stClientAddr.sin_port)
			}
		}
    }
    close(nSocketFd);
    return (void*)NULL;
}


