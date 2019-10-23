#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sqlite3.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

typedef struct Data
{
	int work;					//cli工作指令
	int flag;					//回执flag
	char mess[1024];				//消息
	char account[30];			//用户登入账号
	char name[30];				//用户昵称
	char pass[30];				//用户密码
	char online[30];				//在线情况
	char time[30];
	int root;					//root权限？	
	int jinyan;					//禁言flag？
	char toName[30];			
	char fromName[30];
	char fileName[30];
	char question[256];		//密保问题
	char answer[256];			//密保答案
}MsgData;

typedef struct LocalChat
{
	char mess[1024];
	char fromName[30];
	char toName[30];
	char time[30];
	int flag;//判断私聊还是群聊 1私聊，0群聊
}chatFile;

char * getTime();
void secondMenuAndAction();
//void chatAll();



char IP[15];//服务器的IP
short PORT = 7777;//服务器服务端口
int clientSocket;
char myName[30];
char myAccount[30];
int isChatOneOnline;
int rootFlag;//判断管理员flag
int bossFlag;

void* recvThread(void* _clientSocket)			 					//一直在这里接收
{
	clientSocket = (int)_clientSocket;
	MsgData msg;
    while(1)
	{
		if(recv(clientSocket,&msg,sizeof(MsgData),0) == -1)//每次都判断
		{
			printf("服务器断开链接\n");
			exit(-1);
		}
		switch(msg.work)
		{
			case 3:
					printf("（群聊）%s : %s\n",msg.name,msg.mess);
					saveGroupChat(&msg,1);
				break;	
			case 4:
					isChatOneOnline = 1;//没找到私聊的人或已下下线
				break;
			case 5: 
					//if(isChatOneOnline != 1)
					//{
						printf("（私聊）%s偷偷对你说：%s\n",msg.fromName,msg.mess);
						saveGroupChat(&msg,0);
					//}
				break;
			case 6://在线人员
					printf("%s\n",msg.name);
				break;			
			case 7: 
					if(msg.flag == 1)
					{
						rootFlag = 1;
						printf("您是root用户请稍后\n");
					}
					else
					{
						printf("您不是root用户\n正在返回群聊........\n");
					}
				break;
			case 9:
					printf("操作失败（对方可能不存在）请重新选择操作\n");
				break;

			case 12:
					printf("文件发送成功\n正在返回群聊........\n");
				break;
			case 13:
					printf("文件发送失败（此人不群聊中不能接收文件）\n正在返回群聊........\n");
				break;
			case 15:
					saveFile(&msg);
					printf("从%s处接收到一个文件\n",msg.fromName);
				break;
			case -1:
					printf("一些意想不到的错误发生了\n");
				break;
			case 21:
					printf("对方不存在或不在线\n");
				break;

		}
		if(bossFlag == 1)
		{
			break;
		}
	}
}



void saveGroupChat(MsgData *msg,int flag)							//保存聊天记录分俩种，群聊私聊分开
{
	FILE *fp = fopen("localChat.txt","a+");
	chatFile temp;
	if(flag == 1)
	{
		strcpy(temp.toName,"All");
		strcpy(temp.fromName,msg->name);
	}
	else
	{
		strcpy(temp.toName,"you");
		strcpy(temp.fromName,msg->fromName);
	}
	strcpy(temp.mess,msg->mess);
	strcpy(temp.time,getTime());
	fwrite(&temp,sizeof(chatFile),1,fp);//将结构体直接写入文件

	fclose(fp);
}

char *getTime()							//获取时间函数
{
	time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
	return asctime(timeinfo);
}


void saveFile(MsgData *msg)
{
	char buf[1024] = {0};
	sprintf(buf,"%s",msg->fileName);
	int pd = open(buf, O_WRONLY|O_CREAT);		
	if (pd == -1)
    {
        perror ("error");
        return;
    }
	memset(buf,0,sizeof(buf));
    strcpy(buf, msg->mess);
    write(pd,buf,strlen(buf));
	close(pd);	
}

