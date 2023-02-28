/*****************************************************************
ファイル名	: client_win.c
機能		: クライアントのユーザーインターフェース処理
*****************************************************************/

#include"../constants.h"
#include"system.hpp"

static SDL_Window *gMainWindow;
static SDL_Renderer *gMainRenderer;
int winflag;//どちらのhpが0になったか判断
int game_endflag;
SDL_Rect Map_rect = { WIN_X/2, WIN_Y/2 , WIN_X,WIN_Y};//描画領域
SDL_Texture *tmap;
int MakeMap(void);
int RenderWindow(void);
MapInfo gMap;
void Chomp(char *str);
//弾とロボの距離を返す関数
int calc_distance(double x1, double y1, double x2, double y2);

static int CheckButtonNO(int x,int y,int num);
void DrawRobo(int x,int y,int clientID);
void DrawInitRobo(void);
void Init_Robo(void);
int robo_calc_atari_right(int roboex_x,int roboex_y,int roboaf_x,int roboaf_y);
int robo_calc_atari_left(int roboex_x,int roboex_y,int roboaf_x,int roboaf_y);
int robo_calc_atari_up(int roboex_x,int roboex_y,int roboaf_x,int roboaf_y);
int robo_calc_atari_under(int roboex_x,int roboex_y,int roboaf_x,int roboaf_y);
void Init_bullet(void);
void Init_bakudan(void);
void Init_Title(int clientID);
void Init_HPbar(void);


SDL_bool AdjustPoint0(SDL_Rect overlap);
SDL_bool AdjustOverlapBlock0(void);
SDL_bool AdjustPoint1(SDL_Rect overlap);
SDL_bool AdjustOverlapBlock1(void);
SDL_bool OverlapBlock_bullet0(void);
SDL_bool OverlapBlock_bullet1(void);
SDL_bool OverlapBlock_bom0(void);
SDL_bool OverlapBlock_bom1(void);
/*ロボ情報の構造体*/
 RoboInfo Robo[MAX_CLIENTS];

 /*HPバー情報の構造体*/
 HPbarInfo HPbar[MAX_CLIENTS];
 HPbar_frameInfo HPbar_frame[MAX_CLIENTS];
/*プレイヤー情報の構造体*/
 PlayerInfo Player[MAX_CLIENTS];

//グラフィックハンドル格納用配列
int gh[12];
//弾
SHOT shot_p0[PSHOT_NUM];//p0が出した弾
SHOT shot_p1[PSHOT_NUM];//p1が出した弾
int count_pshot0 = 0;//球数
int count_pshot1 = 0;
//爆弾
BAKUDAN bakudan_p0[PBAKUDAN_NUM];
BAKUDAN bakudan_p1[PBAKUDAN_NUM];
int count_pbakudan0 = 0;//爆弾の数
int count_pbakudan1 = 0;


int GUI_flag = 1; //タイトル画面の操作から,ゲーム画面の操作へ変更するためのflag
int Game_flag = 0; //メインイベントループに入るためのflag(0:タイトル画面,1:ゲーム画面)
int Win_flag = 0;
int Loose_flag = 0;
Mix_Music *music;
Mix_Chunk *chunk;
Mix_Chunk *chunk_dm;
Mix_Chunk *chunk_bang;

/*****************************************************************
関数名	: InitWindows
機能	: メインウインドウの表示，設定を行う
引数	: int	clientID		: クライアント番号
		  int	num				: 全クライアント数
出力	: 正常に設定できたとき0，失敗したとき-1
*****************************************************************/
int InitWindows(int clientID,int num,char name[][MAX_NAME_SIZE])
{
	int i;
	SDL_Texture *texture;
	SDL_Surface *image;
	SDL_Rect src_rect;
	SDL_Rect dest_rect;
	char *s,title[10];

    /* 引き数チェック */
    assert(0<num && num<=MAX_CLIENTS);

	/* SDLの初期化 */
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("failed to initialize SDL.\n");
		return -1;
	}
	 // SDL_mixerの初期化（MP3ファイルを使用）
    Mix_Init(MIX_INIT_MP3);

    // オーディオデバイスの初期化
    if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        printf("failed to initialize SDL_mixer.\n");
        SDL_Quit();
        exit(-1);
    }

    // BGMと効果音のサウンドファイルの読み込み
    if((music = Mix_LoadMUS("ENERGY-INFLATION.mp3")) == NULL || (chunk = Mix_LoadWAV("shot01.wav")) == NULL || (chunk_bang = Mix_LoadWAV("explosion.wav")) == NULL || (chunk_dm = Mix_LoadWAV("damage.wav")) == NULL) {
        printf("failed to load music and chunk.\n");
        Mix_CloseAudio(); // オーディオデバイスの終了
        SDL_Quit();
        exit(-1);
    }


	/* メインのウインドウを作成する */
	if((gMainWindow = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_X, WIN_Y, 0)) == NULL) {
		printf("failed to initialize videomode.\n");
		return -1;
	}

	gMainRenderer = SDL_CreateRenderer(gMainWindow, -1, SDL_RENDERER_SOFTWARE);//SDL_RENDERER_ACCELERATED |SDL_RENDERER_PRESENTVSYNC);//0);

	/* ウインドウのタイトルをセット */
	sprintf(title,"%d",clientID);
	SDL_SetWindowTitle(gMainWindow, title);
	Init_Title(clientID);

	return 0;
}
//マップ関連
int InitSystem(const char *map_data_file)
{
	int ret = 0;
    int x = 0, y = 0;
	/*マップ情報を読み込む*/
	/*ファイルオープン*/
	FILE *fp = fopen(map_data_file,"r");
	if(fp == NULL){
		return PrintError("failed to open map data file.");
	}
	/*画像ファイルパス*/
	if(NULL == fgets(gMap.path,128,fp)){
		ret = PrintError("failed to read the map image path.");
		 goto CLOSEFILE;
	}
	Chomp(gMap.path);
	/*マップデータ*/

	while(y < MAP_Height){
		if(1 != fscanf(fp, "%u", &(gMap.map[x][y]))){
			ret = PrintError("failed to load map data.");
			goto CLOSEFILE;
		}
		/*溶岩,壁を数える */
		if(gMap.map[x][y] == MT_Mag)
			gMap.mnum++;
		if(gMap.map[x][y] == MT_Block)
			gMap.bnum++;
		/* 次 */
		if (++x >= MAP_Width){
			x = 0;
			y++;
		}
	}
	/* 領域の確保 */
	gMap.mags = (SDL_Rect *)malloc(sizeof(SDL_Rect) * gMap.mnum);
	if(gMap.mags == NULL){
		ret = PrintError("failed to allocate memory.");
		goto CLOSEFILE;
	}
	gMap.blocks = (SDL_Rect *)malloc(sizeof(SDL_Rect) * gMap.bnum);
	if(gMap.blocks == NULL){
		ret = PrintError("failed to allocate memory.");
		goto CLOSEFILE;
	}

CLOSEFILE:
	fclose(fp);
	return ret;
}


void Chomp(char *str)
{
	int i;
	for(i = 0;i < 128; i++)
	if(str[i] == '\0')
	    break;
	if(i > 0 && str[i-1] == '\n')
	    str[i - 1] = '\0';
}


/* ウインドウ描画
最初に使われている
メインウインドウに背景，キャラなどを転送する
*/

int RenderWindow(void)
{
	int ret = 0;
	SDL_Rect src = Map_rect;
    SDL_Point src_center;
    SDL_Rect dst ={0,0,WIN_X,WIN_Y};
	ret = SDL_RenderCopyEx(gMainRenderer, tmap, &src, &dst, 0, NULL, SDL_FLIP_NONE); //画像の向きの調整
    if (ret < 0) {
        PrintError(SDL_GetError());
    }
		//ロボについて
		Robo[0].dst = {Robo[0].x, Robo[0].y, 100,100}; // 画像のコピー先の座標と領域（x, y, w, h)
	   	Robo[1].dst = {Robo[1].x, Robo[1].y, 100,100}; // 画像のコピー先の座標と領域（x, y, w, h)
		SDL_RenderCopy(gMainRenderer, Robo[0].texture, &Robo[0].src,&Robo[0].dst);
		SDL_RenderCopy(gMainRenderer, Robo[1].texture, &Robo[1].src,&Robo[1].dst);

		//HPバーについて
		HPbar[0].dst = {HPbar[0].x, HPbar[0].y, 0,44}; // 画像のコピー先の座標と領域（x, y, w, h)
	   	HPbar[1].dst = {HPbar[1].x, HPbar[1].y, 394,44}; // 画像のコピー先の座標と領域（x, y, w, h)
		HPbar_frame[0].dst = {HPbar_frame[0].x, HPbar_frame[0].y, 400,50}; // 画像のコピー先の座標と領域（x, y, w, h)
	   	HPbar_frame[1].dst = {HPbar_frame[1].x, HPbar_frame[1].y, 400,50}; // 画像のコピー先の座標と領域（x, y, w, h)
		Player[0].dst = {Player[0].x, Player[0].y, 32,44}; // 画像のコピー先の座標と領域（x, y, w, h)
	   	Player[1].dst = {Player[1].x, Player[1].y, 32,44}; // 画像のコピー先の座標と領域（x, y, w, h)
		SDL_RenderCopy(gMainRenderer, HPbar[0].texture, &HPbar[0].src,&HPbar[0].dst);
		SDL_RenderCopy(gMainRenderer, HPbar[1].texture, &HPbar[1].src,&HPbar[1].dst);
		SDL_RenderCopy(gMainRenderer, HPbar_frame[0].texture, &HPbar_frame[0].src,&HPbar_frame[0].dst);
		SDL_RenderCopy(gMainRenderer, HPbar_frame[1].texture, &HPbar_frame[1].src,&HPbar_frame[1].dst);
		SDL_RenderCopy(gMainRenderer, Player[0].texture, &Player[0].src,&Player[0].dst);
		SDL_RenderCopy(gMainRenderer, Player[1].texture, &Player[1].src,&Player[1].dst);
		
		SDL_RenderPresent(gMainRenderer);

    return ret;
}

