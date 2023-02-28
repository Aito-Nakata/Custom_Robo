/*****************************************************************
ファイル名	: server_command.c
機能		: サーバーのコマンド処理
*****************************************************************/

#include"server_common.h"
#include"server_func.h"

static void SetIntData2DataBlock(void *data,int intData,int *dataSize);
static void SetCharData2DataBlock(void *data,char charData,int *dataSize);
static int GetRandomInt(int n);

/*****************************************************************
関数名	: ExecuteCommand
機能	: クライアントから送られてきたコマンドを元に，
		  引き数を受信し，実行する
引数	: char	command		: コマンド
		  int	pos			: コマンドを送ったクライアント番号
出力	: プログラム終了コマンドが送られてきた時には0を返す．
		  それ以外は1を返す
*****************************************************************/
int ExecuteCommand(char command,int pos)
{
    unsigned char	data[MAX_DATA];
    int			dataSize,intData,intKey,intZahyo,intBullet_num;//データのサイズ,クライアント番号,方向,座標,弾の番号
	int 		movedzahyo = 0;
    int			endFlag = 1;
	int robox = 0;

    /* 引き数チェック */
    assert(0<=pos && pos<MAX_CLIENTS);

#ifndef NDEBUG
    printf("#####\n");
    printf("ExecuteCommand()\n");
    printf("Get command %c\n",command);
#endif
		if(henkei_flag0 == 1){
			henkei_count0 -= 1;
			if(henkei_count0 < 0){
				henkei_flag0 = 0;
				SendDashkaizyoCommand(0);
			}
		}
		if(henkei_flag1 == 1){
			henkei_count1 -=1;
			if(henkei_count1 < 0){
				henkei_flag1 = 0;
				SendDashkaizyoCommand(1);
			}
		}
    switch(command){
	    case END_COMMAND:
			dataSize = 0;
			/* コマンドのセット */
			SetCharData2DataBlock(data,command,&dataSize);

			/* 全ユーザーに送る */
			SendData(ALL_CLIENTS,data,dataSize);

			endFlag = 0;
			break;
		case TITLE_COMMAND:
			RecvIntData(pos,&intData);
			dataSize = 0;
			SendTitleCommand(pos);
			break;
		case MOVE_COMMAND://移動処理
			RecvIntData(pos,&intData);
			RecvIntData(pos,&intKey);
			RecvIntData(pos,&intZahyo);
			movedzahyo = intZahyo;
		/*if(intKey == 1 ||intKey == 4){//右か下,つまり正の方向
			movedzahyo += MOVE_KYORI;
		}
		if(intKey == 2 || intKey == 3){//左か上,つまり負の方向
			movedzahyo -= MOVE_KYORI;
		}
		*/
			dataSize = 0;
			/* コマンドのセット */
			SetCharData2DataBlock(data,command,&dataSize);
			SetIntData2DataBlock(data,intData,&dataSize);
			SetIntData2DataBlock(data,intKey,&dataSize);
			SetIntData2DataBlock(data,movedzahyo,&dataSize);
			SendData(ALL_CLIENTS,data,dataSize);
			break;
		case DASH_COMMAND:
			RecvIntData(pos,&intData);
			if(intData == 0){
				henkei_flag0 = 1;
				henkei_count0 = 10;
			}
			if(intData == 1){
				henkei_flag1 = 1;
				henkei_count1 = 10;
			}

			dataSize = 0;
			/* コマンドのセット */
			SetCharData2DataBlock(data,command,&dataSize);
			SetIntData2DataBlock(data,intData,&dataSize);
			SendData(ALL_CLIENTS,data,dataSize);
			break;
		case BOX_KAIZYO_COMMAND:
			RecvIntData(pos,&intData);
			dataSize = 0;
			SendBoxkaizyoCommand(pos);
			break;
		case BULLET_COMMAND://弾が打たれた
			RecvIntData(pos,&intData);
			RecvIntData(pos,&intBullet_num);
			dataSize = 0;
			/* コマンドのセット */
			SetCharData2DataBlock(data,command,&dataSize);
			SetIntData2DataBlock(data,intData,&dataSize);
			SetIntData2DataBlock(data,intBullet_num,&dataSize);
			SendData(ALL_CLIENTS,data,dataSize);
			break;
		case BAKUDAN_COMMAND:
			RecvIntData(pos,&intData);
			RecvIntData(pos,&intBullet_num);
			dataSize = 0;
			/* コマンドのセット */
			SetCharData2DataBlock(data,command,&dataSize);
			SetIntData2DataBlock(data,intData,&dataSize);
			SetIntData2DataBlock(data,intBullet_num,&dataSize);
			SendData(ALL_CLIENTS,data,dataSize);
			break;
		case JUDTH_COMMAND://
			int flag;
			RecvIntData(pos,&flag);
			dataSize = 0;
			/* コマンドのセット */
			//p1の勝ち
			if(flag == 0){
				//p0に敗北状態を伝える
				SetCharData2DataBlock(data,LOOSE_COMMAND,&dataSize);
				SetIntData2DataBlock(data,0,&dataSize);
				SendData(0,data,dataSize);
				dataSize = 0;
				//p1に勝利状態を伝える
				SetCharData2DataBlock(data,WIN_COMMAND,&dataSize);
				SetIntData2DataBlock(data,0,&dataSize);
				SendData(1,data,dataSize);
			}
			//p0の勝ち
			if(flag == 1){
				//p0に勝利状態を伝える
				SetCharData2DataBlock(data,WIN_COMMAND,&dataSize);
				SetIntData2DataBlock(data,1,&dataSize);
				SendData(0,data,dataSize);
				dataSize = 0;
				//p1に敗北状態を伝える
				SetCharData2DataBlock(data,LOOSE_COMMAND,&dataSize);
				SetIntData2DataBlock(data,1,&dataSize);
				SendData(1,data,dataSize);
			}

			break;
	    default:
			/* 未知のコマンドが送られてきた */
			fprintf(stderr,"0x%02x is not command!\n",command);
    }
    return endFlag;
}

