#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#define MAX_TOKENS_SIZE 1024
#define COMMAND_DELIM 10
#define MAX_PATH 256

void cd(const char* dir);
void reaper(int sig);

int main ()
{
	const char * delim =" \n";
	char *buffer=NULL;
	char ** tokens;
	char* ptr_token;
	size_t len=0;
	char enter_error=0;
	char path_command[MAX_PATH];
	int* commandline_end;
	int exit_terminal=0;
	
	struct sigaction act_CHLD;
	struct sigaction oact_CHLD;
	sigaction(SIGCHLD,NULL,&oact_CHLD);
	act_CHLD.sa_handler=reaper;
	sigemptyset(&act_CHLD.sa_mask);
	act_CHLD.sa_flags = SA_NOCLDSTOP;
	sigaction(SIGCHLD,&act_CHLD,NULL);
	
	struct sigaction act_INT;
	struct sigaction oact_INT;
	sigaction(SIGINT,NULL,&oact_INT);
	act_INT.sa_handler=SIG_IGN;
	sigaction(SIGINT,&act_INT,NULL);
	 
	tokens=(char **)malloc(MAX_TOKENS_SIZE*sizeof(char));
	if (tokens==NULL)
	{
		perror("Memory error: ");
		return EXIT_FAILURE;
	}
	
	commandline_end=malloc(sizeof(int)*COMMAND_DELIM);
	if (commandline_end==NULL)
	{
		free(tokens);
		perror("Memory error: ");
		return EXIT_FAILURE;
	}
	
	do
	{
		if (exit_terminal)
			break;
		printf("myterminal$\n");
		getline (&buffer,&len,stdin);

		if (buffer[0]=='\n')
			continue;
			
		int token_counter=0;
		char*saveptr;
		ptr_token=strtok_r(buffer,delim,&saveptr);
		tokens[token_counter]=ptr_token;
		token_counter++;
		
		int cycle_exit=0; 
		while (1)
		{
			if ((ptr_token=strtok_r(NULL,delim,&saveptr))==NULL)
				cycle_exit=1;
			
			if (token_counter>=MAX_TOKENS_SIZE)
			{
				char ** re_tokens;
				re_tokens=(char 	**)realloc(tokens,sizeof(char*)*token_counter*2);
				if (re_tokens==NULL)
				{
						perror("Input is too long. Memory error: ");
						free(buffer);
						free(tokens);
						return EXIT_FAILURE;
				}
				tokens=re_tokens;
			}
			tokens[token_counter]=ptr_token;
			token_counter++;
			if (cycle_exit)
				break;

		}
		

		int commandend_counter=0;
		
		for (int i=0;i<token_counter;i++)
		{
			if (commandend_counter>=COMMAND_DELIM)
			{
				int * re_commandline_end;
				re_commandline_end=(int*)realloc(commandline_end,sizeof(int)*commandend_counter*2);
				
				if (re_commandline_end==NULL)
					{
						perror("Memory error: ");
						free(tokens);
						free(buffer);
						free(commandline_end);
						return EXIT_FAILURE;
					}
				commandline_end=re_commandline_end;
			}
			if(tokens[i]==NULL|| strcmp(tokens[i],";")==0 || strcmp(tokens[i],"&")==0)
			{

					if(commandend_counter!=0 && tokens[i]!=NULL && commandline_end[commandend_counter-1]==i-1)
					{
						printf("Syntax error!\n");
						enter_error=1;
						free(commandline_end);
						break;	
					}
					
					if(commandend_counter!=0 && tokens[i]==NULL && commandline_end[commandend_counter-1]==i-1)
						break;	
					
						commandline_end[commandend_counter]=i;
						commandend_counter++;
			}
		}
		
		if (enter_error)
		{
			enter_error=0;
			continue;
		}
		printf("%d\n",commandend_counter);
		for (int i=0,command_file=0;i<commandend_counter;i++)
		{
			int background=0;
			pid_t pid;
			char* ptr;
			printf("%s\n",tokens[command_file]);
			printf("%d\n",i);
			if (tokens[commandline_end[i]]!=NULL && strcmp(tokens[commandline_end[i]],"&")==0)
				background=1;
				tokens[commandline_end[i]]=NULL;

			if(strcmp(tokens[command_file],"exit")==0)
			{
				exit_terminal=1;
				break;	
			}
				
			
			if (strcmp(tokens[command_file],"cd")==0)	
			{
				cd(tokens[command_file+1]);
				command_file=commandline_end[i]+1;
				continue;
			}
			strcpy(path_command,tokens[command_file]);
			printf("%s\n",path_command);
			if ((ptr=strrchr(tokens[command_file],'/'))!=NULL)
			{
				strcpy(tokens[command_file],ptr+1);
			}
			
			pid=fork();
			
			if (pid<0)
			{
				perror("fork error:");
      	break;
			}
			else	if (pid==0)
			{
				sigaction(SIGCHLD,&oact_CHLD,NULL);
				
				if (!background)
				{
					sigaction(SIGINT,&oact_INT,NULL);
				}
				
				if (execvp(path_command, tokens+command_file) == -1) 
				{
      		perror("exec error:");
					exit(EXIT_FAILURE);
    		}
			}
			else if (pid>0)
			{
				if (!background)
					waitpid(pid,(int*)0,0);
			}
			command_file=commandline_end[i]+1;
		}	
	}
	while (1);
	
		free(buffer);
		free(tokens);
		free(commandline_end);

	return EXIT_SUCCESS;
}

void cd(const char* dir)
{
	if (dir==NULL)
	{
		printf("Error: there is no argument");
	}
	else

	if (chdir(dir) != 0) 
	{
    perror("cd error:");
	}
	return;
}

void reaper (int sig)
{
	pid_t pid;
	int stat;
	
	while((pid=waitpid(-1,&stat,WNOHANG))>0)
	{}
	return;
}