/* マップデータを基に背景作成
 *
 * 返値
 *   正常終了: 0
 *   エラー  : 負数
 */
int MakeMap(void)
{
	int ret = 0;
	 SDL_Surface *img = IMG_Load(gMap.path);
    if (img == NULL)
        return PrintError(IMG_GetError());

	SDL_Surface *map = SDL_CreateRGBSurface(0,MAP_Width * MAP_ChipSize + Map_rect.w, MAP_Height * MAP_ChipSize + Map_rect.h, 32, img->format->Rmask, img->format->Gmask, img->format->Bmask, img->format->Amask);
	if(map == NULL){
		ret = PrintError(SDL_GetError());
	}

	/* マップの配置*/
	  SDL_FillRect(map, NULL, SDL_MapRGB(map->format, 0, 0, 0));
	  if (map) {
        SDL_Rect src = { 0, 0, MAP_ChipSize, MAP_ChipSize };
        SDL_Rect dst = { 0 };
        int m = 0, b = 0;
        for (int y = 0; y < MAP_Height; y++) {
            for (int x = 0; x < MAP_Width; x++) {
                /* MapTypeの値順に画像が横方向に並んでいると想定 */
                src.x = gMap.map[x][y] * MAP_ChipSize;
                src.y = 0;
                /* 転送場所の設定 */
                dst.x = x * MAP_ChipSize + Map_rect.w/2; //転送場所を補正
                dst.y = y * MAP_ChipSize + Map_rect.h/2;
                if (0 > SDL_BlitSurface(img, &src, map, &dst)) {
                    ret = PrintError(SDL_GetError());
                }

				 /* 溶岩と壁の矩形を覚えておく（当たり判定のため） */
                switch (gMap.map[x][y]) {
                case MT_Mag:
                    gMap.mags[m++] = dst;
                    break;
                case MT_Block:
                    gMap.blocks[b++] = dst;
                default:
                    break;
                }
			}
		}
		/* マップはテクスチャに */
        tmap = SDL_CreateTextureFromSurface(gMainRenderer, map);

	  }

	/* サーフェイス解放(テクスチャに転送後はゲーム中に使わないので) */
    SDL_FreeSurface(map);
    SDL_FreeSurface(img);
	return ret;

}

/* 重なりを補正する
 *  対象キャラの座標をoverlapとmaskが重ならない位置まで補正する
 *
 * 引数
 *   overlap: 重なり部分
 *
 * 返値: 補正したらTRUE
 */

SDL_bool AdjustPoint0(SDL_Rect overlap)
{
    SDL_bool ret = SDL_FALSE;
    SDL_Point p0  = {Robo[0].x + WIN_X/2, Robo[0].y + WIN_Y/2 };//ロボ0の計算上の座標
    if (overlap.w && overlap.h) {
        ret = SDL_TRUE;
        if (overlap.w < overlap.h) {
            /* x方向の重なりの補正
                overlapはmask範囲の4辺のいずれかに接するので
                接する辺の逆方向に座標を補正する
             */
            if (overlap.x > p0.x) {
                Robo[0].x -= overlap.w;
            } else {
                Robo[0].x += overlap.w;
            }
        } else {
            /* y方向の重なりの補正 */
            if (overlap.y > p0.y) {
                Robo[0].y -= overlap.h;
            } else {
                Robo[0].y += overlap.h;
            }
		}
        }

    return ret;
}

/* 壁との重なりを補正する
 *  対象キャラが壁に接したときに座標を補正する
 *
 * 引数
 *   なし
 *
 * 返値: 補正したときSDL_TRUE
 */
SDL_bool AdjustOverlapBlock0(void)
{
    SDL_bool ret    = SDL_FALSE;
    SDL_Rect chrect0 = {Robo[0].x + WIN_X/2 ,Robo[0].y + WIN_Y/2 ,Robo[0].width,Robo[0].height};//ロボ0の計算上の領域

    /* 壁の矩形一覧とキャラ当たり矩形との重なりを調べ，
     * 重なっていれば補正する
     */
    for (int i = 0; i < gMap.bnum; i++) {
        SDL_Rect r;
        if (SDL_IntersectRect(&chrect0, &(gMap.blocks[i]), &r)) {
            if (AdjustPoint0(r)) {
                ret = SDL_TRUE;
            }
        }
    }
    return ret;
}
/*上記関数のロボ1版*/
SDL_bool AdjustPoint1(SDL_Rect overlap)
{
    SDL_bool ret = SDL_FALSE;
    SDL_Point p1  = {Robo[1].x + WIN_X/2, Robo[1].y + WIN_Y/2 };//ロボ1の計算上の座標
    if (overlap.w && overlap.h) {
        ret = SDL_TRUE;
        if (overlap.w < overlap.h) {
            /* x方向の重なりの補正
                overlapはmask範囲の4辺のいずれかに接するので
                接する辺の逆方向に座標を補正する
             */
            if (overlap.x > p1.x) {
                Robo[1].x -= overlap.w;
            } else {
                Robo[1].x += overlap.w;
            }
        } else {
            /* y方向の重なりの補正 */
            if (overlap.y > p1.y) {
                Robo[1].y -= overlap.h;
            } else {
                Robo[1].y += overlap.h;
            }
		}
        }

    return ret;
}

SDL_bool AdjustOverlapBlock1(void)
{
    SDL_bool ret    = SDL_FALSE;
    SDL_Rect chrect1 = {Robo[1].x + WIN_X/2 ,Robo[1].y + WIN_Y/2 ,Robo[1].width,Robo[1].height};//ロボ1の計算上の領域

    /* 壁の矩形一覧とキャラ当たり矩形との重なりを調べ，
     * 重なっていれば補正する
     */
    for (int i = 0; i < gMap.bnum; i++) {
        SDL_Rect r;
        if (SDL_IntersectRect(&chrect1, &(gMap.blocks[i]), &r)) {
            if (AdjustPoint1(r)) {
                ret = SDL_TRUE;
            }
        }
    }
    return ret;
}



/*****************************************************************
関数名	: DestroyWindow
機能	: SDLを終了する
引数	: なし
出力	: なし
*****************************************************************/
void DestroyWindow(void)
{
	SDL_Quit();
}

/*****************************************************************
関数名	: WindowEvent
機能	: メインウインドウに対するイベント処理を行う
引数	: int		num		: 全クライアント数
引数	: int		clientID		: クライアントID
出力	: なし
*****************************************************************/
void WindowEvent(int num,int clientID)
{
	SDL_Event event;
	SDL_MouseButtonEvent *mouse;
	int key = 0;
	int flag_robo0_atari = 0;
	int flag_robo1_atari = 0;
	if(game_endflag == 1){
    winflag = which_win();//0ならp1の勝ち,1ならp0の勝ち
    SendJudthData(winflag);
    }

    /* 引き数チェック */
    assert(0<num && num<=MAX_CLIENTS);
if(SDL_PollEvent(&event)){
		if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE){
				SendEndCommand();
				}
		else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT){
						key = 1;//右
						if(clientID == 0){
							SendRobomoveData(clientID,key,Robo[0].x);
						}
						if(clientID == 1){
							SendRobomoveData(clientID,key,Robo[1].x);
						}
		}
		else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT /*&& Robo[clientID].Lflag != 1*/){
						key = 2;//左
						if(clientID == 0){
							SendRobomoveData(clientID,key,Robo[0].x);
						}
						if(clientID == 1){
							SendRobomoveData(clientID,key,Robo[1].x);
						}
		}

		else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP /*&& Robo[clientID].Uflag != 1*/){
						key = 3;//上
						if(clientID == 0){
							SendRobomoveData(clientID,key,Robo[0].y);
						}
						if(clientID == 1){
							SendRobomoveData(clientID,key,Robo[1].y);
						}

		}

		else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN /*&& Robo[clientID].Dflag != 1*/){
						key = 4;//下
						if(clientID == 0){
							SendRobomoveData(clientID,key,Robo[0].y);
						}
						if(clientID == 1){
							SendRobomoveData(clientID,key,Robo[1].y);
						}
		}

		else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_z && Robo[clientID].type == 0){
							SendDashData(clientID);
							Robo[clientID].type = 3;
							//printf("debug\n");
		}

		else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && Robo[clientID].type != 1){
					printf("robo0hp=%d,robo1hp = %d",Robo[0].hp,Robo[1].hp);
					if(clientID == 0 && shot_p0[count_pshot0].flag != 1){
						//shot_p0[count_pshot0].flag = 1;
						SendBulletData(clientID,count_pshot0);
						count_pshot0++;
					}
					if(clientID == 1 && shot_p1[count_pshot1].flag != 1){
						//shot_p1[count_pshot1].flag = 1;
						SendBulletData(clientID,count_pshot1);
						count_pshot1++;
					}
					if(count_pshot0 > PSHOT_NUM-1){
						count_pshot0 = 0;
					}
					if(count_pshot1 > PSHOT_NUM-1){
						count_pshot1 = 0;
					}
		}

		else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_x && Robo[clientID].type != 1){
				if(clientID == 0 && bakudan_p0[count_pbakudan0].flag != 1){
					SendBakudanData(clientID,count_pbakudan0);
					count_pbakudan0++;
				}
				if(clientID == 1 && bakudan_p1[count_pbakudan1].flag != 1){
					SendBakudanData(clientID,count_pbakudan1);
					count_pbakudan0++;
				}
				if(count_pbakudan0 > PBAKUDAN_NUM-1){
					count_pbakudan0 = 0;
				}
				if(count_pbakudan1 > PBAKUDAN_NUM-1){
					count_pbakudan1 = 0;
				}
		}


		else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_RIGHT){
				key = 5;//右
						if(clientID == 0){
							SendRobomoveData(clientID,key,Robo[0].x);
						}
						if(clientID == 1){
							SendRobomoveData(clientID,key,Robo[1].x);
						}
		}
		else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LEFT){
						key = 6;//左
						if(clientID == 0){
							SendRobomoveData(clientID,key,Robo[0].x);
						}
						if(clientID == 1){
							SendRobomoveData(clientID,key,Robo[1].x);
						}


		}

		else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_UP){
						key = 7;//上
						if(clientID == 0){
							SendRobomoveData(clientID,key,Robo[0].y);
						}
						if(clientID == 1){
							SendRobomoveData(clientID,key,Robo[1].y);
						}

		}


		else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_DOWN){
						key = 8;//下
						if(clientID == 0){
							SendRobomoveData(clientID,key,Robo[0].y);
						}
						if(clientID == 1){
							SendRobomoveData(clientID,key,Robo[1].y);
						}
		}
	

}


}

