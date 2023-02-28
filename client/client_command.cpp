/*****************************************************************
�ե�����̾	: client_command.c
��ǽ		: ���饤����ȤΥ��ޥ�ɽ���
*****************************************************************/

#include"../constants.h"
#include"system.hpp"

static void SetIntData2DataBlock(void *data,int intData,int *dataSize);
static void SetCharData2DataBlock(void *data,char charData,int *dataSize);
static void RecvCircleData(void);
static void RecvRectangleData(void);
static void RecvDiamondData(void);
static void RecvlooseData(void);
static void RecvwinData(void);


/*****************************************************************
�ؿ�̾	: ExecuteCommand
��ǽ	: �����С����������Ƥ������ޥ�ɤ򸵤ˡ�
		  ���������������¹Ԥ���
����	: char	command		: ���ޥ��
����	: �ץ���ཪλ���ޥ�ɤ��������Ƥ������ˤ�0���֤���
		  ����ʳ���1���֤�
*****************************************************************/
int ExecuteCommand(char command)
{
    int	endFlag = 1;
    int flag_robo0_atari = 0;//p1のロボが相手とぶつかっているかの判定フラグ
	int flag_robo1_atari = 0;
//#ifndef NDEBUG
    printf("#####\n");
    printf("ExecuteCommand()\n");
    printf("command = %c\n",command);
//#endif
    switch(command){
		case END_COMMAND:
			endFlag = 0;
			break;
        case TITLE_COMMAND:
        int client_title;
            printf("受け取った\n");
            RecvIntData(&client_title);
            SetRoboTitleData(client_title);
            break;
        case MOVE_COMMAND:
        int moved;//移動後の座標
        int key,client;
        	RecvIntData(&client);
			RecvIntData(&key);
            RecvIntData(&moved);
            SetMoveRobo(client,key,moved);
            break;
        case DASH_COMMAND:
        int client_d;
            RecvIntData(&client_d);
            SetRoboDashData(client_d);
            break;
        case BOX_KAIZYO_COMMAND:
        int client_b_kaizyo;
            RecvIntData(&client_b_kaizyo);
            SetRoboBox_KaizyoData(client_b_kaizyo);
            break;
        case DASH_KAIZYO_COMMAND:
        int client_d_kaizyo;
            RecvIntData(&client_d_kaizyo);
            SetRoboDash_KaizyoData(client_d_kaizyo);
            break;
        case BULLET_COMMAND:
            int client_num,bullet_num;
            RecvIntData(&client_num);
            RecvIntData(&bullet_num);
            SetBulletData(client_num,bullet_num);
            break;
        case BAKUDAN_COMMAND:
            int client_bnum,bakudan_num;
            RecvIntData(&client_bnum);
            RecvIntData(&bakudan_num);
            SetBakudanData(client_bnum,bakudan_num);
            break;
        case WIN_COMMAND://win
            int wflag;
            RecvIntData(&wflag);
            match_winflag(wflag);
			RecvwinData();
            SDL_Delay(5000);
            endFlag = 0;
			break;
		case LOOSE_COMMAND://loose
            int lflag;
            RecvIntData(&lflag);
            match_winflag(lflag);
			RecvlooseData();
            SDL_Delay(5000);
            endFlag = 0;
			break;
    }
    return endFlag;
}


/*****************************************************************
�ؿ�̾	: SendEndCommand
��ǽ	: �ץ����ν�λ���Τ餻�뤿��ˡ�
		  �����С��˥ǡ���������
����	: �ʤ�
����	: �ʤ�
*****************************************************************/
void SendEndCommand(void)
{
    unsigned char	data[MAX_DATA];
    int			dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendEndCommand()\n");
#endif
    dataSize = 0;
    /* ���ޥ�ɤΥ��å� */
    SetCharData2DataBlock(data,END_COMMAND,&dataSize);

    /* �ǡ��������� */
    SendData(data,dataSize);
}

/*****
static
*****/
/*****************************************************************
�ؿ�̾	: SetIntData2DataBlock
��ǽ	: int ���Υǡ����������ѥǡ����κǸ�˥��åȤ���
����	: void		*data		: �����ѥǡ���
		  int		intData		: ���åȤ���ǡ���
		  int		*dataSize	: �����ѥǡ����θ��ߤΥ�����
����	: �ʤ�
*****************************************************************/
static void SetIntData2DataBlock(void *data,int intData,int *dataSize)
{
    int tmp;

    /* ����������å� */
    assert(data!=NULL);
    assert(0<=(*dataSize));

    tmp = htonl(intData);

    /* int ���Υǡ����������ѥǡ����κǸ�˥��ԡ����� */
    memcpy(data + (*dataSize),&tmp,sizeof(int));
    /* �ǡ��������������䤹 */
    (*dataSize) += sizeof(int);
}

