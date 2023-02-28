/*****************************************************************
ファイル名	: constants.h
機能		: サーバーとクライアントで使用する定数の宣言を行う
*****************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_
const int IMAGE_WIDTH = 16;
const int IMAGE_HEIGHT = 16;

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<assert.h>
#include<math.h>
#include<unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>



#define PORT			(u_short)8888	/* ポート番号 */

#define MAX_CLIENTS		4				/* クライアント数の最大値 */
#define MAX_NAME_SIZE	10 				/* ユーザー名の最大値*/

#define MAX_DATA		200				/* 送受信するデータの最大値 */

#define WIN_X 864 //ウィンドウのx
#define WIN_Y 864 //ウィンドウのy
#define MOVE_KYORI 2 //移動距離
#define PSHOT_NUM 8 //弾数
#define PSHOT_SPEED 4 //弾の速さ
#define PBAKUDAN_NUM 1 //爆弾の数
#define PBAKUDAN_SPEED 1.5 //爆弾の速さ

#define END_COMMAND		'E'		  		/* プログラム終了コマンド */
#define TITLE_COMMAND 'T' 
#define CIRCLE_COMMAND	'C'				/* 円表示コマンド */
#define DASH_KAIZYO_COMMAND 'K' 
#define BOX_KAIZYO_COMMAND 'H' /*ボックス解除コマンド（変形のH）*/ 
#define DASH_COMMAND	'D'	/*ダッシュコマンド */
#define BULLET_COMMAND 'B' //弾関連
#define MOVE_COMMAND 'M' //移動コマンド
#define JUDTH_COMMAND 'J' //勝敗判定コマンド
#define WIN_COMMAND	    'W'	//勝利コマンド		
#define LOOSE_COMMAND	'L'//負けコマンド
#define BAKUDAN_COMMAND 'Q' //爆弾コマンド


#endif