//メインループ内で使用
//描画の更新
void ResetDraw(void){
		//マップの準備
		SDL_Rect src = Map_rect;
    	SDL_Point src_center;
    	SDL_Rect dst ={0,0,WIN_X,WIN_Y};
	    SDL_RenderCopyEx(gMainRenderer, tmap, &src, &dst, 0, NULL, SDL_FLIP_NONE); //画像の向きの調整
		AdjustOverlapBlock0();
		AdjustOverlapBlock1();
		//ロボについて
	   	 Robo[0].dst = {Robo[0].x, Robo[0].y,Robo[0].width,Robo[0].height}; // 画像のコピー先の座標と領域（x, y, w, h)
	   	 Robo[1].dst = {Robo[1].x, Robo[1].y,Robo[1].width,Robo[1].height}; // 画像のコピー先の座標と領域（x, y, w, h)
		if(Robo[0].x + Robo[0].width < Robo[1].x + Robo[1].width /2){
			SDL_RenderCopy(gMainRenderer, Robo[0].texture, &Robo[0].src,&Robo[0].dst);
			//SDL_RenderCopy(gMainRenderer, Robo[1].texture, &Robo[1].src,&Robo[1].dst);
		}
		if(Robo[1].x > Robo[0].x + Robo[0].width /2){
		SDL_RenderCopy(gMainRenderer, Robo[1].texture, &Robo[1].src,&Robo[1].dst);
		}
		if(Robo[0].x + Robo[0].width > Robo[1].x + Robo[1].width /2){
			SDL_RenderCopyEx(gMainRenderer,Robo[0].texture,&Robo[0].src,&Robo[0].dst,0,NULL,SDL_FLIP_HORIZONTAL);
			//SDL_RenderCopyEx(gMainRenderer,Robo[1].texture,&Robo[1].src,&Robo[1].dst,0,NULL,SDL_FLIP_HORIZONTAL);
		}
		if(Robo[1].x < Robo[0].x + Robo[0].width /2){
		SDL_RenderCopyEx(gMainRenderer,Robo[1].texture,&Robo[1].src,&Robo[1].dst,0,NULL,SDL_FLIP_HORIZONTAL);
		}
		//HPバーについて
		HPbar[0].dst = {HPbar[0].x, HPbar[0].y, 394*Robo[0].hp/100,44}; // 画像のコピー先の座標と領域（x, y, w, h)
	   	HPbar[1].dst = {HPbar[1].x, HPbar[1].y, (100 - Robo[1].hp)*394/100,44}; // 画像のコピー先の座標と領域（x, y, w, h)
		HPbar_frame[0].dst = {HPbar_frame[0].x, HPbar_frame[0].y, 400,50}; // 画像のコピー先の座標と領域（x, y, w, h)
	   	HPbar_frame[1].dst = {HPbar_frame[1].x, HPbar_frame[1].y, 400,50}; // 画像のコピー先の座標と領域（x, y, w, h)
		Player[0].dst = {Player[0].x, Player[0].y, 32,44}; // 画像のコピー先の座標と領域（x, y, w, h)
	   	Player[1].dst = {Player[1].x, Player[1].y, 32,44}; // 画像のコピー先の座標と領域（x, y, w, h)
		SDL_RenderCopy(gMainRenderer, HPbar_frame[0].texture, &HPbar_frame[0].src,&HPbar_frame[0].dst);
		SDL_RenderCopy(gMainRenderer, HPbar_frame[1].texture, &HPbar_frame[1].src,&HPbar_frame[1].dst);
		SDL_RenderCopy(gMainRenderer, HPbar[0].texture, &HPbar[0].src,&HPbar[0].dst);
		SDL_RenderCopy(gMainRenderer, HPbar[1].texture, &HPbar[1].src,&HPbar[1].dst);
		SDL_RenderCopy(gMainRenderer, Player[0].texture, &Player[0].src,&Player[0].dst);
		SDL_RenderCopy(gMainRenderer, Player[1].texture, &Player[1].src,&Player[1].dst);

		if(Win_flag == 1){
			TTF_Init(); // TrueTypeフォントを用いるための初期化
			TTF_Font *font_win = TTF_OpenFont("./GenShinGothic-Bold.ttf", 50); // TrueTypeフォントデータを格納する構造体
			SDL_Color red = {0x00, 0xFF, 0x00, 0x00};	// フォントの色を指定するための構造体（白色）
			SDL_Surface* strings_win = TTF_RenderUTF8_Blended(font_win, " You Win!!", red); // サーフェイス（メインメモリ上の描画データ）を格納する構造体
			SDL_Texture *texture_strings_win = SDL_CreateTextureFromSurface(gMainRenderer, strings_win);
			SDL_Rect src_rect_strings_win = {0, 0, strings_win->w, strings_win->h}; // コピー元画像の
			SDL_Rect dst_rect_strings_win = {250, 350, 400,200}; // 画像のコピー先の座標と領域（x, y, w, h)
			SDL_RenderCopy(gMainRenderer, texture_strings_win, &src_rect_strings_win,&dst_rect_strings_win);
		}
		if(Loose_flag == 1){
			TTF_Init(); // TrueTypeフォントを用いるための初期化
			TTF_Font *font_loose = TTF_OpenFont("./GenShinGothic-Bold.ttf", 50); // TrueTypeフォントデータを格納する構造体
			SDL_Color red = {0x2F, 0x00, 0xAB, 0x01};	// フォントの色を指定するための構造体（白色）
			SDL_Surface* strings_loose = TTF_RenderUTF8_Blended(font_loose, " You Lose...", red); // サーフェイス（メインメモリ上の描画データ）を格納する構造体
			SDL_Texture *texture_strings_loose = SDL_CreateTextureFromSurface(gMainRenderer, strings_loose);
			SDL_Rect src_rect_strings_loose = {0, 0, strings_loose->w, strings_loose->h}; // コピー元画像の
			SDL_Rect dst_rect_strings_loose = {250, 350, 400,200}; // 画像のコピー先の座標と領域（x, y, w, h)
			SDL_RenderCopy(gMainRenderer, texture_strings_loose, &src_rect_strings_loose,&dst_rect_strings_loose);
		}

		//たまに関してrendercopyしてpresent	してる
		OverlapBlock_bullet0();
		OverlapBlock_bullet1();
		OverlapBlock_bom0();
		OverlapBlock_bom1();
		
		Draw_Bakudan();
		Draw_Bullet();
}

//ロボが重なっているか判断する
int Robo_calc(void){
	if( (Robo[0].x + Robo[0].width) > Robo[1].x
		&&Robo[0].x < (Robo[1].x + Robo[1].width)
		&& (Robo[0].y + Robo[0].width) > Robo[1].y
		&& Robo[0].y < (Robo[1].y + Robo[1].width)
		)
		return 1;

	else
	return 0;
	}

//右移動のときの右線と相手の左線との当たり判定の計算
int Robo_calc_x_right(int clientID){
	if(clientID == 0){//プレイヤー0の時
	if(Robo[0].x + Robo[0].width > Robo[1].x && Robo[0].x + Robo[0].width < Robo[1].x + Robo[1].width)
		return 1;
	else
	return 0;
	}
	if(clientID == 1){
	if(Robo[1].x + Robo[1].width > Robo[0].x && Robo[1].x + Robo[1].width < Robo[0].x + Robo[0].width)
		return 1;
	else
	return 0;
	}
}

