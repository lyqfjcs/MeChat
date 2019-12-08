#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define BuffNum		1024
char 	g_cMsgbuf[BuffNum] = {0};
int 	g_itime = 0;
char 	g_cbuf[512] = {0};
FILE 	*fp;
int 	g_iCnt = 0;

int iWriteRemindTime()
{
	fseek(fp, 0, SEEK_SET);
	snprintf(g_cbuf, sizeof(g_cbuf), "remined time:%d\n", g_itime);
	fputs(g_cbuf, fp);
	printf("Write TIME: %s\n",g_cbuf);
	fflush(fp);
	
	return 0;
}
int iWriteNtpMsg()
{
	iWriteRemindTime();
	snprintf(g_cMsgbuf+strlen(g_cMsgbuf),sizeof(g_cMsgbuf), "ntp messge time :%d\n", g_itime);
	fputs(g_cMsgbuf, fp);

	fflush(fp);
	if(g_iCnt < 5)
		g_iCnt++;
	
	return 0;
}

int iReadNtpMsg()//char * _Msgbuf)
{
	int i;
	char buf[BuffNum] = {0};
	fseek(fp, 0, SEEK_SET);
	fgets(buf, BuffNum, fp);
	if(g_iCnt >= 5)
	{
		fgets(g_cMsgbuf, BuffNum, fp);
	}
	memset(g_cMsgbuf,0,sizeof(g_cMsgbuf));
	for(i=0; i<5; i++)
	{
		if(fgets(g_cMsgbuf+strlen(g_cMsgbuf), BuffNum, fp) == NULL)
		{
			printf("Read NONE%d:%s\n",i,g_cMsgbuf);
			break;
		}
		else 
			{
			printf("Read%d :%s\n",i,g_cMsgbuf);
			}
	}
	
	return 0;
}

int main()
{
	

	fp = fopen("logMsg","w+");
	if(fp == NULL)
	{	
		printf("file open err\n");
		return -1;
	}
	while (1)
	{
		sleep(5);
		//printf("sizeof g_cBuf：%d", g_cbuf);
		g_itime++;
		iReadNtpMsg();
		iWriteNtpMsg();
	}
	fclose(fp);
}
