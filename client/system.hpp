/*****************************************************************
ファイル名	: client_func.h
機能		: クライアントの外部関数の定義
*****************************************************************/

#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_

#include"../constants.h"
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h> // TrueTypeフォントを表示するために必要なヘッダファイルをインクルード
#include<SDL2/SDL_mixer.h>

/* client_net.c */
extern int SetUpClient(char* hostName,int *clientID,int *num,char clientName[][MAX_NAME_SIZE]);
extern void CloseSoc(void);
extern int RecvIntData(int *intData);
extern void SendData(void *data,int dataSize);
extern int SendRecvManager(void);

/* client_win.c */
extern int InitWindows(int clientID,int num,char name[][MAX_NAME_SIZE]);
extern void DestroyWindow(void);
extern int InitSystem(const char *map_data_file);
extern void WindowEvent(int num,int clientID);
void ResetDraw(void);
int robo_calc_atari(int roboex_x,int roboex_y,int roboaf_x,int roboaf_y);
extern void Draw_Bullet(void);
void SetBakudanData(int pos,int bullet_num);
void SetBulletData(int pos,int bullet_num);
void SetRoboMoveData(int key,int clientID,int movedzahyo);
extern int Robo_calc();
int Robo_calc_x_right(int clientID);
int Robo_calc_x_left(int clientID);
int Robo_calc_y_up(int clientID);
int Robo_calc_y_down(int clientID);
void match_winflag(int flag);

extern int which_win(void);
void DrawWin(void);
void DrawLoose(void);
void Draw_Bakudan(void);
void Deleat_tama(void);
void SetMoveRobo(int clientID,int key,int moved);
void MoveRobo(void);
void SetRoboDashData(int clientID);
void SetRoboTitleData(int clientID);
extern int GUI_flag; //タイトル画面の操作から,ゲーム画面の操作へ変更するためのflag
extern int Game_flag; //メインイベントループに入るためのflag(0:タイトル画面,1:ゲーム画面)
extern int box_flag;
extern int box_count;
void cooltime(void);


/* client_command.c */
extern int ExecuteCommand(char command);
extern void SendRectangleCommand(void);
extern void SendCircleCommand(int pos);
extern void SendEndCommand(void);
extern void SendRobomoveData(int pos,int key,int zahyo);
extern void SendBulletData(int pos,int bullet_num);
extern void SendJudthData(int winflag);
void SendBakudanData(int pos,int bakudan_num);
void SendDashData(int pos);
void SendBoxKaizyoData(int pos);
void SendTitleData(int pos);
void SetRoboDash_KaizyoData(int clientID);
void SetRoboBox_KaizyoData(int clientID);

/*window.cpp*/
extern int PrintError(const char* str);
extern void presenttime(void);

extern SDL_bool AdjustPoint0(SDL_Rect overlap);
extern SDL_bool AdjustOverlapBlock0(void);
extern SDL_bool AdjustPoint1(SDL_Rect overlap);
extern SDL_bool AdjustOverlapBlock1(void);
extern SDL_bool OverlapBlock_bullet0(void);
extern SDL_bool OverlapBlock_bullet1(void);
extern SDL_bool OverlapBlock_bom0(void);
extern SDL_bool OverlapBlock_bom1(void);

/*マップサイズ（チップ数）*/
enum{
    MAP_Width    = 18,//map.dataの縦の数
    MAP_Height   = 18,//横の数
    MAP_ChipSize = 48 /*1チップの大きさ*/
};

/*マップの種類*/
typedef enum {
    MT_Mag    = 0, /*溶岩*/
    MT_Block  = 1, /*壁*/
    MT_Desert = 2, /*砂漠*/
    MT_Ground = 3 /*地面*/
} MapType;

/*マップ情報*/
typedef struct{
    MapType map[MAP_Width][MAP_Height]; /*配置*/
    SDL_Rect *mags;                     /*溶岩の矩形一覧*/
    SDL_Rect *blocks;                   /*壁の矩形一覧*/
    int mnum;                           /*溶岩の矩形数*/
    int bnum;                           /*壁の矩形数*/
    char path[128];
}MapInfo;

//ロボについて
typedef struct{
    double x;
    double y;
    int width;
    int height;
    int hp;
    int speed;
    int type;//状態,ダッシュは1
    int flag_title;
    int Dcount; //ダッシュ状態のカウント
    int Rflag;//右押している
    int Lflag;//左押している
    int Uflag;//上押している
    int Dflag;//下押している
    int title = 0;//タイトル画面(0:タイトル画面,1:ゲーム画面)
    SDL_Surface *image;
    SDL_Surface *image_dash;
    SDL_Surface *image_box;
    SDL_Rect src;
    SDL_Rect dst;
    SDL_Texture *texture;
} RoboInfo;

/*HPバー*/
typedef struct{
    double x;
    double y;
    int width;
    int height;
    SDL_Surface *image;
    SDL_Rect src;
    SDL_Rect dst;
    SDL_Texture *texture;
} HPbarInfo;

/*プレイヤー表示*/
typedef struct{
    double x;
    double y;
    int width;
    int height;
    SDL_Surface *image;
    SDL_Rect src;
    SDL_Rect dst;
    SDL_Texture *texture;
} PlayerInfo;

typedef struct{
    double x;
    double y;
    int width;
    int height;
    SDL_Surface *image;
    SDL_Rect src;
    SDL_Rect dst;
    SDL_Texture *texture;
} HPbar_frameInfo;

/*玉*/
struct SHOT{
    int flag;//弾が打たれているかの判断
    double x;
    double y;
	int vec;//弾の方向 //0右１左
	double rad;//角度
    SDL_Surface *bullet_gh;
    SDL_Texture *texture_bullet;
    int width,height;
	SDL_Rect src_rect_bullet;
	SDL_Rect dst_rect_bullet;
	SDL_Rect prev_rect_bullet;
    SDL_Rect mapbullet;//マップと弾の当たり判定に使用する弾の領域
};

struct BAKUDAN{
    int flag;//弾が打たれているかの判断
    double x;
    double y;
	int vec;//弾の方向 //0右１左
	double rad;//角度
    SDL_Surface *bakudan_gh;
    SDL_Texture *texture_bakudan;

    SDL_Surface *explosion;
    SDL_Texture *texture_ex;
    SDL_Rect src_rect_ex;
	SDL_Rect dst_rect_ex;
    int excount;

    int width,height;
	SDL_Rect src_rect_bakudan;
	SDL_Rect dst_rect_bakudan;
	SDL_Rect prev_rect_bakudan;
    SDL_Rect mapbom;
};

/*
extern Mix_Music *music;
extern Mix_Chunk *chunk;
*/
void Playmusic(void);

extern MapInfo gMap;
extern RoboInfo Robo[MAX_CLIENTS];

#endif