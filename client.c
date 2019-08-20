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
char nickname[50];
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

struct tech info;


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
		mes_prov = msgrcv(id_tech_sc, &info, sizeof(info), 2, 0);
		strcpy(info.nick, nickname);
		info.status = 1;
		info.type = 3;	//Посылка подтвержение
		msgsnd(id_tech_cs, &info, sizeof(info), 0);
	}
	pthread_cancel(tid_tech);
}

void *text_print()
{
	struct users inp_text; // Входящие сообщения
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
	struct users out_text;
	
	/*Ининциализируем ключи для 4 очередей сообщений*/
	key_tech_sc = ftok("./server", 'S');
	key_tech_cs = ftok("./server", 'C');
	key_sc = ftok("./server", 'R');
	key_cs = ftok("./server", 'W');
	
	/*Подключаемя к ОчерСооб*/
	id_tech_sc = msgget(key_tech_sc, 0660);
	err_id(key_tech_sc);
	id_tech_cs = msgget(key_tech_cs, 0660);
	err_id(key_tech_cs);
	id_sc = msgget(key_sc, 0660);
	err_id(id_sc);
	id_cs = msgget(key_cs, 0660);
	err_id(id_cs);
	
	/*Создаем "Имя пользозователя и отправляем его на сервер"*/
	puts("Enter nickname");
	fgets(nickname, 50, stdin);
	strcpy(info.nick, nickname);
	info.type = 1;		//Тип 1 - запрос на подключение
	info.status = 1; 	//Статус 1 - говорит, что пользоваетель активен
	msgsnd(id_tech_cs, &info, sizeof(info), 0);
	
	/*Создаем два вспомогательных потока: для технической связи и для 
	 * печати новых сообщений на экран*/
	pthread_create(&tid_tech, NULL, tech_message, NULL);
	pthread_create(&tid_print, NULL, tech_message, NULL);
	
	/*В главном потоке будем считывать вводимые сообщения до тех пор 
	 * пока не будет набрана команда "!exit!". Считанное сообщение 
	 * отправляется на сервер*/
	while(flag != 1)
	{
		fgets(command, 255, stdin);
		if(strcmp(command, "!exit!\n") == 0)
		{
			flag = 1;
		}
		else
		{
			strcpy(out_text.message, command);
			out_text.type = 1;
			strcpy(out_text.nick, nickname);
			msgsnd(id_cs, &out_text, sizeof(out_text), 0);
		}
		
	}
	
	/*Дожидаемся закрытия всех потоков*/
	pthread_join(tid_tech, &status);
	pthread_join(tid_print, &status);
	exit(0);
}