void firstMenu()
{
	printf("\n");
	printf("\t聊天室demo\t\n");
	printf("\t*****************\n");
	printf("\t1.注册\n");
	printf("\t2.登入\n");
	printf("\t3.修改密码\n");
	printf("\t4.修改昵称\n");
	printf("\t5.找回密码\n");
	printf("\t6.退出\n");
	printf("\t群主需抢先注册昵称为root\n");
	printf("\t*****************\n");
}

void registerNewAccount()									//注册
{
	MsgData msg;
	msg.work = 1;
	int accountTemp;	
	char tempName[30];
	char tempAccount[30];
	char tempPassword[30];	
	srand((unsigned) time(NULL));				//时间函数生成随机
	accountTemp = rand()%10000;
	sprintf(msg.account,"%d",accountTemp);				//整形转换字符串的一种方法	
	printf("若不想注册，任一输入下输入out即可返回\n");
	printf("输入你的昵称：");
	gets(tempName);
	if(strcmp(tempName,"out") == 0)
	{
		return ;
	}
	printf("输入你的密码：");
	gets(tempPassword);
	if(strcmp(tempPassword,"out") == 0)
	{
		return ;
	}

	strcpy(msg.name,tempName);
	strcpy(msg.pass,tempPassword);	
	if(strcmp(msg.name,"root") == 0)				//注册群主
	{
		msg.root = 1;
		strcpy(msg.account,"111");
	}
	else
	{
		msg.root = 0;
	}
	send(clientSocket,&msg,sizeof(MsgData),0);		//发送给总控制台;	
	if(recv(clientSocket,&msg,sizeof(MsgData),0) <= 0)//每次都判断
	{
		printf("服务器断开链接\n");
		exit(-1);
	}		
	if(msg.flag == 1)
	{
		printf("注册成功\n");
		printf("\n您的登入账号为：%s 请务必牢记\n",msg.account);
	}
	else if(msg.flag == 0)
	{
		printf("服务器错误注册失败\n");
	}
	else if(msg.flag == 3)
	{
		printf("昵称重复请重新注册\n");
	}
} 

void lookOnlinePeople()							//查看在群聊中的人员
{
	MsgData msg;
	msg.work = 6;
	printf("在线名单如下\n");
	send(clientSocket,&msg,sizeof(MsgData),0);	
}



void chatOne()		//私聊
{
	MsgData msg;	
	char tempMess[1024];
	char oneName[30];
	//加入在线人员
	lookOnlinePeople();
	while(1)
	{
		isChatOneOnline = 0;		//每次while将isChatOneOnline重置为0；
		msg.work = 4;
		printf("输入你想要私聊的人：(back退出私聊返群聊)\n");
		gets(oneName);
		if(strcmp(oneName,"back") == 0)
		{
			printf("正在返回群聊........\n");		
			return;
		}
		strcpy(msg.toName,oneName);
		strcpy(msg.fromName,myName);
		if(strcmp(msg.toName, msg.fromName) == 0)
		{
			printf("请勿和自己私聊！！\n");
			continue;
		}
		printf("检验中请等待........\n");
		send(clientSocket,&msg,sizeof(MsgData),0);
		sleep(1);		
		if(isChatOneOnline == 1)		//isChatOneOnline：若无此人就不开始私聊
		{
			printf("在线中没有找到此人\n");		
		}
		else
		{
			printf("您可以开始私聊了\n");
			break;		
		}
		memset(oneName,0,strlen(oneName));
	}
	msg.work = 5;//私聊用5
	while(1)
	{
		gets(tempMess);
		strcpy(msg.mess,tempMess);
		strcpy(msg.time,getTime());
		if(strcmp(tempMess,"back") == 0)
		{
			printf("正在返回群聊........\n");		
			return;
		}
		else
		{
			if(isChatOneOnline == 1)
			{
				printf("对方已不在群聊\n");
				break;
			}
			printf("你对 %s 偷偷说了：%s\n",msg.toName,msg.mess);		
			send(clientSocket,&msg,sizeof(MsgData),0);
		}
		memset(tempMess,0,sizeof(tempMess));
	}
}

