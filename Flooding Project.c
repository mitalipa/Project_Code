//Required Libraries
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<thread.h>
#include<unistd.h>
#define N 10 
#define MODE "r"
#define TIME 1000 

//global variables
int totalPeers;
int network[N+2][N+2];
FILE *logFile;
FILE *fp;
int numberOfMsgLeft=0;
int EMPTY;
struct Message
{
	unsigned int sender;
	unsigned int TTL;
	unsigned int msg;
	struct Message *next;
};

struct Message *HeadQueue[N+2],*TailQueue[N+2];
mutex_t lock[N+2],msgLock,fileLock;
thread_t tid,thr[N+2];
void *peer(void *argument)
{
	struct Message* getMessage(int);
	int sendMessage(int ,int ,int ,int );
	int currentThreadId=(int)argument;
	int neighbors[N+2],i,sender;
	struct Message *node;
	for(i=0;i<totalPeers+2;i++)
		neighbors[i]=network[currentThreadId][i];
	printf("\nNeighbors of threadId %d :",currentThreadId);
	for(i=0;i<totalPeers+2;i++)
		printf("%d ",neighbors[i]);
	usleep(100);
	int checkMsgInQueue=0;
	float msgNotFound=0,msgFound=0,average=0;
	while(checkMsgInQueue<EMPTY)
	{
	if(HeadQueue[currentThreadId]==NULL)
	{
		checkMsgInQueue++;
		//printf("%d=%d ",currentThreadId,checkMsgInQueue);
	}
	else
	
	while(HeadQueue[currentThreadId]!=NULL)
	{
	msgNotFound+=checkMsgInQueue;
	if(checkMsgInQueue!=0) msgFound++;		
	node=getMessage(currentThreadId);
	if(node!=NULL)
	{
		if((node->TTL) >=1)
		{
			node->TTL--;	
			sender=node->sender;
			for(i=0;i<totalPeers+2;i++)
			{
				if(i!=sender && neighbors[i]==1)
				{	
					sendMessage(currentThreadId,i,node->msg,node->TTL);
					mutex_lock(&fileLock);
					fprintf(logFile,"%d %d %d %d %d\n",sender,currentThreadId,i,node->msg,node->TTL);
					mutex_unlock(&fileLock);
				}
			}
		}	
	}
	free(node);
	checkMsgInQueue=0;
	usleep(10);
	}
	usleep(TIME);
	}
	if(msgFound!=0) average=msgNotFound/msgFound;
	printf("\nThread %d,avg #rounds Queue was empty=%10f",currentThreadId,average);
	return NULL;
}
	
