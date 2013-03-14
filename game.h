#include <stdbool.h>
#ifndef GAME_H
	#define GAME_H

#define AUTHOR_NAME		"Wener"
#define AUTHOR_EMAIL	"wenermail@gmail.com"
#define AUTHOR_WEB_SITE	"http://blog.wener.me"

#define MAJOR_VERSION 1
#define SUB_VERSION 0

#define GAME_NAME	"̰����"

enum ConAlign
{
	ConAlign_LEFT
	,ConAlign_RIGHT
	,ConAlign_CENTER
};

/* ��Ϸ״̬ */
typedef enum GAME_STATE
{
	GAME_INIT
	,GAME_MAINMENU
	,GAME_START // ��ʼ��Ϸ ������Ϸ��ʼ��
	,GAME_LOST
	,GAME_WIN
	,GAME_HELP
	,GAME_LOAD // ������Ϸ�� ֱ�ӽ���PLAY״̬ ����ʧ�ܷ���
	,GAME_EXIT
	,GAME_PAUSE
	,GAME_SAVE
	,GAME_PLAY // ������Ϸ��״̬
	,GAME_BACKTO_MAINMENU // play��ʱ�򷵻����˵�ȷ��
} GAME_STATE;
/* �ı���Ϸ״̬ */
void chang_game_state( GAME_STATE state);
/* ��Ϸ״̬�Ƿ񱻸ı� */
bool changed_game_state();
/* ��ȡ��Ϸ״̬ */
const GAME_STATE get_game_state();

void process();
void show();
#endif
