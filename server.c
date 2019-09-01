#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <pthread.h>




pthread_t tid;
int flag = 0;
int id_sc;	//Сервер -> Клиент
int id_cs;	//Клиент -> Сервер
int id_tech_sc;	//tech Сервер -> Клиент
int id_tech_cs;	//tech Клиент -> Сервер
int polzov = 1;

ssize_t mes_prov; 

struct tech
{
	long type;
	char nick[255];
	int status;
};
struct users
{
	long type;
	char nick[255];
	char message[255];
};



struct tech user_info[255];

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
	struct tech info;
	errno = 0;
	int i = 0, j = 0;
	ssize_t mes_prov;
	while(flag != 1)
	{
		/*Проверяем есть ли запросы на присоединение*/
		mes_prov = msgrcv(id_tech_cs, (void *)&info, sizeof(info), 1L, IPC_NOWAIT);
		if((mes_prov != -1))
		{
			user_info[polzov].status = info.status;
			strcpy(user_info[polzov].nick, info.nick);
			polzov++;
			printf("User %s add\n", info.nick);
			printf("Users %d\n", polzov);
		}
		sleep(2);
		/*Сбрасываем статусы всех пользователей на ноль, чтобы в дальне-
		 * шем определить кто не откликнулся. Отсылаем посылки.*/
		for(i = 0; i < polzov; i++)
		{
			user_info[i].status = 0;
			info.type = 2L;
			msgsnd(id_tech_sc, (void *)&info, sizeof(info),  0);
		}
		sleep(2);
		/*Смотрим кто ответил*/
		for(i = 0; i < polzov; i++)
		{
			mes_prov = msgrcv(id_tech_cs, (void *)&info, sizeof(info), 3L, IPC_NOWAIT);
			if(mes_prov != -1)
			{
				for(j = 0; j < polzov; j++)
				{
					if(strcmp(user_info[j].nick, info.nick) == 0)
					{
						user_info[j].status = info.status;
					}
				}
			}
		}
		/*Убираем не ответивших*/
		for(j = 0, i = 0; i < polzov; i++)
		{
			if(user_info[i].status != 1)
			{
				printf("User %s delete\n", user_info[i].nick);
				for(j = i; j < polzov-1; j++)
				{
					user_info[j].status = user_info[j+1].status;
					strcpy(user_info[j].nick, user_info[j+1].nick);
				}
				polzov--;
				printf("Users %d\n", polzov);
			}
		}
		sleep(1);
		if(polzov == 0)
		{
			flag = 1;
		}
	}
	pthread_cancel(tid);
}


int main()
{
	struct tech info;
	errno = 0;
	void *status;
	key_t key_sc;
	key_t key_cs;
	key_t key_tech_sc;
	key_t key_tech_cs;
	
	struct users inp_text;
	struct users out_text;
	int i = 0;
	/*Генерируем 3 ключа: 1 - Сервер -> Клиент; 2 - Клиент -> Сервер; 
	 * 3 - технический*/
	key_sc = ftok("./server", 'R');
	key_cs = ftok("./server", 'W');
	key_tech_sc = ftok("./server", 'S');
	key_tech_cs = ftok("./server", 'C');
	
	id_sc = msgget(key_sc, IPC_CREAT | 0666);
	err_id(id_sc);
	id_cs = msgget(key_cs, IPC_CREAT | 0666);
	err_id(id_cs);
	id_tech_sc = msgget(key_tech_sc, IPC_CREAT | 0666);
	err_id(key_tech_sc);
	id_tech_cs = msgget(key_tech_cs, IPC_CREAT | 0666);
	err_id(key_tech_cs);
	
	/*Ожидаем первого клиента*/
	msgrcv(id_tech_cs, (void *)&info, sizeof(info), 1L, 0);
	/*Получаем ник и статус пользователя*/
	user_info[0].status = info.status;
	strcpy(user_info[0].nick, info.nick);
	printf("User %s add\n", info.nick);
	/*Запускаем основную работу сервера*/
	pthread_create(&tid, NULL, tech_message, NULL);
	printf("Activeted technikal channel.\n");
	while(flag != 1)
	{
		mes_prov = msgrcv(id_cs, (void *)&inp_text, sizeof(inp_text), 1L, 0);
		if(mes_prov != -1)
		{
			printf("The message is received.\n");
			printf("%s:  %s\n", inp_text.nick, inp_text.message);
		}
		else
		{
			printf("Error received message");
		}
		strcpy(out_text.nick, inp_text.nick);
		strcpy(out_text.message, inp_text.message);
		out_text.type = 1L;
		for(i = 0; i < polzov; i++)
		{
			mes_prov = msgsnd(id_sc, (void *)&out_text, sizeof(out_text), 0);
			if(mes_prov != -1)
			{
				printf("Message sent.\n");
				printf("%s:  %s\n", inp_text.nick, inp_text.message);
			}
			else
			{
				printf("Error sent message");
			}
		}
	}
	pthread_join(tid, &status);
	msgctl(id_sc, IPC_RMID, 0);
	msgctl(id_cs, IPC_RMID, 0);
	msgctl(id_tech_sc, IPC_RMID, 0);
	msgctl(id_tech_cs, IPC_RMID, 0);
	exit(0);
}
