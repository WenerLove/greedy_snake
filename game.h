#include <stdbool.h>
#ifndef GAME_H
	#define GAME_H

#define AUTHOR_NAME		"Wener"
#define AUTHOR_EMAIL	"wenermail@gmail.com"
#define AUTHOR_WEB_SITE	"http://blog.wener.me"

#define MAJOR_VERSION 1
#define SUB_VERSION 0

#define GAME_NAME	"贪吃蛇"

enum ConAlign
{
	ConAlign_LEFT
	,ConAlign_RIGHT
	,ConAlign_CENTER
};

/* 游戏状态 */
typedef enum GAME_STATE
{
	GAME_INIT
	,GAME_MAINMENU
	,GAME_START // 开始游戏 进行游戏初始化
	,GAME_LOST
	,GAME_WIN
	,GAME_HELP
	,GAME_LOAD // 加载游戏后 直接进入PLAY状态 加载失败返回
	,GAME_EXIT
	,GAME_PAUSE
	,GAME_SAVE
	,GAME_PLAY // 正在游戏的状态
	,GAME_BACKTO_MAINMENU // play的时候返回主菜单确认
} GAME_STATE;
/* 改变游戏状态 */
void chang_game_state( GAME_STATE state);
/* 游戏状态是否被改变 */
bool changed_game_state();
/* 获取游戏状态 */
const GAME_STATE get_game_state();

void process();
void show();
#endif
