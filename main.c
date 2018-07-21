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
			printf("pipe error\n");
	if ((pid = fork()) < 0)
	{
		printf("fork error\n");
	}
	else if (pid > 0)
	{
		if(runCtrlPro(fdCtrlToCom,fdComToCtrl))
			printf("control process error!");
	}
	else
	{
		if (runCommPro(fdComToCtrl,fdCtrlToCom))
			printf("communication process error!");
	}

	return (0);
}