/*****************************************************************
�ؿ�̾	: SetCharData2DataBlock
��ǽ	: char ���Υǡ����������ѥǡ����κǸ�˥��åȤ���
����	: void		*data		: �����ѥǡ���
		  int		intData		: ���åȤ���ǡ���
		  int		*dataSize	: �����ѥǡ����θ��ߤΥ�����
����	: �ʤ�
*****************************************************************/
static void SetCharData2DataBlock(void *data,char charData,int *dataSize)
{
    /* ����������å� */
    assert(data!=NULL);
    assert(0<=(*dataSize));

    /* char ���Υǡ����������ѥǡ����κǸ�˥��ԡ����� */
    *(char *)(data + (*dataSize)) = charData;
    /* �ǡ��������������䤹 */
    (*dataSize) += sizeof(char);
}


//追加
/*****************************************************************
関数名	: SendRobomoveData
機能	: クライアントにクライアントposが,commandを出したと,
		  サーバーにデータを送る
引数	: int		pos	    : クライアント番号
      : int       key : 方向
      : int       zahyo 現在座標
出力	: なし
*****************************************************************/
void SendRobomoveData(int pos,int key,int zahyo)//posはクライアントの番号//keyは方向
{
    unsigned char	data[MAX_DATA];
    int			dataSize;
    assert(0<=pos && pos<MAX_CLIENTS);

#ifndef NDEBUG
    printf("#####\n");
    printf("robomoveCommand()\n");
    printf("Send robomoveCommand to %d\n",pos);
#endif
    dataSize = 0;
    SetCharData2DataBlock(data,MOVE_COMMAND,&dataSize);
    SetIntData2DataBlock(data,pos,&dataSize);
    SetIntData2DataBlock(data,key,&dataSize);
    SetIntData2DataBlock(data,zahyo,&dataSize);
    
    //データを送信
    SendData(data,dataSize);
}
//ダッシュ状態を送る関数
void SendDashData(int pos){
    unsigned char	data[MAX_DATA];
    int			dataSize;
    dataSize = 0;
    SetCharData2DataBlock(data,DASH_COMMAND,&dataSize);
    SetIntData2DataBlock(data,pos,&dataSize);
    //データを送信
    SendData(data,dataSize);
}
//ボックス状態解除コマンドを送る関数
void SendBoxKaizyoData(int pos){
    unsigned char	data[MAX_DATA];
    int			dataSize;
    dataSize = 0;
    SetCharData2DataBlock(data,BOX_KAIZYO_COMMAND,&dataSize);
    SetIntData2DataBlock(data,pos,&dataSize);
    //データを送信
    SendData(data,dataSize);
}

//タイトル状態を送る関数
void SendTitleData(int pos){
    unsigned char	data[MAX_DATA];
    int			dataSize;
    dataSize = 0;
    SetCharData2DataBlock(data,TITLE_COMMAND,&dataSize);
    SetIntData2DataBlock(data,pos,&dataSize);
    //データを送信
    SendData(data,dataSize);
}




//弾のデータを送る関数    
void SendBulletData(int pos,int bullet_num)
{
    unsigned char	data[MAX_DATA];
    int			dataSize;
    assert(0<=pos && pos<MAX_CLIENTS);

#ifndef NDEBUG
    printf("#####\n");
    printf("BulletCommand()\n");
    printf("Send Bullet Command to %d\n",pos);
#endif 
    dataSize = 0;
    SetCharData2DataBlock(data,BULLET_COMMAND,&dataSize);
    SetIntData2DataBlock(data,pos,&dataSize);
    SetIntData2DataBlock(data,bullet_num,&dataSize);
    //データを送信
    SendData(data,dataSize);
}

//サーバに勝敗を表示させるためにデータを送る   
void SendJudthData(int winflag)
{
    unsigned char	data[MAX_DATA];
    int			dataSize;
#ifndef NDEBUG
    printf("#####\n");
    printf("JudtgCommand()\n");
#endif 
    dataSize = 0;
    SetCharData2DataBlock(data,JUDTH_COMMAND,&dataSize);
     SetIntData2DataBlock(data,winflag,&dataSize);
    //データを送信
    SendData(data,dataSize);
}

//爆弾のデータを送る関数    
void SendBakudanData(int pos,int bakudan_num)
{
    unsigned char	data[MAX_DATA];
    int			dataSize;
    assert(0<=pos && pos<MAX_CLIENTS);

#ifndef NDEBUG
    printf("#####\n");
    printf("BakudanCommand()\n");
    printf("Send Bakudan Command to %d\n",pos);
#endif 
    dataSize = 0;
    SetCharData2DataBlock(data,BAKUDAN_COMMAND,&dataSize);
    SetIntData2DataBlock(data,pos,&dataSize);
    SetIntData2DataBlock(data,bakudan_num,&dataSize);
    //データを送信
    SendData(data,dataSize);
}



/*****************************************************************
関数名	: RecvlooseData
機能	: 敗北と表示する
引数	: なし
出力	: なし
*****************************************************************/
static void RecvlooseData(void)
{
    printf("lose\n");
    DrawLoose();

}

/*****************************************************************
関数名	: RecvwinData
機能	: 勝利と表示する
引数	: なし
出力	: なし
*****************************************************************/
static void RecvwinData(void)
{
    printf("Win\n"); 
    DrawWin();

}