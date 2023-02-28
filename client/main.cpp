/*****************************************************************
ファイル名	: client_main.c
機能		: クライアントのメインルーチン
*****************************************************************/

#include"../constants.h"
#include"system.hpp"

int main(int argc,char *argv[])
{
    int		num;
    char	name[MAX_CLIENTS][MAX_NAME_SIZE];
    int		endFlag=1;
    int     box_flag = 1;
	int     box_count = 0;
    char	localHostName[]="localhost";
    char	*serverName;
    int		clientID;
    SDL_Event event;

    /* 引き数チェック */
    if(argc == 1){
    	serverName = localHostName;
    }
    else if(argc == 2){
    	serverName = argv[1];
    }

    else{
		fprintf(stderr, "Usage: %s, Cannot find a Server Name.\n", argv[0]);
		return -1;
    }
    /* サーバーとの接続 */
    if(SetUpClient(serverName,&clientID,&num,name)==-1){
		fprintf(stderr,"setup failed : SetUpClient\n");
		return -1;
	}
    /* システムの初期化 */
    if(InitSystem("map.data") < 0){
      PrintError("failed to initialize System");
      return -1;
    }
    /* ウインドウの初期化 */
	if(InitWindows(clientID,num,name)==-1){
		fprintf(stderr,"setup failed : InitWindows\n");
		return -1;
	}
    Playmusic();

    while(endFlag && Game_flag == 1 && box_flag == 1){
            if(SDL_PollEvent(&event)){

                if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE){
                    SendEndCommand();
                    }

                else if (box_count <= 9 && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_c){
                    box_count++;
                }

                else if(box_count >= 10){
                    box_flag = 0;
                    SendBoxKaizyoData(clientID);
                    //printf("debug\n");
                }
            }   
        ResetDraw();
        presenttime();
        cooltime();
        //移動処理
        MoveRobo();
        endFlag = SendRecvManager();
    }
    /* メインイベントループ */
    while(endFlag && Game_flag == 1 && box_flag == 0){
       //サーバとのデータのやり取り
		WindowEvent(num,clientID);
        presenttime();
        cooltime();
        //移動処理
        MoveRobo();
        ResetDraw();
		endFlag = SendRecvManager();

    };

    /* 終了処理 */
	DestroyWindow();
	CloseSoc();

    return 0;
}
/* エラーメッセージ表示
 *
 * 引数
 *   str: エラーメッセージ
 *
 * 返値: -1
 */
int PrintError(const char* str)
{
    fprintf(stderr, "%s\n", str);
    return -1;
}

static Uint32 SignalHandler(Uint32 interval, void *param)
{
	ResetDraw();

	return interval;
}