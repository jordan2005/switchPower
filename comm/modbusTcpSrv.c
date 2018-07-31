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
	uint8_t ucRegCoilsBuf[REG_COILS_SIZE/8]; //��Ȧ״̬
	uint8_t ucRegDiscreteBuf[REG_DISCRETE_SIZE/8]; //��������״̬
	uint16_t usRegInputBuf[REG_INPUT_NREGS]; //����Ĵ�������
	uint16_t usRegHoldingBuf[REG_HOLDING_NREGS]; //���ּĴ�������
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
  * @brief  ����Ĵ���������������Ĵ����ɶ���������д��
  * @param  pucRegBuffer  ��������ָ��
  *         usAddress     �Ĵ�����ʼ��ַ
  *         usNRegs       �Ĵ�������
  * @retval eStatus       �Ĵ���״̬
  */
eMBErrorCode 
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
  eMBErrorCode    eStatus = MB_ENOERR;
  int16_t         iRegIndex;
  
  //��ѯ�Ƿ��ڼĴ�����Χ��
  //Ϊ�˱��⾯�棬�޸�Ϊ�з�������
  if( ( (int16_t)usAddress >= REG_INPUT_START ) \
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
  {
    //��ò���ƫ���������β�����ʼ��ַ-����Ĵ����ĳ�ʼ��ַ
    iRegIndex = ( int16_t )( usAddress - REG_INPUT_START );
    //�����ֵ
    while( usNRegs > 0 )
    {
      //��ֵ���ֽ�
      *pucRegBuffer++ = ( uint8_t )(tModbusRegs.usRegInputBuf[iRegIndex] >> 8 );
      //��ֵ���ֽ�
      *pucRegBuffer++ = ( uint8_t )(tModbusRegs.usRegInputBuf[iRegIndex] & 0xFF );
      //ƫ��������
      iRegIndex++;
      //�������Ĵ��������ݼ�
      usNRegs--;
    }
  }
  else
  {
    //���ش���״̬���޼Ĵ���  
    eStatus = MB_ENOREG;
  }

  return eStatus;
}

/**
  * @brief  ���ּĴ��������������ּĴ����ɶ����ɶ���д
  * @param  pucRegBuffer  ������ʱ--��������ָ�룬д����ʱ--��������ָ��
  *         usAddress     �Ĵ�����ʼ��ַ
  *         usNRegs       �Ĵ�������
  *         eMode         ������ʽ��������д
  * @retval eStatus       �Ĵ���״̬
  */
eMBErrorCode 
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode )
{
  //����״̬
  eMBErrorCode    eStatus = MB_ENOERR;
  //ƫ����
  int16_t         iRegIndex;
  
  //�жϼĴ����ǲ����ڷ�Χ��
  if( ( (int16_t)usAddress >= REG_HOLDING_START ) \
     && ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
  {
    //����ƫ����
    iRegIndex = ( int16_t )( usAddress - REG_HOLDING_START );
    
    switch ( eMode )
    {
      //��������  
      case MB_REG_READ:
        while( usNRegs > 0 )
        {
          *pucRegBuffer++ = ( uint8_t )( tModbusRegs.usRegHoldingBuf[iRegIndex] >> 8 );
          *pucRegBuffer++ = ( uint8_t )( tModbusRegs.usRegHoldingBuf[iRegIndex] & 0xFF );
          iRegIndex++;
          usNRegs--;
        }
        break;

      //д������ 
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
    //���ش���״̬
    eStatus = MB_ENOREG;
  }
  
  return eStatus;
}


/**
  * @brief  ��Ȧ�Ĵ�������������Ȧ�Ĵ����ɶ����ɶ���д
  * @param  pucRegBuffer  ������---��������ָ�룬д����--��������ָ��
  *         usAddress     �Ĵ�����ʼ��ַ
  *         usNRegs       �Ĵ�������
  *         eMode         ������ʽ��������д
  * @retval eStatus       �Ĵ���״̬
  */
eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
  //����״̬
  eMBErrorCode eStatus = MB_ENOERR;
  //�Ĵ�������
  int16_t iNCoils = ( int16_t )usNCoils;
  //�Ĵ���ƫ����
  int16_t usBitOffset;

  //���Ĵ����Ƿ���ָ����Χ��
  if( ( (int16_t)usAddress >= REG_COILS_START ) &&
        ( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE ) )
  {
    //����Ĵ���ƫ����
    usBitOffset = ( int16_t )( usAddress - REG_COILS_START );
    switch ( eMode )
    {
      //������
    case MB_REG_READ:
		while( iNCoils > 0 )
		{
			*pucRegBuffer++ = xMBUtilGetBits((UCHAR*)tModbusRegs.ucRegCoilsBuf, usBitOffset,
										  ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ) );
			iNCoils -= 8;
			usBitOffset += 8;
		}
		break;

      //д����
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
  //����״̬
  eMBErrorCode    eStatus = MB_ENOERR;
  //�����Ĵ�������
  int16_t         iNDiscrete = ( int16_t )usNDiscrete;
  //ƫ����
  uint16_t        usBitOffset;

  //�жϼĴ���ʱ�����ƶ���Χ��
  if( ( (int16_t)usAddress >= REG_DISCRETE_START ) &&
        ( usAddress + usNDiscrete <= REG_DISCRETE_START + REG_DISCRETE_SIZE ) )
  {
    //���ƫ����
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

	/* ����һ���׽ӿڵ������� */
	nSocketFd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&stServAddr,0,sizeof(struct sockaddr_in));

	stServAddr.sin_family = AF_INET;
	stServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	stServAddr.sin_port = htons(SERVER_PORT);

	setsockopt(nSocketFd,SOL_SOCKET,SO_REUSEADDR,(const char*)&isReuse,sizeof(isReuse));

	/* ������׽����������ͱ��ص�ַ������ */
	nRet = bind(nSocketFd,(struct sockaddr*)&stServAddr,sizeof(stServAddr));
	if(-1 == nRet)
	{
		perror("bind failed :");
		close(nSocketFd);
		return -1;
	}

	/* ���ø��׽ӿڵļ���״̬�� */
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
		 * �˴�ֻ�ܼ��ͻ�����disconnect ���ܼ������ߵ��쳣�Ͽ�
		 * �ɿ������Ǽ����ĳ���ӵ�λʱ��δ�յ���Ϣ���ɷ����������Ͽ������ӣ�����ɡ�
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
		xMBPortEventPost(EV_FRAME_RECEIVED);  //����EV_FRAME_RECEIVED�¼���������eMBpoll()�����е�״̬��
		eMBPoll();   //����EV_FRAME_RECEIVED�¼�
		eMBPoll();   //����EV_EXECUTE�¼�
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
	memcpy((char*)getSetParaBak(), (char*)(tModbusRegs.usRegHoldingBuf), sizeof(TPowerSetPara));//���ݵ�ǰ���ּĴ���
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
    int epollFd; //epoll������
    struct epoll_event eventList[MAX_EPOLL_EVENTS];//�¼�����
    const int TIME_OUT_MS = 3000;
    sockListen = tcpSrvInit();

    if (MBTCPInit()<0)
    {
    	DEBUG_PRINT("MBTCP initialization error\r\n");
    	return (void*)NULL;
    }

    // epoll ��ʼ��
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

        //ֱ�ӻ�ȡ�˻�¼�����,�����˻����,�����Ǻ�poll����Ĺؼ�
        for(i=0; i<ret; ++i)
        {
            //�����˳�
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