//左移動のときの左線と相手の右線との当たり判定の計算
int Robo_calc_x_left(int clientID){
	if(clientID == 0){//プレイヤー0の時
	if(Robo[0].x < Robo[1].x + Robo[1].width && Robo[0].x > Robo[1].x)
		return 1;
	else
	return 0;
	}
	if(clientID == 1){
	if(Robo[1].x < Robo[0].x + Robo[0].width && Robo[1].x > Robo[0].x)
		return 1;
	else
	return 0;
	}
}

//y座標について当たり判定の計算(上移動)
int Robo_calc_y_up(int clientID){
	if(clientID == 0){//プレイヤー0の時
	if(Robo[0].y < Robo[1].y + Robo[1].height && Robo[0].y > Robo[1].y)
		return 1;
	else
	return 0;
	}
	if(clientID == 1){
	if(Robo[1].y < Robo[0].y + Robo[0].height && Robo[1].y > Robo[0].y)		
		return 1;

	else
	return 0;
	}
}

//y座標について当たり判定の計算(下移動)
int Robo_calc_y_down(int clientID){
	if(clientID == 0){//プレイヤー0の時
	if(Robo[0].y + Robo[0].height > Robo[1].y && Robo[0].y + Robo[0].height < Robo[1].y + Robo[1].height)
		return 1;
	else
	return 0;
	}
	if(clientID == 1){
	if(Robo[1].y + Robo[1].height > Robo[0].y && Robo[1].y + Robo[1].height < Robo[0].y + Robo[0].height)	
		return 1;

	else
	return 0;
	}
}



//ロボの初期設定
void Init_Robo(void){
		//初期座標
		Robo[0].x = MAP_ChipSize;
		Robo[0].y = MAP_ChipSize;
		Robo[0].width = 100;
		Robo[0].height = 100;
		Robo[0].hp = 100;
		Robo[0].type = 0;
		Robo[0].Dcount = 0;
		Robo[0].speed = MOVE_KYORI;
		
		Robo[1].x = WIN_X - (MAP_ChipSize*3);
		Robo[1].y = WIN_Y - (MAP_ChipSize*3);
		Robo[1].width = 100;
		Robo[1].height = 100;
		Robo[1].hp = 100;
		Robo[1].type = 0;
		Robo[1].Dcount = 0;
		Robo[1].speed = MOVE_KYORI;
		//初期画像
		Robo[0].image = IMG_Load("robo0.png");
		Robo[1].image = IMG_Load("robo1.png");
		Robo[0].image_dash = IMG_Load("robo0_henkei.png");
		Robo[1].image_dash = IMG_Load("robo1_henkei.png");
		Robo[0].image_box = IMG_Load("robo0_box.png");
		Robo[1].image_box = IMG_Load("robo1_box.png");
		Robo[0].texture = SDL_CreateTextureFromSurface(gMainRenderer, Robo[0].image_box);
		Robo[1].texture = SDL_CreateTextureFromSurface(gMainRenderer, Robo[1].image_box);
	   	Robo[0].src = {0, 0, Robo[0].image->w, Robo[0].image->h}; // コピー元画像の
		Robo[1].src = {0, 0, Robo[1].image->w, Robo[1].image->h}; // コピー元画像の
		//移動キーのflag
		Robo[0].Rflag = 0;
		Robo[0].Lflag = 0;
		Robo[0].Uflag = 0;
		Robo[0].Dflag = 0;
		Robo[1].Rflag = 0;
		Robo[1].Lflag = 0;
		Robo[1].Uflag = 0;
		Robo[1].Dflag = 0;
}
//HPバー,プレイヤー情報の初期設定
void Init_HPbar(void){
		//初期座標
		HPbar[0].x = 35;
		HPbar[0].y = 3;
		HPbar[0].width = Robo[0].hp*3;
		HPbar[0].height = 44;
		
		HPbar[1].x = WIN_X - 397-32;
		HPbar[1].y = 3;
		HPbar[1].width = Robo[1].hp*3;
		HPbar[1].height = 44;

		HPbar_frame[0].x = WIN_X - 432;
		HPbar_frame[0].y = 0;
		HPbar_frame[0].width = 400;
		HPbar_frame[0].height = 50;

		HPbar_frame[1].x = 32;
		HPbar_frame[1].y = 0;
		HPbar_frame[1].width = 400;
		HPbar_frame[1].height = 50;

		Player[0].x = 0;
		Player[0].y = 0;
		Player[0].width = 32;
		Player[0].height = 44;

		Player[1].x = 832;
		Player[1].y = 0;
		Player[1].width = 32;
		Player[1].height = 44;

		//初期画像
		HPbar[0].image = IMG_Load("HPbar0.png");
		HPbar[1].image = IMG_Load("HPbar_gray.png");
		HPbar_frame[0].image = IMG_Load("HPbar_frame.png");
		HPbar_frame[1].image = IMG_Load("HPbar_frame_gray.png");
		Player[0].image = IMG_Load("1p.png");
		Player[1].image = IMG_Load("2p.png");
		HPbar[0].texture = SDL_CreateTextureFromSurface(gMainRenderer, HPbar[0].image);
		HPbar[1].texture = SDL_CreateTextureFromSurface(gMainRenderer, HPbar[1].image);
		HPbar_frame[0].texture = SDL_CreateTextureFromSurface(gMainRenderer, HPbar_frame[0].image);
		HPbar_frame[1].texture = SDL_CreateTextureFromSurface(gMainRenderer, HPbar_frame[1].image);
		Player[0].texture = SDL_CreateTextureFromSurface(gMainRenderer, Player[0].image);
		Player[1].texture = SDL_CreateTextureFromSurface(gMainRenderer, Player[1].image);
	   	HPbar[0].src = {0, 0, HPbar[0].image->w, HPbar[0].image->h}; // コピー元画像の
		HPbar[1].src = {0, 0, HPbar[1].image->w, HPbar[1].image->h}; // コピー元画像の
		HPbar_frame[0].src = {0, 0, HPbar_frame[0].image->w, HPbar_frame[0].image->h}; // コピー元画像の
		HPbar_frame[1].src = {0, 0, HPbar_frame[0].image->w, HPbar_frame[0].image->h}; // コピー元画像の
		Player[0].src = {0, 0, Player[0].image->w, Player[0].image->h}; // コピー元画像の
		Player[1].src = {0, 0, Player[1].image->w, Player[1].image->h}; // コピー元画像の
}

/*弾とマップの矩形の当たり判定の関数*/
SDL_bool OverlapBlock_bullet0(void)
{
    SDL_bool ret    = SDL_FALSE;
	for(int n=0;n<=PSHOT_NUM-1; n++){
	SDL_Rect mapbullet;
	shot_p0[n].mapbullet = {shot_p0[n].x +WIN_X/2,shot_p0[n].y + WIN_Y/2,shot_p0[n].width,shot_p0[n].height};
    /* 壁の矩形一覧と弾の当たり矩形との重なりを調べ，
     * 重なっていればフラグを折って弾を消す
     */
	if(shot_p0[n].flag == 1){
    for (int i = 0; i < gMap.bnum; i++) {
        SDL_Rect r;
        if (SDL_IntersectRect(&shot_p0[n].mapbullet, &(gMap.blocks[i]), &r)) {
            if (ret = SDL_TRUE){
				shot_p0[n].flag = 0;
							}
        }
    }
	}
	}
    return ret;
}

/*弾とマップの矩形の当たり判定の関数*/
SDL_bool OverlapBlock_bullet1(void)
{
    SDL_bool ret    = SDL_FALSE;
	for(int n=0;n<=PSHOT_NUM-1; n++){
	SDL_Rect mapbullet;
	shot_p1[n].mapbullet = {shot_p1[n].x +WIN_X/2,shot_p1[n].y + WIN_Y/2,shot_p1[n].width,shot_p1[n].height};
    /* 壁の矩形一覧と弾の当たり矩形との重なりを調べ，
     * 重なっていればフラグを折って弾を消す
     */
	if(shot_p1[n].flag == 1){
    for (int i = 0; i < gMap.bnum; i++) {
        SDL_Rect r;
        if (SDL_IntersectRect(&shot_p1[n].mapbullet, &(gMap.blocks[i]), &r)) {
            if (ret = SDL_TRUE){
				shot_p1[n].flag = 0;
							}
        }
    }
	}
	}
    return ret;
}

/*爆弾とマップの矩形の当たり判定の関数*/
SDL_bool OverlapBlock_bom0(void)
{
    SDL_bool ret    = SDL_FALSE;
	for(int n=0;n<=PBAKUDAN_NUM; n++){
	SDL_Rect mapbom;
	bakudan_p0[n].mapbom = {bakudan_p0[n].x +WIN_X/2,bakudan_p0[n].y + WIN_Y/2,bakudan_p0[n].width,bakudan_p0[n].height};
    /* 壁の矩形一覧と弾の当たり矩形との重なりを調べ，
     * 重なっていればフラグを折って弾を消す
     */
	if(bakudan_p0[n].flag == 1){
    for (int i = 0; i < gMap.bnum; i++) {
        SDL_Rect r;
        if (SDL_IntersectRect(&bakudan_p0[n].mapbom, &(gMap.blocks[i]), &r)) {
            if (ret = SDL_TRUE){
				bakudan_p0[n].flag = 2;
				bakudan_p0[n].excount = 100;
				Mix_PlayChannel(1, chunk_bang, 0); // 効果音の再生（1回再生）
							}
        }
    }
	}
	}
    return ret;
}