void viewLocalChat()								//保存聊天记录分俩种，群聊私聊分开
{
	FILE *fp = fopen("localChat.txt","r");
	chatFile temp;
	int ret = fread(&temp,sizeof(chatFile),1,fp);
	while(ret > 0)
	{
		printf("%s:",temp.time);
		printf("\t%s:",temp.fromName);
		printf(" 对%s:",temp.toName);
		printf("说：%s\n",temp.mess);	
		ret = fread(&temp,sizeof(chatFile),1,fp);
	}
	fclose(fp);
}


void sendFile()
{
	MsgData msg;
	msg.work = 13;
	char buf[1024] = {0};
	char tempName[30];
	char txt[30];
	printf("请输入要发送的对象昵称\n");
	gets(tempName);
	strcpy(msg.toName,tempName);
	printf("输入要传的文件名(加后缀)\n");
	gets(txt);
	strcpy(msg.fileName,txt);
	sprintf(buf,"%s",txt);
	int pd = open(buf, O_RDONLY|O_CREAT);
    if (pd == -1)
    {
        perror ("error");
        return;
    }	
	int ret = 0;	
    memset(buf,0,sizeof(buf));	
    ret = read(pd, buf, 1024);
    buf[ret] = '\0';
    strcpy(msg.mess,buf);	
	strcpy(msg.fromName,myName);
	send(clientSocket,&msg,sizeof(MsgData),0);	
	close(pd);	
}


void chatManual()  //群聊手册
{
	system("clear");
	printf("********************************************\n");
	printf("\t群聊操作指南\n");
	printf("q2：进入私聊模式（私聊模式下输back返回群聊）\n");
	printf("q3：查看在线人员\n");
	printf("q4：查看本地聊天记录\n");
	printf("q5：进入管理员操作\n");
	printf("q6：发送文件\n");
	printf("群聊模式下输入bye退出群聊返回主程序\n");
	printf("********************************************\n");
}



void chatAll()				//群聊功能
{
	MsgData msg;	
	char tempMess[1024];	
	msg.work = 3;
	msg.flag = 1;
	strcpy(msg.name,myName);
	system("clear");
	printf("群聊\n");
	sprintf(msg.mess,"进入了群聊");	//被禁言后，这句发不出去，会提示你被禁言
	send(clientSocket,&msg,sizeof(MsgData),0);	
	memset(msg.mess,0,strlen(msg.mess));		
	while(1)
	{
		msg.flag = 0;
		gets(tempMess);		
		strcpy(msg.mess,tempMess);		
		strcpy(msg.time,getTime());	
		if (strcmp(msg.mess,"bye") == 0)					//bye退出群聊操作；
		{
			memset(tempMess,0,sizeof(tempMess));		
			sprintf(msg.mess,"退出了群聊\n");
			send(clientSocket,&msg,sizeof(MsgData),0);		//将退出的消息发给公频
			memset(msg.mess,0,strlen(msg.mess));	
			strcpy(msg.mess,"bye");						//bye退出在公频的在线人数；
			send(clientSocket,&msg,sizeof(MsgData),0);
			break;
		}
		else if(strcmp(msg.mess,"q2") == 0)				//q2进入私聊模式；
		{
			chatOne();	
			sleep(1);
			printf("您已返回群聊\n");			
		}
		else if(strcmp(msg.mess,"q3") == 0)				//查看在线人员
		{
			lookOnlinePeople();
			sleep(1);
			printf("\n您已返回群聊\n");	
		}
		else if(strcmp(msg.mess,"q4") == 0)				//查看聊天记录
		{
			viewLocalChat();								//改为文件
			sleep(1);
			printf("\n您已返回群聊\n");	
		}
		else if(strcmp(msg.mess,"q6") == 0)			//发送文件
		{
			sendFile();
			sleep(1);
			printf("您已返回群聊\n");
		}
		else if(strcmp(msg.mess,"help") == 0)	
		{
			chatManual();							//手册
			sleep(1);
			printf("您已返回群聊\n");
		}			
		else
		{
			send(clientSocket,&msg,sizeof(MsgData),0);
			//sleep(1);
		}
		memset(msg.mess,0,strlen(msg.mess));	
		memset(msg.time,0,strlen(msg.time));	
		memset(tempMess,0,sizeof(tempMess));	
	}	
}



