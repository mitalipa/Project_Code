//Required Libraries
#include<stdio.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include <limits.h>
#include <sys/fs/ufs_fs.h>
#include <sys/fs/ufs_inode.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

//global variables
char *selectionTable[4]={"-name","-mtime","-user",NULL};
char *sel;
char *arg;
char **cmdList;
int indexOfPathName;
FILE *cmdExec;
int countOfCmdList=0;

//Prototypes
int processDirEntry(char *,char *,char *);
void printDirEntry(char *,char *);
int testForHyphen(char *);
void executeCommand();
int checkFileUsername(char *);
int checkFileTime(char *);

//This Function will visit the directory given in the paramter and prints the entry names and the modified time
int visitDir(char *source)	
{
	DIR *dir;
	struct dirent *dirEntry;
	struct stat buf;
	unsigned long ftype;
	char *fileNamePath;
	char *dirType="DIR";
	char *regType="REG";
	char *othType="Oth";
	dir=opendir(source);	
	if(dir==NULL)
	{	
		//Error While opening the directory
		perror(source);	
		return -1;
	}
	while(1)
	{
		dirEntry=readdir(dir);
		if(dirEntry == NULL)
			break;
		//concatenate the directory and the file name to find absolute path
		fileNamePath=(char *)malloc(sizeof(char)*(strlen(source)+strlen(dirEntry->d_name)+2));
		sprintf(fileNamePath,"%s/%s",source,dirEntry->d_name);	
		if(strcmp(dirEntry->d_name,".")==0 || strcmp(dirEntry->d_name,"..")==0)
			continue;
		if(lstat(fileNamePath,&buf)>=0)
		{
			ftype=buf.st_mode & 0xF000;
                	if(ftype==S_IFDIR)
			{
				processDirEntry(dirType,dirEntry->d_name,fileNamePath);	
                       		visitDir(fileNamePath);
			}	
			else if(ftype == S_IFREG)
			{
				processDirEntry(regType,dirEntry->d_name,fileNamePath);	
			}
			else
			{
				processDirEntry(othType,dirEntry->d_name,fileNamePath);	
			}
		}
		else
		{
			perror(fileNamePath);
			break;
		}
			
	free(fileNamePath);
	}
	if(closedir(dir)==0)
		return 0;
	else
	{
		//error while closing the directory
		perror(dirEntry->d_name);
		return -1;
	}	
}
/*This method return the status variable .status defines t
he outcome of this method. if status =0 processArgument me
thod
and was
successful and if it is -1 it failed in matching the selec
tion argument*/

int processArgument(int argc,char **argv)
{
	int i,N;
	//if the source folder is not given
	if(argv[1]==NULL)
                return 1;
	if(argc==2)
		printf("Source Argument = %s\n\n",argv[1]);
	if(argv[1]!=NULL && argv[2]!=NULL && argv[3]!=NULL)
                printf("source= %s ,selection= %s ,Argument= %s \n\n",argv[1],argv[2],argv[3]);
	if(argv[3]==NULL && argv[2]!=NULL)
	{
		printf("Argument missing\n");
		return 1;
	}
	if(argv[2]!=NULL && argv[3]!=NULL)
	{
		for(i=0;selectionTable[i]!=NULL;i++)
		{	
			if(strcasecmp(selectionTable[i],argv[2])==0)
			{
				sel=(char *)malloc(sizeof(char)*strlen(argv[2]));
				arg=(char *)malloc(sizeof(char)*strlen(argv[3]));
				if(sel==NULL || arg==NULL)
                                	return 1;
				strcpy(sel,argv[2]);
				strcpy(arg,argv[3]);
			}
		}
	}
	if(argc>4)
	{
		N=argc-3;
		cmdList=(char**)malloc(sizeof(char*)*N);

		cmdList[0]=(char *)malloc(sizeof(char)*(strlen(argv[4])-1));
		strcpy(cmdList[0],argv[4]+1);
		countOfCmdList++;
		i=5;
		int j=1;
		while(i<argc)
		{
			if(testForHyphen(argv[i])==0)	
			{
				cmdList[j]=(char *)malloc(sizeof(char)*strlen(argv[i]));
				strcpy(cmdList[j],argv[i]);
				countOfCmdList++;
			}
			else
				break;
		i++;
		j++;
		}
		indexOfPathName = j;
		if(i<argc)
		{
			cmdList[j+1]=(char *)malloc(sizeof(char)*strlen(argv[i]));
			strcpy(cmdList[j+1],argv[i]);
			countOfCmdList++;
		}
		countOfCmdList++;
	}
return 0;
}
//This method will check for - in the command option and command argument string
int testForHyphen(char *arg)
{
	int status=-1;
	char hyphen='-';
	if(arg[0]==hyphen)
		status=0;
	else
		status=1;
return status;	
}