int main(int argc,char* argv[])
{
	if(argv[2]==NULL)
	{
		printf("Please enter value of num_Empty!!!\n");
		return 1;
	}
	if(argv[1]==NULL)
	{	
		printf("Please enter File Name!!!\n");
		return 1;
	}
	EMPTY=atoi(argv[2]);
	int readInputFile(char *);
	int displaynetwork();	
	int sendMessage(int ,int ,int ,int );
	int enterQueue(struct Message *,int );
	int displayMessage(struct Message *);
	int checkQueue();	
	struct Message* getMessage(int);
	int initializeQueues();
	int broadcastMessage();
	logFile=fopen("stage7_log","w");
	int statusRead=readInputFile(argv[1]);
	if(statusRead==0)
		displaynetwork();	
	printf("\n");
	//creating totalPeers thread
	int i,status;
	for (i=2; i<totalPeers+2; i++)
 		 thr_create(NULL, 0, peer, (void*)i, THR_BOUND, &thr[i]);
	printf("\n");	
	broadcastMessage();
	//wait for all threads to finish
	while (thr_join(0, &tid, (void**)&status)==0)
  		printf("\n[Status of thread Id %d is=%d]", tid,status);

	
	checkQueue();	
	fclose(logFile);
	fclose(fp);
	return 0;
}
//reads inputfile and sets network array as per the connectivitiy
int readInputFile(char *fileName)
{
	char str[60],s[2]=" ";
	int i,totalNeighbor,temp=1;
	if(fileName==NULL)
	{
		printf("Please enter FileName!!!\n");
		return 1;
	}
	fp=fopen(fileName,MODE);
	if(fp==NULL)
	{
		printf("Error while opening a file!!!\n");
		return 1;
	} 
	fgets(str,60,fp);
	sscanf(str,"%d",&totalPeers);
	for(i=1;i<=totalPeers;i++)
	{
		fgets(str,60,fp);
		totalNeighbor=atoi(strtok(str,s));
			temp=1;
			while(temp<=totalNeighbor)
			{
				int neighbor=0;
				neighbor=atoi(strtok(NULL,s));
				network[i][neighbor]=1;
				network[neighbor][i]=1;
				temp++;
			}

	}		
return 0;	
}
//displays the network
int displaynetwork()
{
	system("clear");
	int i,j;
	printf("Network mapping between peers is :\n\n"); 
	for(i=0;i<N+2;i++)
	{
		printf("%3d  | ",i);
		for(j=0;j<N+2;j++)
		{
			printf("%d ",network[i][j]);
		}
		printf("\n");
	}
return 0;	

}
//will initializes both the queues with NULL at the start of the program
int initializeQueues()
{
	memset(HeadQueue,NULL,sizeof(HeadQueue));
	memset(TailQueue,NULL,sizeof(TailQueue));
	return 0;	
}
int sendMessage(int sender,int receiver,int msg,int TTL)
{
	int enterQueue(struct Message *,int );
	struct Message *nextNode;
	nextNode=(struct Message *)malloc(sizeof(struct Message));
	nextNode->sender=sender;
	nextNode->TTL=TTL;
	nextNode->msg=msg;
	nextNode->next=NULL;
	enterQueue(nextNode,receiver);

	return 0;
}
int enterQueue(struct Message *nextNode,int receiver)
{
	mutex_lock(&msgLock);
	numberOfMsgLeft++;
	mutex_unlock(&msgLock);
	//Lock receiver
	mutex_lock(&lock[receiver]);
	//first node in the receiver's queue
	if(HeadQueue[receiver]==NULL && TailQueue[receiver]==NULL)
	{
		HeadQueue[receiver]=nextNode;
		TailQueue[receiver]=nextNode;
	}//other than first node
	else
	{
		TailQueue[receiver]->next=nextNode;
		TailQueue[receiver]=TailQueue[receiver]->next;
	}
	//Unlock receiver
	mutex_unlock(&lock[receiver]);	
	return 0;	
}
struct Message* getMessage(int receiver)
{
	//Lock receiver
	mutex_lock(&lock[receiver]);
	
	struct Message *node=NULL;
	if(HeadQueue[receiver]!=NULL)
	{
		node=HeadQueue[receiver];
		HeadQueue[receiver]=HeadQueue[receiver]->next;
		if(HeadQueue[receiver]==NULL)
			TailQueue[receiver]=NULL;
		node->next=NULL;
	}
	
	//Unlock receiver
	mutex_unlock(&lock[receiver]);	

	mutex_lock(&msgLock);
	if(node!=NULL) numberOfMsgLeft--;
	mutex_unlock(&msgLock);
	return node;		
}
//displays a particular message by providing a particular node os a queue
int displayMessage(struct Message *node)
{
	if(node!=NULL)
		 printf ("Sender is %d,TTL is %d and msg is %d.\n",node->sender,node->TTL,node->msg); 
	return 0;	
}
//it will check whether quueue is empty or not and accordingly msg will be displayed
int checkQueue()
{
	int i;int flag=0;
	printf("\n");
	for(i=0;i<N+2;i++)
	{
		if(HeadQueue[i]!=NULL)
		{
			printf("\nQueue of peer %d is NOT empty!!\n",i);
			flag=1;
		}
	 mutex_destroy(&lock[i]);
	}
if(flag==0)
printf("\nAll Queues are Empty!!! ");
printf("\nDestroyed all locks!!\n");
return 0;
}
int broadcastMessage()
{
	while(1)
	{
		int msgToSend=0,TTL=0;
		char str[60];
		fgets(str,60,fp);
		if(feof(fp))
			break;
		printf("\n\nNow sending Message ... %s",str);
		sscanf(str,"%d %d",&msgToSend,&TTL);
		if(sendMessage(1,2,msgToSend,TTL)==0)
			printf("Message sent successfuly!!\n\n");
		memset(str,'\0',sizeof(str));
		usleep(TIME);
	}
return 0;
}
