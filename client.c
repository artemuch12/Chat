#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

extern int errno;

int id_sc;	//Сервер -> Клиент
int id_cs;	//Клиент -> Сервер
int id_tech_sc;	//tech Сервер -> Клиент
int id_tech_cs;	//tech Клиент -> Сервер
int flag = 0;


pthread_t tid_tech;
pthread_t tid_print;



struct tech
{
	long type;
	char nick[50];
	int status;
};
struct users
{
	long type;
	char nick[50];
	char message[50];
};

struct keys
{
	long type;
	key_t key_cs;
	key_t key_sc;
};


struct tech info;
struct users inp_text;
struct keys keys_wr;

void err_id(int id)
{
	if(id == -1)
	{
		printf("Error.\n");
		exit(-1);
	}
}

void *tech_message()
{
	ssize_t mes_prov;
	while(flag != 1)
	{
		mes_prov = msgrcv(id_tech_sc, &inp_text, sizeof(inp_text), 1, 0);
		msgsnd(id_tech_cs, &info, sizeof(info), 0);
	}
	pthread_cancel(tid_tech);
}

void *text_print()
{
	ssize_t mes_prov;
	while(flag != 1)
	{
		mes_prov = msgrcv(id_sc, &inp_text, sizeof(inp_text), 1, 0);
		printf("%s:  %s\n", inp_text.nick, inp_text.message);
	}
	pthread_cancel(tid_print);
	
}


int main()
{
	void *status;
	key_t key_sc;
	key_t key_cs;
	key_t key_tech_sc;
	key_t key_tech_cs;
	
	char command[255];
	
	key_tech_sc = ftok("./server", 'S');
	key_tech_cs = ftok("./server", 'C');
	
	id_tech_sc = msgget(key_tech_sc, 0660);
	err_id(key_tech_sc);
	id_tech_cs = msgget(key_tech_cs, 0660);
	err_id(key_tech_cs);
	puts("Enter nickname");
	fgets(info.nick, 50, stdin);
	strcpy(inp_text.message, info.nick);
	info.type = 1;
	info.status = 1;
	msgsnd(id_tech_cs, &info, sizeof(info), 0);
	msgrcv(id_tech_sc, &keys_wr, sizeof(keys_wr), 2, 0);
	id_sc = msgget(key_sc, 0660);
	id_cs = msgget(key_sc, 0660);
	pthread_create(&tid_tech, NULL, tech_message, NULL);
	pthread_create(&tid_print, NULL, tech_message, NULL);
	while(flag != 1)
	{
		fgets(command, 255, stdin);
		if(strcmp(command, "!exit!") == 0)
		{
			flag = 1;
		}
		else
		{
			strcpy(inp_text.message, command);
			
		}
		msgsnd(id_cs, &inp_text, sizeof(inp_text), 0);
	}
	pthread_join(tid_tech, &status);
	pthread_join(tid_print, &status);
	exit(0);
}