int enterAccount()												//登入
{
	MsgData msg;	
	msg.work = 2;	
	char tempAccount[30];
	char tempPassword[30];	//如果超过，不安全
	printf("输入你的账号：");
	/*
		myGets(tempAccount);
	*/
	gets(tempAccount);
	printf("输入你的密码：");
	gets(tempPassword);
	strcpy(msg.account,tempAccount);
	strcpy(msg.pass,tempPassword);	
	send(clientSocket,&msg,sizeof(MsgData),0);	
	if(recv(clientSocket,&msg,sizeof(MsgData),0) <= 0)//每次都判断
	{
		printf("服务器断开链接\n");
		exit(-1);
	}	
	if(msg.flag == 1)
	{
		printf("\n");
		printf("欢迎你%s\n",msg.name);
		printf("\n");
		strcpy(myName,msg.name);	
		strcpy(myAccount,msg.account);
	}
	else if(msg.flag == 2)
	{
		printf("该账号已登入\n");
		return 0;
	}
	else if(msg.flag == 3)
	{
		printf("密码错误\n");
		return 0;
	}
	else if(msg.flag == 5)
	{
		printf("无此用户\n");
		return 0;
	}
	else
	{
		printf("系统错误,请重试\n");
		return 0;
	}
	return 1;
}


void secondMenuAndAction()
{
	char getWork[5];
	int actions;
	int flag = 0;
	while(1)
	{	
		sleep(1);
		printf("\t****************\n");
		printf("\t3.进入群聊\n");
		printf("\t6.离线消息接收\n");
		printf("\t5.退出到主程序\n");
		printf("\t****************\n");
		
		gets(getWork);
		actions = atoi(getWork);
		
		switch(actions)
		{
			case 3:printf("actions = %d\n",actions);chatAll();break;
			case 5:linkOffline();flag = 1;break;
		}
		
		if(flag == 1)
		{		
			bossFlag = 1;
			MsgData msg;
			msg.work = 16;
			send(clientSocket,&msg,sizeof(MsgData),0);
			break;
		}
	}
}

void linkOffline()
{
	MsgData msg;	
	msg.work = 15;	
	strcpy(msg.name,myName);	
	send(clientSocket,&msg,sizeof(MsgData),0);	
}


void actions()
{
	char getWork[5];//用字符串代替int防止输入错误
	int actions;
	while(1)
	{	
		firstMenu();
		printf("输入操作:");
		gets(getWork);
		actions = atoi(getWork);
		switch(actions)
		{
			case 1://注册
					registerNewAccount();				
				break;					
			case 2://登入
					if(enterAccount() == 1)					
					{
						bossFlag = 0;//这是啥？
						pthread_t pId;				//启用线程
						pthread_create(&pId,NULL,recvThread,(void *)clientSocket);
						pthread_detach(pId);
						secondMenuAndAction();					
					}
				break;	
			case 6:
					linkOffline();
					exit(0);							//退出
				break;
		}
		memset(getWork,0,strlen(getWork));
	}
}




int main(int argc,char *argv[])
{
	if(argc != 2)
	{
		printf("请加上IP地址\n");
		exit(-1);
	}
	else
	{
		printf("IP = %s\n",argv[1]);
		strcpy(IP,argv[1]);
		printf("IP =  %s\n",IP);
	}
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == -1)
    {
        perror ("socket");
        return -1;
    }
	struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family  = AF_INET;     // 设置地址族
    addr.sin_port    = htons(PORT); // 端口号转换为网络字节序
    inet_aton(IP,&(addr.sin_addr));
	//addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	int ret = connect(clientSocket, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1)
    {
        perror ("connect errer");
        return -1;
    }
	printf("成功连上服务器\n");
	actions();
    close(clientSocket);
    return 0;	
}