/*****************************************************************
関数名	: SendDashkaizyoCommand
機能	: クライアントに菱形を表示させるためにデータを送る
引数	: なし
出力	: なし
*****************************************************************/
void SendDashkaizyoCommand(int clientID)
{
    unsigned char data[MAX_DATA];
    int           dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendDashkaizyoCommand\n");
#endif
    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data,DASH_KAIZYO_COMMAND,&dataSize);
	SetIntData2DataBlock(data,clientID,&dataSize);

    /* クライアントに送る */
    SendData(ALL_CLIENTS,data,dataSize);
}
/*****************************************************************
関数名	: SendBoxkaizyoCommand
機能	: クライアントに菱形を表示させるためにデータを送る
引数	: なし
出力	: なし
*****************************************************************/
void SendBoxkaizyoCommand(int clientID)
{
    unsigned char data[MAX_DATA];
    int           dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendBoxkaizyoCommand\n");
#endif
    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data,BOX_KAIZYO_COMMAND,&dataSize);
	SetIntData2DataBlock(data,clientID,&dataSize);

    /* クライアントに送る */
    SendData(ALL_CLIENTS,data,dataSize);
}

/*****************************************************************
関数名	: SendTitleCommand
機能	: クライアントに待機画面を表示させるためにデータを送る
引数	: なし
出力	: なし
*****************************************************************/
void SendTitleCommand(int clientID)
{
    unsigned char data[MAX_DATA];
    int           dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendTitleCommand\n");
#endif
    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data,TITLE_COMMAND,&dataSize);
	SetIntData2DataBlock(data,clientID,&dataSize);

    /* クライアントに送る */
    SendData(ALL_CLIENTS,data,dataSize);
}

/*****
static
*****/
/*****************************************************************
関数名	: SetIntData2DataBlock
機能	: int 型のデータを送信用データの最後にセットする
引数	: void		*data		: 送信用データ
		  int		intData		: セットするデータ
		  int		*dataSize	: 送信用データの現在のサイズ
出力	: なし
*****************************************************************/
static void SetIntData2DataBlock(void *data,int intData,int *dataSize)
{
    int tmp;

    /* 引き数チェック */
    assert(data!=NULL);
    assert(0<=(*dataSize));

    tmp = htonl(intData);

    /* int 型のデータを送信用データの最後にコピーする */
    memcpy(data + (*dataSize),&tmp,sizeof(int));
    /* データサイズを増やす */
    (*dataSize) += sizeof(int);
}

/*****************************************************************
関数名	: SetCharData2DataBlock
機能	: char 型のデータを送信用データの最後にセットする
引数	: void		*data		: 送信用データ
		  int		intData		: セットするデータ
		  int		*dataSize	: 送信用データの現在のサイズ
出力	: なし
*****************************************************************/
static void SetCharData2DataBlock(void *data,char charData,int *dataSize)
{
    /* 引き数チェック */
    assert(data!=NULL);
    assert(0<=(*dataSize));

    /* int 型のデータを送信用データの最後にコピーする */
    *(char *)(data + (*dataSize)) = charData;
    /* データサイズを増やす */
    (*dataSize) += sizeof(char);
}

/*****************************************************************
関数名	: GetRandomInt
機能	: 整数の乱数を得る
引数	: int		n	: 乱数の最大値
出力	: 乱数値
*****************************************************************/
static int GetRandomInt(int n)
{
    return rand()%n;
}