/*爆弾とマップの矩形の当たり判定の関数*/
SDL_bool OverlapBlock_bom1(void)
{
    SDL_bool ret    = SDL_FALSE;
	for(int n=0;n<=PBAKUDAN_NUM; n++){
	SDL_Rect mapbom;
	bakudan_p1[n].mapbom = {bakudan_p1[n].x +WIN_X/2,bakudan_p1[n].y + WIN_Y/2,bakudan_p1[n].width,bakudan_p1[n].height};
    /* 壁の矩形一覧と弾の当たり矩形との重なりを調べ，
     * 重なっていればフラグを折って弾を消す
     */
	if(bakudan_p1[n].flag == 1){
    for (int i = 0; i < gMap.bnum; i++) {
        SDL_Rect r;
        if (SDL_IntersectRect(&bakudan_p1[n].mapbom, &(gMap.blocks[i]), &r)) {
            if (ret = SDL_TRUE){
				bakudan_p1[n].flag = 2;
				bakudan_p1[n].excount = 100;
				Mix_PlayChannel(1, chunk_bang, 0); // 効果音の再生（1回再生）
							}
        }
    }
	}
	}
    return ret;
}
//弾の初期設定
void Init_bullet(void){
		//弾
		for(int i=0;i<=PSHOT_NUM-1; i++){
			shot_p0[i].flag = 0;
			shot_p0[i].bullet_gh = IMG_Load("tama.png");
			shot_p0[i].texture_bullet = SDL_CreateTextureFromSurface(gMainRenderer, shot_p0[i].bullet_gh);
			shot_p0[i].width = 25;
			shot_p0[i].height = 25;
			shot_p0[i].src_rect_bullet = {0, 0,shot_p0[i].bullet_gh->w,shot_p0[i].bullet_gh->h};

			shot_p1[i].flag= 0;
			shot_p1[i].bullet_gh = IMG_Load("tama.png");
			shot_p1[i].texture_bullet = SDL_CreateTextureFromSurface(gMainRenderer, shot_p1[i].bullet_gh);
			shot_p1[i].width = 25;
			shot_p1[i].height = 25;
			shot_p1[i].src_rect_bullet = {0, 0,shot_p1[i].bullet_gh->w,shot_p1[i].bullet_gh->h};
		}
}

//爆弾の初期化設定
void Init_bakudan(void){
	for(int i=0;i<PBAKUDAN_NUM;i++){
		bakudan_p0[i].flag = 0;
		bakudan_p0[i].bakudan_gh = IMG_Load("bakudan.png");
		bakudan_p0[i].texture_bakudan = SDL_CreateTextureFromSurface(gMainRenderer,bakudan_p0[i].bakudan_gh);
		bakudan_p0[i].width = 40;
		bakudan_p0[i].height = 40;
		bakudan_p0[i].src_rect_bakudan = {0,0,bakudan_p0[i].bakudan_gh->w,bakudan_p0[i].bakudan_gh->h};

		bakudan_p0[i].explosion = IMG_Load("explosion.png");
		bakudan_p0[i].texture_ex = SDL_CreateTextureFromSurface(gMainRenderer,bakudan_p0[i].explosion);
		bakudan_p0[i].src_rect_ex = {0,0,bakudan_p0[i].explosion->w,bakudan_p0[i].explosion->h};
		bakudan_p0[i].excount = 0;

		bakudan_p1[i].flag = 0;
		bakudan_p1[i].bakudan_gh = IMG_Load("bakudan.png");
		bakudan_p1[i].texture_bakudan = SDL_CreateTextureFromSurface(gMainRenderer,bakudan_p0[i].bakudan_gh);
		bakudan_p1[i].width = 40;
		bakudan_p1[i].height = 40;
		bakudan_p1[i].src_rect_bakudan = {0,0,bakudan_p1[i].bakudan_gh->w,bakudan_p1[i].bakudan_gh->h};

		bakudan_p1[i].explosion = IMG_Load("explosion.png");
		bakudan_p1[i].texture_ex = SDL_CreateTextureFromSurface(gMainRenderer,bakudan_p1[i].explosion);
		bakudan_p1[i].src_rect_ex = {0,0,bakudan_p1[i].explosion->w,bakudan_p1[i].explosion->h};
		bakudan_p1[i].excount = 0;
	}

}
//タイトルを描画する関数
void Init_Title(int clientID){
	SDL_Rect src_rect;
	SDL_Rect dest_rect;
	/* タイトル関連 */
	SDL_Event event;
	SDL_Surface* strings; // サーフェイス（メインメモリ上の描画データ）を格納する構造体
	TTF_Font *font; // TrueTypeフォントデータを格納する構造体
	SDL_Color white = {0x2F, 0x2F, 0x2F, 0x2F};	// フォントの色を指定するための構造体（白色）
	TTF_Init(); // TrueTypeフォントを用いるための初期化
    // 文字列描画処理
    font = TTF_OpenFont("./GenShinGothic-Bold.ttf", 30); // fonts-japanese-gothicフォントを12ポイントで使用（読み込み）
	if(clientID == 0){
    strings = TTF_RenderUTF8_Blended(font, " You Are 1P", white); // 描画文字の作成と格納（白色のfonts-japanese-gothicフォントで，文字列をサーフェイスに描画＝データとして格納）
	}
	if(clientID == 1){
    strings = TTF_RenderUTF8_Blended(font, " You Are 2P", white); // 描画文字の作成と格納（白色のfonts-japanese-gothicフォントで，文字列をサーフェイスに描画＝データとして格納）
	}
	/* タイトル関連 */
	SDL_Surface *image0 = IMG_Load("title.jpg");
	SDL_Surface *image1 = IMG_Load("push_enter.png");
	SDL_Surface *image2 = IMG_Load("title_rogo.png");
	SDL_Surface *image3 = IMG_Load("title_wait.jpeg");
	SDL_Surface *image4 = IMG_Load("wait.png");
	SDL_Texture *texture0 = SDL_CreateTextureFromSurface(gMainRenderer, image0);
	SDL_Texture *texture1 = SDL_CreateTextureFromSurface(gMainRenderer, image1);
	SDL_Texture *texture2 = SDL_CreateTextureFromSurface(gMainRenderer, image2);
	SDL_Texture *texture_strings = SDL_CreateTextureFromSurface(gMainRenderer, strings);
	SDL_Texture *texture3 = SDL_CreateTextureFromSurface(gMainRenderer, image3);
	SDL_Texture *texture4 = SDL_CreateTextureFromSurface(gMainRenderer, image4);
	SDL_Rect src_rect0 = {0, 0, image0->w, image0->h}; // コピー元画像の
	SDL_Rect dst_rect0 = {0, 0, WIN_X,WIN_Y}; // 画像のコピー先の座標と領域（x, y, w, h)
	SDL_Rect src_rect1 = {0, 0, image1->w, image1->h}; // コピー元画像の
	SDL_Rect dst_rect1 = {300, 500, image1->w,image1->h}; // 画像のコピー先の座標と領域（x, y, w, h)
	SDL_Rect src_rect2 = {0, 0, image2->w, image2->h}; // コピー元画像の
	SDL_Rect dst_rect2 = {50, 130, image2->w*0.75, image2->h*0.75}; // 画像のコピー先の座標と領域（x, y, w, h)
	SDL_Rect src_rect_strings = {0, 0, strings->w, strings->h}; // コピー元画像の
	SDL_Rect dst_rect_strings = {300, 600, 400,100}; // 画像のコピー先の座標と領域（x, y, w, h)
	SDL_Rect src_rect3 = {0, 0, image3->w, image3->h}; // コピー元画像の
	SDL_Rect dst_rect3 = {0, 0, WIN_X,WIN_Y}; // 画像のコピー先の座標と領域（x, y, w, h)
	SDL_Rect src_rect4 = {0, 0, image4->w, image4->h}; // コピー元画像の
	SDL_Rect dst_rect4 = {200, 200, image4->w, image4->h}; // 画像のコピー先の座標と領域（x, y, w, h)
	SDL_RenderCopy(gMainRenderer, texture0, &src_rect0,&dst_rect0);
	SDL_RenderCopy(gMainRenderer, texture1, &src_rect1,&dst_rect1);
	SDL_RenderCopy(gMainRenderer, texture2, &src_rect2,&dst_rect2);
	SDL_RenderCopy(gMainRenderer, texture_strings, &src_rect_strings,&dst_rect_strings);
	SDL_RenderCopy(gMainRenderer, texture3, &src_rect3,&dst_rect3);
	SDL_RenderCopy(gMainRenderer, texture4, &src_rect4,&dst_rect4);
	SDL_RenderPresent(gMainRenderer);

	while(GUI_flag == 1  && Robo[clientID].title == 0){
		
		SDL_RenderClear(gMainRenderer);
		SDL_RenderCopy(gMainRenderer, texture0, &src_rect0,&dst_rect0);
		SDL_RenderCopy(gMainRenderer, texture1, &src_rect1,&dst_rect1);
		SDL_RenderCopy(gMainRenderer, texture2, &src_rect2,&dst_rect2);
		SDL_RenderCopy(gMainRenderer, texture_strings, &src_rect_strings,&dst_rect_strings);
		SDL_RenderPresent(gMainRenderer);
		SendRecvManager();

		 if(SDL_PollEvent(&event)){
			 switch (event.type) {
                case SDL_KEYDOWN:                
					   switch(event.key.keysym.sym){
							case SDLK_RETURN: 
								SendTitleData(clientID);
								printf("debug\n");
                                break;
					   }
					}
		 }
	}
		 
	while(GUI_flag == 1  && Robo[clientID].title == 1){

		SDL_RenderClear(gMainRenderer);
		SDL_RenderCopy(gMainRenderer, texture3, &src_rect3,&dst_rect3);
		SDL_RenderCopy(gMainRenderer, texture4, &src_rect4,&dst_rect4);
		SDL_RenderPresent(gMainRenderer);
		SendRecvManager();
		
			if(Robo[0].title == 1 && Robo[1].title == 1){
				//タイトル画面消去
				SDL_RenderClear(gMainRenderer);
				//ロボ、弾、HPバーの初期化
				Init_Robo();
				Init_bullet();
				Init_bakudan();
				Init_HPbar();
				game_endflag = 0;
				/*マップ転送領域の設定*/
				Map_rect.w = Map_rect.h = WIN_X;
				Map_rect.x = Map_rect.y = WIN_X / 2;
				/* マップ画像の作成 */
				MakeMap();
				/*ウィンドウへの描画 初めの*/
				RenderWindow();		

				Game_flag = 1; //ゲーム画面操作ループに移行
				GUI_flag = 0; //タイトル画面操作ループから抜け出す
			}
	}
	

	}
