/*
 ============================================================================
 Name        : pipe.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include "linuxType.h"

#include "controlPro.h"
#include "communicationPro.h"

int main(void)
{
	fid fdCtrlToCom[2];
	fid fdComToCtrl[2];
	pid_t pid;

	if ((pipe(fdCtrlToCom) < 0) || (pipe(fdComToCtrl) < 0))
			DEBUG_PRINT("pipe error\n");
	if ((pid = fork()) < 0)
	{
		DEBUG_PRINT("fork error\n");
	}
	else if (pid > 0)
	{
		if(runCtrlPro(fdCtrlToCom,fdComToCtrl))
			DEBUG_PRINT("control process error!");
	}
	else
	{
		if (runCommPro(fdComToCtrl,fdCtrlToCom))
			DEBUG_PRINT("communication process error!");
	}

	return (0);
}

