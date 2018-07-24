
#include "port.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
/*-------------------------socket includes-----------------------------------*/
#include <sys/types.h>          /* See NOTES */
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#include "modbusTcpSrv.h"

#define BACKLOG  (10)

BOOL  xMBTCPPortInit( USHORT usTCPPort )
{
	return TRUE;

}

BOOL  xMBTCPPortGetRequest( UCHAR **ppucMBTCPFrame, USHORT * usTCPLength )
{
    *ppucMBTCPFrame = (uint8_t *) getProtocolBuf()->ucTCPRequestFrame;
    *usTCPLength = getProtocolBuf()->ucTCPRequestLen;
    /* Reset the buffer. */  
    getProtocolBuf()->ucTCPRequestLen = 0;
    return TRUE;  
}


BOOL xMBTCPPortSendResponse( const UCHAR *pucMBTCPFrame, USHORT usTCPLength )
{  
	memcpy(getProtocolBuf()->ucTCPResponseFrame,pucMBTCPFrame , usTCPLength);
	getProtocolBuf()->ucTCPResponseLen = usTCPLength;
	getProtocolBuf()->bFrameSent = TRUE; // 通过W5500发送数据
	return getProtocolBuf()->bFrameSent;
}

void  vMBTCPPortClose( void )
{
};

void vMBTCPPortDisable( void )
{
}