//タイトル画面から待機画面に変更する関数
void SetRoboTitleData(int clientID){
	if(clientID == 0){
		Robo[0].title = 1;
	}
	if(clientID == 1){
		Robo[1].title = 1;
	}
}

//弾をセット
void SetBulletData(int pos,int bullet_num){
	if(pos == 0  && Robo[0].x < Robo[1].x){
		shot_p0[bullet_num].vec = 0;//右に飛ばす
		shot_p0[bullet_num].flag = 1;
		shot_p0[bullet_num].x = (Robo[0].x + 100);
		shot_p0[bullet_num].y = (Robo[0].y + 50);
		//if(Robo[0].y < Robo[1].y){
		//shot_p0[bullet_num].rad = atan2((Robo[1].x-shot_p0[bullet_num].x),(Robo[1].y-shot_p0[bullet_num].y));
		//}
		//if(Robo[0].y>Robo[1].y){
		shot_p0[bullet_num].rad = atan2(((Robo[1].y+50) - shot_p0[bullet_num].y),((Robo[1].x+50) - shot_p0[bullet_num].x));
		//}
	 }
	if(pos == 0  && Robo[0].x > Robo[1].x){
		shot_p0[bullet_num].vec = 1;//左に飛ばす
		shot_p0[bullet_num].flag = 1;
		shot_p0[bullet_num].x = Robo[0].x;
		shot_p0[bullet_num].y = (Robo[0].y + 50);
		shot_p0[bullet_num].rad = atan2(((Robo[1].y+50) - shot_p0[bullet_num].y),((Robo[1].x+50) - shot_p0[bullet_num].x));
	 }
	 if(pos == 1 && Robo[1].x < Robo[0].x){
		 shot_p1[bullet_num].vec = 0;//右に飛ばす
		shot_p1[bullet_num].flag = 1;
		shot_p1[bullet_num].x = (Robo[1].x + 100);
		shot_p1[bullet_num].y = (Robo[1].y + 50);
		shot_p1[bullet_num].rad = atan2(((Robo[0].y+50) - shot_p1[bullet_num].y),((Robo[0].x+50) - shot_p1[bullet_num].x));
	 }
	 if(pos == 1 && Robo[1].x > Robo[0].x){
		shot_p1[bullet_num].vec = 1;//左に飛ばす
		shot_p1[bullet_num].flag = 1;
		shot_p1[bullet_num].x = Robo[1].x;
		shot_p1[bullet_num].y = (Robo[1].y + 50);
		shot_p1[bullet_num].rad = atan2(((Robo[0].y+50) - shot_p1[bullet_num].y),((Robo[0].x+50) - shot_p1[bullet_num].x));
	 }
	 Mix_PlayChannel(1, chunk, 0);

}
//爆弾をセット
void SetBakudanData(int pos,int bullet_num){
	if(pos == 0  && Robo[0].x < Robo[1].x){
		bakudan_p0[bullet_num].vec = 0;//右に飛ばす
		bakudan_p0[bullet_num].flag = 1;
		bakudan_p0[bullet_num].x = (Robo[0].x + 100);
		bakudan_p0[bullet_num].y = (Robo[0].y + 50);
		bakudan_p0[bullet_num].rad = atan2(((Robo[1].y+50) - bakudan_p0[bullet_num].y),((Robo[1].x+50) - bakudan_p0[bullet_num].x));
	 }
	if(pos == 0  && Robo[0].x > Robo[1].x){
		bakudan_p0[bullet_num].vec = 1;//左に飛ばす
		bakudan_p0[bullet_num].flag = 1;
		bakudan_p0[bullet_num].x = Robo[0].x;
		bakudan_p0[bullet_num].y = (Robo[0].y + 50);
		bakudan_p0[bullet_num].rad = atan2(((Robo[1].y+50) - bakudan_p0[bullet_num].y),((Robo[1].x+50) - bakudan_p0[bullet_num].x));
	 }
	 if(pos == 1 && Robo[1].x < Robo[0].x){
		bakudan_p1[bullet_num].vec = 0;//右に飛ばす
		bakudan_p1[bullet_num].flag = 1;
		bakudan_p1[bullet_num].x = (Robo[1].x + 100);
		bakudan_p1[bullet_num].y = (Robo[1].y + 50);
		bakudan_p1[bullet_num].rad = atan2(((Robo[0].y+50) - bakudan_p1[bullet_num].y),((Robo[0].x+50) - bakudan_p1[bullet_num].x));
	 }
	 if(pos == 1 && Robo[1].x > Robo[0].x){
		bakudan_p1[bullet_num].vec = 1;//左に飛ばす
		bakudan_p1[bullet_num].flag = 1;
		bakudan_p1[bullet_num].x = Robo[1].x;
		bakudan_p1[bullet_num].y = (Robo[1].y + 50);
		bakudan_p1[bullet_num].rad = atan2(((Robo[0].y+50) - bakudan_p1[bullet_num].y),((Robo[0].x+50) - bakudan_p1[bullet_num].x));
	 }

}

//移動したあとにその座標を格納している
void SetRoboMoveData(int key,int clientID,int movedzahyo){
	if (key ==1 && clientID == 0){//右
		Robo[0].x = movedzahyo;
	}
	if (key ==1 && clientID == 1){
		Robo[1].x = movedzahyo;
	}
	if (key ==2 && clientID == 0){//左
		Robo[0].x = movedzahyo;
	}
	if (key ==2 && clientID == 1){
		Robo[1].x = movedzahyo;
	}
	if (key ==3 && clientID == 0){//上
		Robo[0].y = movedzahyo;
	}
	if (key ==3 && clientID == 1){
		Robo[1].y = movedzahyo;
	}
	if (key ==4 && clientID == 0){//下
		Robo[0].y = movedzahyo;
	}
	if (key ==4 && clientID == 1){
		Robo[1].y = movedzahyo;
	}
}

//ロボにダッシュ状態を付与する関数
void SetRoboDashData(int clientID){
	if(clientID == 0){
		Robo[0].type = 1;
		Robo[0].texture = SDL_CreateTextureFromSurface(gMainRenderer, Robo[0].image_dash);
		Robo[0].Dcount = 0;
		Robo[0].speed = MOVE_KYORI*2;
	}
	if(clientID == 1){
		Robo[1].type = 1;
		Robo[1].texture = SDL_CreateTextureFromSurface(gMainRenderer, Robo[1].image_dash);
		Robo[1].Dcount = 0;
		Robo[1].speed = MOVE_KYORI*2;
	}
}
//ダッシュ状態からもとに戻る関数
void SetRoboDash_KaizyoData(int clientID){
	if(clientID == 0){
		Robo[0].type = 2;
		Robo[0].texture = SDL_CreateTextureFromSurface(gMainRenderer, Robo[0].image);
		Robo[0].Dcount = 200;
		Robo[0].speed = MOVE_KYORI;
	}
	if(clientID == 1){
		Robo[1].type = 2;
		Robo[1].texture = SDL_CreateTextureFromSurface(gMainRenderer, Robo[1].image);
		Robo[1].Dcount = 200;
		Robo[1].speed = MOVE_KYORI;
	}
}
//ボックス状態からもとに戻る関数
void SetRoboBox_KaizyoData(int clientID){
	if(clientID == 0){
		Robo[0].type = 0;
		Robo[0].texture = SDL_CreateTextureFromSurface(gMainRenderer, Robo[0].image);
		Robo[0].Dcount = 200;
		Robo[0].speed = MOVE_KYORI;
	}
	if(clientID == 1){
		Robo[1].type = 0;
		Robo[1].texture = SDL_CreateTextureFromSurface(gMainRenderer, Robo[1].image);
		Robo[1].Dcount = 200;
		Robo[1].speed = MOVE_KYORI;
	}
}


