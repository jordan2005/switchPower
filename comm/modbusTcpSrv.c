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
	/* ���տͻ��˷��������ݲ���ʾ���� */
	getProtocolBuf()->ucTCPRequestLen = recv(*pfd, getProtocolBuf()->ucTCPRequestFrame, MB_TCP_BUF_SIZE, 0);
	if (getProtocolBuf()->ucTCPRequestLen <= 0)
	{
		close(*pfd);//����Ͽ���ô���� to do
		printf("client %d disconnect.\n",*pfd);
		*pfd = 0;
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
        FD_SET(nSocketFd,&readfds);//readfds���������е�һ�����׽���������nSocketFd
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        maxfd = nSocketFd;

        /* �����е�sock�����������뵽�������������ȥ */
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
        if(retval < 0)   //����
            perror("select");
		else if(retval == 0)// ��û����Ӧ
        {
			printf("timeout\n");
            continue;
        }

        for (index = 0; index < selectCount; index++)//�ж����ĸ��ͻ��˵���Ӧ
        {
            if(FD_ISSET(selectFd[index], &readfds))
            {
            	pthread_mutex_lock(&mutexSetPara);
            	memcpy((char*)getSetParaBak(), (char*)(tModbusRegs.usRegHoldingBuf), sizeof(TPowerSetPara));//���ݵ�ǰ���ּĴ���
            	handleMBMsg(&selectFd[index]);
            	if (checkParaChange(&tQueue) > 0)
            		sem_post(&semtSetCmd);
            	pthread_mutex_unlock(&mutexSetPara);
            }
        }

		if (FD_ISSET(nSocketFd, &readfds)) //�����µĿͻ������ӽ���
		{
		    struct sockaddr_in stClientAddr;
		    int clifd = 0;
			socketAddrLen = sizeof(struct sockaddr_in); //�������ӣ����������Ҫ���ӹ����������׽ӿ�����
			/*
			 * nSocketFd����������û���µ����ӣ�����пͻ��������ӣ�accept����������һ�����׽�������ÿͻ�����ͨ�ţ����ҷ������׽��ֵ�����������clifd
			 * select ����nSocketFd �� selectFd[],ͨ�����nSocketFd�����Լ����û���µ����ӣ�ͨ����� selectFd[]������֪���Ƿ���������Ҫ���롣
			 */
			clifd = accept(nSocketFd, (struct sockaddr*)&stClientAddr, &socketAddrLen);//nSocketFd����������û���µ����ӣ����
			if(-1 == clifd)
			{
				perror("accept error: ");
				return (void*)NULL;
			}
			else
			{
				selectFd[selectCount] = clifd;
				/* ��ÿһ��ip��ַ�������*/
				strncpy(szIP[selectCount],inet_ntoa(stClientAddr.sin_addr),20);
				selectCount++;
				printf("commect %s %d successful\n",inet_ntoa(stClientAddr.sin_addr),ntohs(stClientAddr.sin_port));//ntohs(stClientAddr.sin_port)
			}
		}
    }
    close(nSocketFd);
    return (void*)NULL;
}