void setPathCmdList(char *fileNamePath)
{
	cmdList[indexOfPathName]=NULL;
	cmdList[indexOfPathName]=(char *)malloc(sizeof(char)*strlen(fileNamePath));
	strcpy(cmdList[indexOfPathName],fileNamePath);
}
void executeCommand()
{
        int i=0,length=0;
        char *buffer;
        while(i<countOfCmdList)
        {
                length+=(strlen(cmdList[i++])+1);
        }
        buffer=(char *)malloc(sizeof(char)*(length-1));
	strcpy(buffer,cmdList[0]);
        i=1;
        while(i<countOfCmdList)
        {
                sprintf(buffer,"%s %s",buffer,cmdList[i++]);
        }
        fprintf(cmdExec,"%s\n",buffer);
        free(buffer);

}

void performAction(char *ftype,char *fileNamePath)
{
        if(cmdList==NULL || strcasecmp(ftype,"DIR")==0)
                printDirEntry(ftype,fileNamePath);
        else
        {
                setPathCmdList(fileNamePath);
                printDirEntry(ftype,fileNamePath);
                executeCommand();
        }
}
int processDirEntry(char *ftype,char * fileName,char *fileNamePath)
{
	if(sel==NULL)
	{
		printDirEntry(ftype,fileNamePath);
		return 0;
	}
	else
	{
		if(strcmp(sel,selectionTable[0])==0)
		{
			if(strcasecmp(fileName,arg)==0)
				performAction(ftype,fileNamePath);
			return 0;
		}
		if(strcasecmp(sel,selectionTable[1])==0)
		{
			if(checkFileTime(fileNamePath)==0)
				 performAction(ftype,fileNamePath);
			return 0;
		}
		if(strcasecmp(sel,selectionTable[2])==0)
		{
			if(checkFileUsername(fileNamePath)==0)
				 performAction(ftype,fileNamePath);
			return 0;
		}
	}
return 0;
}
/*This method will compare the file name with the file name in the source folder and status is returned to the calling function*/
int checkFileUsername(char *name)
{
	struct stat buf;
	int status=-1;	
	struct passwd *pass;
	if(lstat(name,&buf)>=0)
	{
		pass=getpwuid(buf.st_uid);
		if(pass!=NULL)
		{
			if(strcmp(arg,pass->pw_name)==0)
				status=0;
		}
		
	}
return status;
}
/*This method will compare the file time with file name in the source folder */
int checkFileTime(char *name)
{
	struct stat buf;
	int status = -1;
	int currentTime,t;
	int argTime=(atoi(arg)*24*60*60);
	currentTime =time(NULL);
	if(lstat(name,&buf)>=0)
	{
		if((currentTime-buf.st_mtime)<=argTime)
		{
			t=buf.st_mtime;
			status = 0;
		}
	}
return status;
}
void printDirEntry(char *ftype,char *fileNamePath)
{
			printf("%-10s %-10s\n",ftype,fileNamePath);
}
int main(int argc,char *argv[])
{
	unsigned int ftype;
	struct stat buf;
	cmdExec=fopen("cmdExec.sh","w");
	if(argc<2)
	{
		printf("Invalid source\n");
		return 1;
	}
	//Processing slection and argument before encountering the directories
	if(processArgument(argc,argv)!=0)
	{
		printf("Check selection/Arg\n");
		return 1;
	}
	else
	if(lstat(argv[1],&buf)>=0)
	{
		ftype=buf.st_mode & 0xF000;
		if(ftype==S_IFDIR)
		{
			processDirEntry("DIR",argv[1],argv[1]);
			visitDir(argv[1]);
		}
		else
                        printf("Argument is not a directory\n");

	}
	else
	{
		perror(argv[1]);
		return 1;
	}
	fclose(cmdExec);
	system("sh -f cmdExec.sh");
	return 0;
}
