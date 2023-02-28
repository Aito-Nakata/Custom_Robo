/*****************************************************************
�ե�����̾	: server_func.h
��ǽ		: �����С��γ���ؿ�����
*****************************************************************/

#ifndef _SERVER_FUNC_H_
#define _SERVER_FUNC_H_

#include"server_common.h"

/* server_net.c */
extern int SetUpServer(int num);
extern void Ending(void);
extern int RecvIntData(int pos,int *intData);
extern void SendData(int pos,void *data,int dataSize);
extern int SendRecvManager(void);

/* server_command.c */
extern int ExecuteCommand(char command,int pos);
extern void SendDiamondCommand(void);
void SendDashkaizyoCommand(int clientID);
void SendBoxkaizyoCommand(int clientID);
void SendTitleCommand(int clientID);


static int henkei_count0;
static int henkei_count1;
static int henkei_flag0;
static int henkei_flag1;

#endif
