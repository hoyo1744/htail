

//tail-f
//실행중 검색가능 기능 추가
//검색된 단어 색기능 표시
//정지가능
//시작가능


#include "tail_define.h"

int loopFlag=TRUE;	
int wd;
int eventfd;
FILE* fp;
char fileName[FILE_NAME_SIZE];
char command[FILE_BUFF];
struct sigaction act;
pthread_mutex_t mutex;
string start="\033[1;32m";
string endd="\033[1;0m";


void setColorSearchText(char* target,char* search)
{
	
	string strTarget=target;
	string strSearch=search;

	int npos=0;
	
	while((npos=strTarget.find(search,npos))>=0)
	{
		if(npos==strTarget.npos)
			break;
		if(npos>=strTarget.size())
			break;

		strTarget.insert(npos,start);
		strTarget.insert(npos+start.size()+strSearch.size(),endd);
		npos+=start.size()+1;
	}
	printf("%s",strTarget.c_str());

}
void setTextColorPrint(int color,char* str)
{
	//테스트용 코드
	printf("%c[1;%dm",27,color);
	printf("%s",str);
	printf("%c[1;0m",27);
}


void showStartLog()
{
	printf("=============================================\n");
	printf("StartLog\n");
	printf("=============================================\n");
}

void showEndLog()
{
	printf("=============================================\n");
	printf("EndLog\n");
	printf("=============================================\n");
}


void closeHandler()
{
	loopFlag=false;

}
//강제종료시 시그널 핸들러
void processCloseHandler(int signo)
{
	//로그 끝	
	showEndLog();
	inotify_rm_watch(eventfd,wd);
	close(eventfd);
	fclose(fp);
	
}

//객체초기
void init()
{
	loopFlag=TRUE;	
	wd=0;
	eventfd=0;
	fp=NULL;
	memset(fileName,0,sizeof(fileName));
	memset(command,0,sizeof(command));
	pthread_mutex_init(&mutex,NULL);	
}

//시그널관련 초기화
void signalInit()
{
	//시그널선언
	act.sa_handler=processCloseHandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	sigaction(SIGINT,&act,0);
}


//검색하고자 하는 단어
//멀티쓰레드
void enteredCommand(char* cmd)
{	
	memset(command,0,sizeof(command));
	int ans=snprintf(command,strlen(cmd)+1,cmd);
	
	if(ans<0)
	{
		perror("snprintf");
		exit(1);
	}




}

void* thread_main(void* arg)
{
	int i;
	int cnt=*((int*)arg);
	char str[FILE_BUFF];
	memset(str,0,sizeof(str));
	

	while(1)
	{
		scanf("%s",str);
		printf("입력 단어 : %s\n",str);
		
		//검색단어 초기화
		if(str==searchInit)
			memset(command,0,sizeof(command));

		//프로세스 종료
		if(str==searchClose)
			closeHandler();



		if(strlen(str)!=0)
		{
			pthread_mutex_lock(&mutex);
			enteredCommand(str);
			memset(str,0,sizeof(str));
			pthread_mutex_unlock(&mutex);				
		}
	}
	return NULL;


}
int main(int argc,char* argv[])
{
	//value init
	init();

	//signal init
	signalInit();

	pthread_t pid=0;

	int thread_param=5;

	if(pthread_create(&pid,NULL,thread_main,(void*)&thread_param)!=0)
	{
		perror("pthread_create");
		exit(1);
	}
	
	if(pthread_detach(pid)!=0)
	{
		perror("pthread_detach");
		exit(1);
	}

	if(argc==1)
	{
		printf("Usage : please input filename\n");
		exit(1);
	}


	sprintf(fileName,"%s",argv[1]);
	fp=fopen(fileName,"r");
	if(fp==NULL)
	{
		printf("FileOpenError");
		exit(1);
	}

	//파일 탐색
	fseek(fp,0,SEEK_END);

	eventfd=inotify_init();
	if(eventfd<0)
	{
		printf("inotify_init Fail()\n");
		exit(1);
	}

	wd=inotify_add_watch(eventfd,fileName,IN_MODIFY|IN_MOVE_SELF);
	if(wd<0)
	{
		printf("inotify_add_watch Fail()\n");
		exit(1);
	}




	//로그시작
	showStartLog();
	while(loopFlag)
	{
		fd_set fdrdset;
		FD_ZERO(&fdrdset);
		FD_SET(eventfd,&fdrdset);

		//#include <sys/time.h>의 이유
		struct timeval tv;
		tv.tv_sec=1;
		tv.tv_usec=0;
		int rc=select(eventfd+1,&fdrdset,(fd_set*)NULL,(fd_set*)NULL,&tv);

		if(rc==0)
			continue;
		if(rc<0)
		{
			printf("select Fail\n");
			exit(1);
			loopFlag=FALSE;
			break;
		}
		if(!FD_ISSET(eventfd,&fdrdset))
			continue;	

		char buff[FILE_BUFF];
		int readLen=read(eventfd,buff,sizeof(buff));
		if(readLen<0)
		{
			printf("event Read Fail()\n");
			loopFlag=FALSE;
			break;
		}
		int eventPos=0;
	
		struct inotify_event* event=(struct inotify_event*)&buff[eventPos];
		if(eventPos<readLen)
		{

			if(event->mask & IN_MODIFY)
			{

				char readbuff[FILE_BUFF];
				memset(readbuff,0,sizeof(readbuff));
				while(fgets(readbuff,sizeof(readbuff),fp)!=NULL)
				{
					//setTextColorPrint(COLOR_YELLOW,"log : ");
					int searchLen=strlen(command);
					if(searchLen==0)
						printf("%s",readbuff);
					else
					{
						char* pos=NULL;
						//검색하고자 하는 단어가 있다.
						if((pos=strstr(readbuff,command))!=NULL)
						{
							//단어가 현재 로그에 존재한다.
							setColorSearchText(readbuff,command);

						}
						else
						{
							printf("%s\n",readbuff);
						}
						


					}
					memset(readbuff,0,sizeof(readbuff));

				}

			}
			else if(event->mask & IN_DELETE || event->mask & IN_MOVE_SELF)
			{
				printf("File delete or move\n");
				loopFlag=FALSE;
				break;
			}
			event=(struct inotify_event*)(event+eventPos);
			eventPos+=(sizeof(*event)+event->len);
		}
		else
		{
			readLen=read(eventfd,buff,sizeof(buff));
			eventPos=0;

			if(readLen==0)
				continue;

		}
		eventPos+=(sizeof(*event)+event->len);
	}
	//로그 끝	
	showEndLog();
	inotify_rm_watch(eventfd,wd);
	close(eventfd);
	fclose(fp);


	return 0;

}

