/*
 * controlPro.c
 *
 *  Created on: Jul 14, 2018
 *      Author: book
 */

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "linuxType.h"
#include "para.h"
#include "controlPro.h"

//#define SERVER_PORT 4321


static volatile TPowerRunPara tPowerRunPara =
		{
			.tGyRunBasePara  = RUN_BASE_PARA,
			.tGyRunSuperPara = {8,7,6,5},
			.tDyRunPara      = {5,6,7,8}
		};

static volatile TPowerSetPara tPowerSetPara =
		{
			.tGySetPara   = GY_SET_PARA,
			.tDySetPara   = {1},
			.tTimeSetPara = {2},
			.tGySetOper   = {3},
			.tDySetOper   = {4}
		};



int writeRunPara()
{
	int fd = -1;        // fd ����file descriptor���ļ�������
	int ret = -1;
	int cnt = sizeof(TPowerRunPara)/2;
	int offset = 0;
	char writeBuf[sizeof(TPowerRunPara)/2 * 6] = {{'0'}};
	uint16* pRunPara = (uint16*)&tPowerRunPara;
	int i = 0;

	fd = open("/var/www/rundata.txt", O_WRONLY|O_CREAT);
	if (fd < 0)
	{
		DEBUG_PRINT("rundata.txt �ļ��򿪴���\n");
		return -1;
	}

	for (i=0; i<cnt; ++i)
		offset += sprintf((char*)writeBuf+offset,"%d,",pRunPara[i]);
		//sprintf(writeBuf[i], "5d%,", pRunPara[i]);
	(char*)writeBuf[offset-1]='\n';

	ret = write(fd, (char*)writeBuf, sizeof(writeBuf));
	if (ret < 0)
	{
		DEBUG_PRINT("writeʧ��.\n");
		return -1;
	}
	    /*
	        // ���ļ�
	        ret = read(fd, buf, 5);
	        if (ret < 0)
	        {
	            printf("readʧ��\n");
	        }
	        else
	        {
	            printf("ʵ�ʶ�ȡ��%d�ֽ�.\n", ret);
	            printf("�ļ������ǣ�[%s].\n", buf);
	        }
	    */
	        // ���������ر��ļ�
	close(fd);
	return 0;
}



void simRunPara()
{
	uint16 a[] = RUN_BASE_PARA;
	uint16* pRunPara = (uint16*)&tPowerRunPara.tGyRunBasePara;
	int len = sizeof(tPowerRunPara.tGyRunBasePara)/2;
	int i = 0;
	srand((unsigned)time(NULL));
	for( i = 0; i < len;i++ )
	{
		pRunPara[i] = a[i] + rand() % 10;
	}
}



void* sendRunPara(void* fdSend)
{
	while(1)
	{
		simRunPara();
		if (write(*((fid*)fdSend+1), (unsigned char*)&tPowerRunPara, sizeof(tPowerRunPara)) < 0)
			DEBUG_PRINT("write pipe error.\n");
		usleep(5000);

		if (write(*((fid*)fdSend+1), (unsigned char*)&tPowerSetPara, sizeof(tPowerSetPara)) < 0)
			DEBUG_PRINT("write pipe error.\n");
		usleep(5000);
		writeRunPara();
	}

	return NULL;
}


#define BUFF_LEN 1024
void* readSetCmd(void* fdRecv)
{
	while(1)
	{
		int n;
		char para[BUFF_LEN];
		n = read(*(fid*)fdRecv, para, BUFF_LEN);
		DEBUG_PRINT("\n receieve para set change:\n");
		//display received para;
		{
			int i;
			TChangeParaMsg* ptRecv = (TChangeParaMsg*) para;
			uint16* ptSetPara =  (uint16*)&tPowerSetPara;
			for (i=0; i<n/sizeof(TChangeParaMsg); i++)
			{
				DEBUG_PRINT("changePos:%d changeValue:%d .\n", ptRecv[i].usPos, ptRecv[i].u16Value);
				ptSetPara[ptRecv[i].usPos] = ptRecv[i].u16Value;
			}
		}
	}

	return NULL;
}



int runCtrlPro(fid* pfdRecv,fid* pfdSend)
{
	pthread_t tid;

	close(pfdSend[0]);
	close(pfdRecv[1]);

	pthread_create(&tid, NULL, sendRunPara, pfdSend);
	pthread_create(&tid, NULL, readSetCmd, pfdRecv);

	pthread_join(tid, NULL);
	while(1)
	{
		sleep(1);
	}

	return 0;
}