//弾を描画する関数
void Draw_Bullet(void){
		double dist0,dist1;//それぞれの弾とロボの距離
		for(int i=0;i<=PSHOT_NUM-1;i++){
	if(shot_p0[i].flag == 1){
			shot_p0[i].dst_rect_bullet = {shot_p0[i].x,shot_p0[i].y,25,25}; // コピー元画像の
			SDL_RenderCopy(gMainRenderer, shot_p0[i].texture_bullet, &shot_p0[i].src_rect_bullet,&shot_p0[i].dst_rect_bullet);
			if(shot_p0[i].vec == 0){
				shot_p0[i].x += cos(shot_p0[i].rad)*PSHOT_SPEED;
				shot_p0[i].y += sin(shot_p0[i].rad)*PSHOT_SPEED;
			}
			if(shot_p0[i].vec == 1){
				shot_p0[i].x += cos(shot_p0[i].rad)*PSHOT_SPEED;
				shot_p0[i].y += sin(shot_p0[i].rad)*PSHOT_SPEED;
			}
			//弾が画面外、キャラに当たったらフラグをおる
			if(shot_p0[i].x > 900 || shot_p0[i].x < -25 || shot_p0[i].y > 900 || shot_p0[i].y < -25){
				shot_p0[i].flag = 0;
				shot_p0[i].vec = -1;
			}
			//弾がロボに当たると消える処理
			dist0 = calc_distance(shot_p0[i].x + shot_p1[i].width/2, shot_p0[i].y + shot_p0[i].height/2, Robo[1].x + 50, Robo[1].y + 50);
			if(dist0 <= 50){

				shot_p0[i].flag = 0;
				Mix_PlayChannel(1, chunk_dm, 0); // 効果音の再生（1回再生）
				Robo[1].hp -=10;
			}
		}
	if(shot_p1[i].flag == 1){
			shot_p1[i].dst_rect_bullet = {shot_p1[i].x,shot_p1[i].y,25,25}; // コピー元画像の
			SDL_RenderCopy(gMainRenderer, shot_p1[i].texture_bullet,  &shot_p1[i].src_rect_bullet,&shot_p1[i].dst_rect_bullet);
			if(shot_p1[i].vec == 0){
				shot_p1[i].x += cos(shot_p1[i].rad)*PSHOT_SPEED;
				shot_p1[i].y += sin(shot_p1[i].rad)*PSHOT_SPEED;
			}
			if(shot_p1[i].vec == 1){
				shot_p1[i].x += cos(shot_p1[i].rad)*PSHOT_SPEED;
				shot_p1[i].y += sin(shot_p1[i].rad)*PSHOT_SPEED;
			}
			if(shot_p1[i].x >900 || shot_p1[i].x < -25 || shot_p1[i].y > 900 || shot_p1[i].y < -25){
				shot_p1[i].flag = 0;
				shot_p1[i].vec = -1;
			}
			//弾がロボに当たると消える処理
			dist1 = calc_distance(shot_p1[i].x + shot_p1[i].width/2, shot_p1[i].y + shot_p1[i].height/2,Robo[0].x + 50,Robo[0].y + 50);
			if(dist1 <= 50){
				shot_p1[i].flag = 0;
				Mix_PlayChannel(1, chunk_dm, 0); // 効果音の再生（1回再生）
				Robo[0].hp -= 10;
			}
		}
	}
	SDL_RenderPresent(gMainRenderer);
	if(Robo[0].hp <= 0 || Robo[1].hp <= 0){
		game_endflag = 1;
	}
}
void Draw_Bakudan(void){
			double dist0,dist1;//それぞれの弾とロボの距離
	for(int i=0;i<=PBAKUDAN_NUM-1;i++){

		if(bakudan_p0[i].flag == 1){
			bakudan_p0[i].dst_rect_bakudan = {bakudan_p0[i].x,bakudan_p0[i].y,bakudan_p0[i].width,bakudan_p0[i].height}; // コピー元画像の
			bakudan_p0[i].dst_rect_ex = {bakudan_p0[i].x -5,bakudan_p0[i].y +5,bakudan_p0[i].explosion->w,bakudan_p0[i].explosion->h};
			SDL_RenderCopy(gMainRenderer, bakudan_p0[i].texture_bakudan, &bakudan_p0[i].src_rect_bakudan,&bakudan_p0[i].dst_rect_bakudan);
			if(bakudan_p0[i].vec == 0){
				bakudan_p0[i].x += cos(bakudan_p0[i].rad)*PBAKUDAN_SPEED;
				bakudan_p0[i].y += sin(bakudan_p0[i].rad)*PBAKUDAN_SPEED;
			}
			if(bakudan_p0[i].vec == 1){
				bakudan_p0[i].x += cos(bakudan_p0[i].rad)*PBAKUDAN_SPEED;
				bakudan_p0[i].y += sin(bakudan_p0[i].rad)*PBAKUDAN_SPEED;
			}

			//爆弾が画面外、キャラに当たったらフラグをおる
			if(bakudan_p0[i].x > 900 || bakudan_p0[i].x < -25 || bakudan_p0[i].y > 900 || bakudan_p0[i].y < -25){
				bakudan_p0[i].flag = 2;
				bakudan_p0[i].excount = 100;
				bakudan_p0[i].vec = -1;
			}
			//爆弾がロボに当たると消える処理
			dist0 = calc_distance(bakudan_p0[i].x + bakudan_p0[i].width/2, bakudan_p0[i].y + bakudan_p0[i].height/2, Robo[1].x + 50, Robo[1].y + 50);
			if(dist0 <= 50){
				bakudan_p0[i].flag = 2;
				bakudan_p0[i].excount = 100;
				Mix_PlayChannel(1, chunk_bang, 0); // 効果音の再生（1回再生）
				Robo[1].hp -=30;
			}
		}
		if(bakudan_p1[i].flag == 1){
			bakudan_p1[i].dst_rect_bakudan = {bakudan_p1[i].x,bakudan_p1[i].y,bakudan_p1[i].width,bakudan_p1[i].height}; // コピー元画像の
			bakudan_p1[i].dst_rect_ex = {bakudan_p1[i].x -5,bakudan_p1[i].y +5,bakudan_p1[i].explosion->w,bakudan_p1[i].explosion->h};//爆発用
			SDL_RenderCopy(gMainRenderer, bakudan_p1[i].texture_bakudan, &bakudan_p1[i].src_rect_bakudan,&bakudan_p1[i].dst_rect_bakudan);
			if(bakudan_p1[i].vec == 0){
				bakudan_p1[i].x += cos(bakudan_p1[i].rad)*PBAKUDAN_SPEED;
				bakudan_p1[i].y += sin(bakudan_p1[i].rad)*PBAKUDAN_SPEED;
			}
			if(bakudan_p1[i].vec == 1){
				bakudan_p1[i].x += cos(bakudan_p1[i].rad)*PBAKUDAN_SPEED;
				bakudan_p1[i].y += sin(bakudan_p1[i].rad)*PBAKUDAN_SPEED;
			}

			//爆弾が画面外、キャラに当たったらフラグをおる
			if(bakudan_p1[i].x > 900 || bakudan_p1[i].x < -25 || bakudan_p1[i].y > 900 || bakudan_p1[i].y < -25){
				bakudan_p1[i].flag = 2;
				bakudan_p1[i].excount = 100;
				bakudan_p1[i].vec = -1;
				//SDL_RenderCopy(gMainRenderer,shot_p0[i].back_texture,&shot_p0[i].prev_rect_bullet,&shot_p0[i].prev_rect_bullet);
			}
			//爆弾がロボに当たると消える処理
			dist1 = calc_distance(bakudan_p1[i].x + bakudan_p1[i].width/2, bakudan_p1[i].y + bakudan_p1[i].height/2, Robo[0].x + 50, Robo[0].y + 50);
			if(dist1 <= 50){
				bakudan_p1[i].flag = 2;
				bakudan_p1[i].excount = 100;
				Mix_PlayChannel(1, chunk_bang, 0); // 効果音の再生（1回再生）
				Robo[0].hp -=30;
			}

		}
		if(bakudan_p0[i].flag == 2)
		{
		 SDL_RenderCopy(gMainRenderer, bakudan_p0[i].texture_ex, &bakudan_p0[i].src_rect_ex,&bakudan_p0[i].dst_rect_ex);
		}

		if(bakudan_p1[i].flag == 2)
		{
		 SDL_RenderCopy(gMainRenderer, bakudan_p1[i].texture_ex, &bakudan_p1[i].src_rect_ex,&bakudan_p1[i].dst_rect_ex);
		}
	}
	if(Robo[0].hp <= 0 || Robo[1].hp <= 0){
		game_endflag = 1;
	}
}


//弾とロボの距離を返す関数
int calc_distance(double x1, double y1, double x2, double y2){
	double dist;
	dist = sqrt((double)((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)));
	return dist;
}

//どちらのプレイヤーが勝利したか判断する関数
int which_win(void){
	if(Robo[0].hp <= 0){
		return 0;//0が体力なくなった
	}
	if(Robo[1].hp <= 0){
		return 1;//1が体力なくなった
	}
}


/*****************************************************************
関数名	: DrawWin
機能	: メインウインドウに勝利結果を表示する
引数	: なし
出力	: なし
*****************************************************************/

void DrawWin(void)
{
	Deleat_tama();
	Win_flag = 1;
	printf("flag = %d",winflag);
	if(winflag == 0){//p1の勝ち
		Robo[0].hp = 0;
	}
	if(winflag == 1){//p0の勝ち
		Robo[1].hp = 0;
	}
	printf("今のrobo0のhp = %d",Robo[0].hp);
	printf("今のrobo1のhp = %d",Robo[1].hp);
	ResetDraw();
	//デバッグ用
	printf("robo0hp=%d,robo1hp = %d",Robo[0].hp,Robo[1].hp);
	SDL_RenderPresent(gMainRenderer);
}
/*****************************************************************
関数名	: DrawLoose
機能	: メインウインドウに敗北結果を表示する
引数	: なし
出力	: なし
*****************************************************************/
void DrawLoose(void)
{
	Deleat_tama();
	Loose_flag = 1;
	printf("flag = %d",winflag);
	if(winflag == 0){//p1の勝ち
		Robo[0].hp = 0;
	}
	if(winflag == 1){//p0の勝ち
		Robo[1].hp = 0;
	}
	printf("今のrobo0のhp = %d",Robo[0].hp);
	printf("今のrobo1のhp = %d",Robo[1].hp);
	ResetDraw();
	//デバッグ用
	printf("robo0hp=%d,robo1hp = %d",Robo[0].hp,Robo[1].hp);
	SDL_RenderPresent(gMainRenderer);
}

//弾とばくだんを消す関数
void Deleat_tama(void){
	for(int i=0;i<=PSHOT_NUM-1;i++){
		shot_p0[i].flag = 0;
		shot_p1[i].flag = 0;
		}
	for(int i=0;i<=PBAKUDAN_NUM-1;i++){
		bakudan_p0[i].flag = 0;
		bakudan_p1[i].flag = 0;
	}


}
//どのフラグを立てるかの関数,サーバからデータ来たあとに使う
void SetMoveRobo(int clientID,int key,int moved){
	if(clientID ==0){
		if(key == 1){
			Robo[0].x = moved;
			Robo[0].Rflag = 1;
		}
		if(key == 2){
			Robo[0].x = moved;
			Robo[0].Lflag = 1;
		}
		if(key == 3){
			Robo[0].y = moved;
			Robo[0].Uflag = 1;
		}
		if(key == 4){
			Robo[0].y = moved;
			Robo[0].Dflag = 1;
		}
		if(key == 5){
			Robo[0].x = moved;
		   Robo[0].Rflag = 0;
		}
		if(key == 6){
			Robo[0].x = moved;
			Robo[0].Lflag = 0;
		}
		if(key == 7){
			Robo[0].y = moved;
		   Robo[0].Uflag = 0;
		}
		if(key == 8){
			Robo[0].y = moved;
		   Robo[0].Dflag = 0;
		}

	}
	if(clientID == 1){
		if(key == 1){
			Robo[1].x = moved;
			Robo[1].Rflag = 1;
		}
		if(key == 2){
			Robo[1].x = moved;
			Robo[1].Lflag = 1;
		}
		if(key == 3){
			Robo[1].y = moved;
			Robo[1].Uflag = 1;
		}
		if(key == 4){
			Robo[1].y = moved;
			Robo[1].Dflag = 1;
		}
		if(key == 5){
			Robo[1].x = moved;
		   Robo[1].Rflag = 0;
		}
		if(key == 6){
			Robo[1].x = moved;
			Robo[1].Lflag = 0;
		}
		if(key == 7){
			Robo[1].y = moved;
		   Robo[1].Uflag = 0;
		}
		if(key == 8){
			Robo[1].y = moved;
		   Robo[1].Dflag = 0;
		}
	}

}


//移動処理,メインループで使う,各フラグに対して座標を更新
void MoveRobo(void){
	int flag_atari = 0;
	int flag_robo0_atarix_right = 0;//p1のロボが相手とぶつかっているかの判定フラグ
	int flag_robo0_atarix_left = 0;//p1のロボが相手とぶつかっているかの判定フラグ
	int flag_robo0_atariy_up = 0;//p1のロボが相手とぶつかっているかの判定フラグ
	int flag_robo0_atariy_down = 0;//p1のロボが相手とぶつかっているかの判定フラグ
	int flag_robo1_atarix_right = 0;//p1のロボが相手とぶつかっているかの判定フラグ
	int flag_robo1_atarix_left = 0;//p1のロボが相手とぶつかっているかの判定フラグ
	int flag_robo1_atariy_up = 0;//p1のロボが相手とぶつかっているかの判定フラグ
	int flag_robo1_atariy_down = 0;//p1のロボが相手とぶつかっているかの判定フラグ
	flag_atari = Robo_calc();//まず重なっているか判断

	//ロボ0
	if(Robo[0].Rflag == 1){
		if(flag_atari == 0){
			Robo[0].x += Robo[0].speed;
		}
		if(flag_atari == 1){
			flag_robo0_atarix_right = Robo_calc_x_right(0);
			if(flag_robo0_atarix_right == 0){
				Robo[0].x += Robo[0].speed;
			}
			if(flag_robo0_atarix_right == 1){
				Robo[0].x -= Robo[0].speed * 2;
			}
		
		}
	}
	if( Robo[0].Lflag == 1){
		if(flag_atari == 0){
			Robo[0].x -= Robo[0].speed;
		}
		if(flag_atari == 1){
			flag_robo0_atarix_left = Robo_calc_x_left(0);
			if(flag_robo0_atarix_left == 0){
				Robo[0].x -= Robo[0].speed;
			}
			if(flag_robo0_atarix_left == 1){
				Robo[0].x += Robo[0].speed * 2;
			}
		
		}
	}
	if( Robo[0].Dflag == 1){
		if(flag_atari == 0){
			Robo[0].y += Robo[0].speed;
		}
		if(flag_atari == 1){
			flag_robo0_atariy_down = Robo_calc_y_down(0);
		if(flag_robo0_atariy_down == 0){
		Robo[0].y += Robo[0].speed;
		}
		if(flag_robo0_atariy_down == 1){
		Robo[0].y -= Robo[0].speed * 2;
		}
		}
	}
	if(Robo[0].Uflag == 1){
		if(flag_atari == 0){
			Robo[0].y -= Robo[0].speed;
		}
		if(flag_atari == 1){
			flag_robo0_atariy_up = Robo_calc_y_up(0);
		if(flag_robo0_atariy_up == 0){
		Robo[0].y -= Robo[0].speed;
		}
		if(flag_robo0_atariy_up == 1){
		Robo[0].y += Robo[0].speed * 2;
		}
		}
	}

	//ロボ1
	if(Robo[1].Rflag == 1){
		if(flag_atari == 0){
			Robo[1].x += Robo[1].speed;
		}
		if(flag_atari == 1){
			flag_robo1_atarix_right = Robo_calc_x_right(1);
			if(flag_robo1_atarix_right == 0){
				Robo[1].x += Robo[1].speed;
			}
			if(flag_robo1_atarix_right == 1){
				Robo[1].x -= Robo[1].speed * 2;
			}
		
		}
	}
	if( Robo[1].Lflag == 1){
		if(flag_atari == 0){
			Robo[1].x -= Robo[1].speed;
		}
		if(flag_atari == 1){
			flag_robo1_atarix_left = Robo_calc_x_left(1);
			if(flag_robo1_atarix_left == 0){
				Robo[1].x -= Robo[1].speed;
			}
			if(flag_robo1_atarix_left == 1){
				Robo[1].x += Robo[1].speed * 2;
			}
		
		}
		
	}
	if( Robo[1].Dflag == 1){
		if(flag_atari == 0){
			Robo[1].y += Robo[1].speed;
		}
		if(flag_atari == 1){
			flag_robo1_atariy_down = Robo_calc_y_down(1);
		if(flag_robo1_atariy_down == 0){
		Robo[1].y += Robo[1].speed;
		}
		if(flag_robo1_atariy_down == 1){
		Robo[1].y -= Robo[1].speed * 2;
		}
		}
	}
	if(Robo[1].Uflag == 1){
		if(flag_atari == 0){
			Robo[1].y -= Robo[1].speed;
		}
		if(flag_atari == 1){
			flag_robo1_atariy_up = Robo_calc_y_up(1);
		if(flag_robo1_atariy_up == 0){
		Robo[1].y -= Robo[1].speed;
		}
		if(flag_robo1_atariy_up == 1){
		Robo[1].y += Robo[1].speed * 2;
		}
		}
	}


}


//ダッシュ後のクールタイム
void cooltime(void){
		if(Robo[0].type == 2){
			Robo[0].Dcount -= 1;
			if(Robo[0].Dcount < 0){
				Robo[0].type= 0;
			}
		}
	if(Robo[1].type == 2){
			Robo[1].Dcount -= 1;
			if(Robo[1].Dcount < 0){
				Robo[1].type= 0;
			}
		}
}

/*爆発の画像のタイマー*/
void presenttime(void){
		if(bakudan_p0[0].flag == 2){
			bakudan_p0[0].excount -= 1;
			if(bakudan_p0[0].excount < 0){
				bakudan_p0[0].flag= 0;
			}
		}
	   if(bakudan_p1[0].flag == 2){
			bakudan_p1[0].excount -= 1;
			if(bakudan_p1[0].excount < 0){
				bakudan_p1[0].flag= 0;
			}

		}
}

//音楽鳴らす関数
void Playmusic(void){
	Mix_PlayMusic(music, -1); // BGMの再生（繰り返し再生）
}

//winflagを動悸する関数
void match_winflag(int wflag){
	winflag = wflag;
}