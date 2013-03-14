#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "conlib.h"
#include "game.h"
#include "z.h"

/* ��Ϸ����Ҫ�õ���һЩ���� */
#define ConKEY_O	0x4F
#define ConKEY_S	0x53
#define ConKEY_L	0x4C
#define ConKEY_R	0x52
#define ConKEY_R	0x52
#define ConKEY_M	0x4D


/* Ԥ����Ĵ��ڵĴ�С */
#define ConWidth 60
#define ConHeight 20

#define SNAKE_AREA_WIDTH 20 * 2
#define SNAKE_AREA_HEIGHT (ConHeight - 2)
#define SNAKE_AREA_OFFSET_X 2
#define SNAKE_AREA_OFFSET_Y 1
// �ߵ��ƶ��ٶ� ������ƶ�һ��
#define SNAKE_SPEED (500 - play_snake_length * 10)

// �浵�ļ���
#define SAVE_FILE_NAME "snake.sav"
/* �ߵ���󳤶� �����ж���Ϸʤ�� */
#define MAX_SNAKE_LENGTH 30
#define INIT_SNAKE_LENGTH 2

// Ԥ������
#define MenuSwitchBeep() beep(600,50)
#define SnakeMoveBeep() beep(1300,50)
#define FoodBeep() beep(800,50)

/* ===== ȫ�ֱ��� ===== */
/* ȫ����Ϸ״̬ */
enum GAME_STATE game_state = GAME_INIT;
enum GAME_STATE last_game_state;

const char* DoubleBorder[11] = 
{
 "�X","�j","�["
,"�d","�p","�g"
,"�^","�m","�a"
,"�T","�U"
};
const char* SingleBorder[11] = 
{
 "��","��","��"
,"��","��","��"
,"��","��","��"
,"��","��"
};
/* ȫ�ּ��������� */
const ConMouse* game_mouse;
const ConKey* game_key;

// play ״̬�Ĺ��ñ���
enum SNAKE_TOWARDS
{
	Toward_UP = 0
	,Toward_DOWN
	,Toward_LEFT
	,Toward_RIGHT
};
// ����ʳ��
void generate_food();

static int play_time;
static int play_snake_head_x;
static int play_snake_head_y;
static int play_snake_tail_x;
static int play_snake_tail_y;
static int play_snake_food_x;
static int play_snake_food_y;
static int play_snake_buff[SNAKE_AREA_HEIGHT][SNAKE_AREA_WIDTH];
static enum SNAKE_TOWARDS play_snake[MAX_SNAKE_LENGTH];
static int play_snake_head;
static int play_snake_tail;
static int play_snake_length;

/* �˵���ѡ�� */
static int mainmenu_choice;
// ʧ�ܶԻ����ѡ��
static int lost_choice;
// ȷ�϶Ի����ѡ��
static int backto_mainmenu_choice;
static int win_choice;

// ��ͷ��״
const char* SnakeHeadShape[4] = {"��","��","��","��"};
const char* SnakeNodeShape = "��";
const char* SnakeFoodShape = "��";
/* ======== ���� ========= */
/* �ı���Ϸ״̬ */
void change_game_state( GAME_STATE state)
{
	last_game_state = game_state;
	game_state = state;
}
/* ��Ϸ״̬�Ƿ񱻸ı� */
bool changed_game_state()
{
	return last_game_state != game_state;
}
/* ��ȡ��Ϸ״̬ */
const GAME_STATE get_game_state()
{
	return game_state;
}
/* ����ΪĬ����ɫ */
void set_default_color()
{
	set_text_color(ConWhite);
	set_background_color(ConBlack);
}

/* ���Ʊ߿� */
void draw_border(int sx, int sy, int w, int h, const char* border[])
{
	int x, y;
	int dx, dy;
	dx = w + sx;
	dy = h + sy;
	w += sx;
	h += sy;
	// ��
	for(x = sx; x < dx; x += 2)
	{
		gotoxy(x,sy);
		printf(border[9]);

		gotoxy(x,dy - 1);
		printf(border[9]);
	}
	// ��
	/* for(y = h - 1; y > sy; y --) */
	for(y = sy; y < dy; y ++)
	{
		gotoxy(sx, y);
		printf(border[10]);
		gotoxy(dx, y);
		printf(border[10]);
	}
	// �����ĸ���
	gotoxy(sx,sy);
	printf(border[0]);
	gotoxy(dx,sy);
	printf(border[2]);
	gotoxy(sx,dy-1);
	printf(border[6]);
	gotoxy(dx,dy-1);
	printf(border[8]);
}
void draw_dialog(int sx, int sy, int w, int h, const char* title, const char*contents, const char* border[])
{
	int x, y;
	int dx, dy;

	dx = sx + w, dy = sy + h;
	// �����Ƭ����
	for(y = sy;y < dy; y ++)
		for(x = sx;x < dx; x ++)
		{
			gotoxy(x, y);
			putchar(' ');
		}
	// ���Ʊ߿�
	draw_border(sx, sy, w, h, border);
	draw_border(sx, sy, w, 3, border);
	// �޲�����
	gotoxy(sx, sy + 3 - 1);
	printf(border[3]);
	gotoxy(sx + w, sy + 3 - 1);
	printf(border[5]);
	// ����
	gotoxy(sx + 4, sy + 1);
	printf(title);
	// ����
	gotoxy(sx + 4, sy + 3);
	printf(contents);
}
void draw_normal_dialog(const char* title, const char* content)
{
	draw_dialog(10, 4, 40, 10, title,content,SingleBorder);
}
/* ���������� !�����Զ����� */
void printa(int ox, int oy, int w,enum ConAlign mode, const char* str)
{
	int x, y;
	int start, end;
	int length = 0;
	char* line;
	bool need_print = false;

	y = oy;
	x = ox;
	line = zalloc(w + 1);
	start = end = 0;

	while(true)
	{
		/* �ַ���� */
		switch(str[end])
		{
			// �������н���һ�����
			case '\0':
			case '\n':
				need_print = true;
			break;
			case '\t':
				length += 4;
			/* break; */
			default:
				length ++;
			break;
		}/* end switch */

		/* ����ж� */
		if(need_print)
		{
			memcpy(line, str+start,length);
			// λ�õ���
			switch(mode)
			{
				case ConAlign_LEFT:
					x = ox;
				break;
				case ConAlign_CENTER:
					x = ox + (w - length)/2;
				break;
				case ConAlign_RIGHT:
					x = ox + w - length;
				break;
			}
			gotoxy(x, y);
			printf(line);

			line[0] = '\0';
			y ++;
			length = 0;
			start = end;
		}
		/* �˳�ѭ������ */
		if(str[end] == '\0')
			break;
		else
			end ++;
	}

}
/* ʳ������ */
void generate_food()
{
	while(true)
	{
		play_snake_food_x = rand() % (SNAKE_AREA_WIDTH/2);
		play_snake_food_y = rand() % SNAKE_AREA_HEIGHT;

		// �����ǰû�к�����ײ
		if(false == play_snake_buff[play_snake_food_y][play_snake_food_x])
			break;
	}
}

void process_init()
{
	/* ����������� */
	srand(time(NULL));
	/* ��ʼconlib�� */
	conlib_init();
	settitle("��Ϸ���ڳ�ʼ��...");

	hide_cursor();


	settitle(GAME_NAME);
	/* �л���Ϸ״̬ */
	change_game_state(GAME_MAINMENU);
}

void process_mainmenu()
{
	/* ��ʼ��״̬ */
	if(changed_game_state())
	{
		mainmenu_choice = 0;
		// ȡ�� changed
		change_game_state(GAME_MAINMENU);
	}
	/* �������� */
	if(game_key && game_key->state)
	switch(game_key->key)
	{
		case ConKEY_UP:
			mainmenu_choice -= 1;
		break;
		case ConKEY_DOWN:
			mainmenu_choice += 1;
		break;
		// �˳���Ϸ
		case ConKEY_ESCAPE:
			change_game_state(GAME_EXIT);
		break;
		/* ����س��¼� ѡ��˵� */
		case ConKEY_RETURN:
		{
			switch(mainmenu_choice)
			{
				case 0:
					change_game_state(GAME_START);
				break;
				case 1:
					change_game_state(GAME_LOAD);
				break;
				case 2:
					change_game_state(GAME_HELP);
				break;
				case 3:
					change_game_state(GAME_EXIT);
				break;
			}
		}
		break;
		default:

		break;
	}

	/* ѡ��ı߽��� */
	if(mainmenu_choice < 0)
		mainmenu_choice = 3;
	else if(mainmenu_choice > 3)
		mainmenu_choice = 0;
}
void process_lost()
{
	/* ��ʼ�� */
	if(changed_game_state())
	{
		lost_choice = 0;
		/* ȡ�� changed */
		change_game_state(GAME_LOST);
	}
	/* �������� */
	if(game_key && game_key->state)
	switch(game_key->key)
	{
		case ConKEY_LEFT:
		case ConKEY_RIGHT:
			// �Ի����ѡ��ֻ�� 0 1֮���л�
			lost_choice = 1 - lost_choice;
		break;
		case ConKEY_RETURN:
			if(0 == lost_choice)
				change_game_state(GAME_START);
			else
				change_game_state(GAME_MAINMENU);
		break;
	}
}
void process_pause()
{
	/* ��ʼ */
	if(changed_game_state())
	{
		/* ȡ��changed */
		change_game_state(GAME_PAUSE);
	}
	/* �������� */
	if(game_key && game_key->state)
	switch(game_key->key)
	{
		case ConKEY_SPACE:
			change_game_state(GAME_PLAY);
		break;
	}
}
void process_save()
{
	/* ��ʼ�� */
	if(changed_game_state())
	{
		/* ȡ�� changed */
		change_game_state(GAME_SAVE);
	}

	FILE* fp = fopen(SAVE_FILE_NAME,"w");
	if(fp == NULL)
	{
		clrscr();
		draw_normal_dialog("����ʧ��","��Ϸ����ʧ��,������Ϸ,�����������");
		pausei();
		change_game_state(GAME_PLAY);
	}
	fprintf(fp,"%d %d %d %d %d %d %d %d %d %d"
			,play_snake_head_x,play_snake_head_y
			,play_snake_tail_x,play_snake_tail_y
			,play_snake_food_x,play_snake_food_y
			,play_snake_head,play_snake_tail
			,play_snake_length,play_time);
	fwrite(play_snake_buff, sizeof(play_snake_buff),1,fp);
	fwrite(play_snake, sizeof(play_snake),1,fp);
	fclose(fp);

	change_game_state(GAME_PLAY);
}
void process_load()
{
	/* ��ʼ�� */
	if(changed_game_state())
	{
		/* ȡ�� changed */
		change_game_state(GAME_LOAD);
	}

	FILE* fp = fopen(SAVE_FILE_NAME,"r");
	if(fp == NULL)
	{
		clrscr();
		draw_normal_dialog("��Ϸ��ȡʧ��","��ȡʧ��,���������˵�,�����������");
		pausei();
		change_game_state(GAME_MAINMENU);
	}

	fscanf(fp,"%d %d %d %d %d %d %d %d %d %d"
			,&play_snake_head_x,&play_snake_head_y
			,&play_snake_tail_x,&play_snake_tail_y
			,&play_snake_food_x,&play_snake_food_y
			,&play_snake_head,&play_snake_tail
			,&play_snake_length,&play_time);
	fread(&play_snake_buff, sizeof(play_snake_buff),1,fp);
	fread(&play_snake, sizeof(play_snake),1,fp);
	fclose(fp);

	change_game_state(GAME_PLAY);
}
void process_backto_mainmenu()
{
	/* ��ʼ�� */
	if(changed_game_state())
	{
		backto_mainmenu_choice = 0;
		/* ȡ�� changed */
		change_game_state(GAME_BACKTO_MAINMENU);
	}
	/* �������� */
	if(game_key && game_key->state)
	switch(game_key->key)
	{
		case ConKEY_LEFT:
		case ConKEY_RIGHT:
			// �Ի����ѡ��ֻ�� 0 1֮���л�
			backto_mainmenu_choice = 1 - backto_mainmenu_choice;
		break;
		case ConKEY_RETURN:
			if(0 == backto_mainmenu_choice)
				change_game_state(GAME_PLAY);
			else
				change_game_state(GAME_MAINMENU);
		break;
	}
}
void process_win()
{
	/* ��ʼ�� */
	if(changed_game_state())
	{
		win_choice = 0;
		/* ȡ�� changed */
		change_game_state(GAME_WIN);
	}
	/* �������� */
	if(game_key && game_key->state)
	switch(game_key->key)
	{
		case ConKEY_LEFT:
		case ConKEY_RIGHT:
			// �Ի����ѡ��ֻ�� 0 1֮���л�
			win_choice = 1 - win_choice;
		break;
		case ConKEY_RETURN:
			if(0 == win_choice)
				change_game_state(GAME_START);
			else
				change_game_state(GAME_MAINMENU);
		break;
	}
}
void process_start()
{
	/* ��ʼ��Ϸ���� */
	play_snake_length = 2;
	play_time = 0;
	play_snake_head_x = 1;
	play_snake_head_y = 0;
	play_snake_tail_x = 0;
	play_snake_tail_y = 0;
	// ���buff
	memset(play_snake_buff, '\0', sizeof(play_snake_buff));
	play_snake_buff[0][0] = true;
	play_snake_buff[0][1] = true;
	play_snake[0] = Toward_RIGHT;
	play_snake[1] = Toward_RIGHT;
	play_snake_head = 1;
	play_snake_tail = 0;
	// ��ʼ����ʳ��
	generate_food();
	/* �л���play */
	change_game_state(GAME_PLAY);
}
void process_play()
{
	static int last_time = 0;
	static int last_clock = 0;
	int snake_node;
	time_t now;
	bool same_toward = false;
	// ��������ת�� �ж�ͷ����һ��
	snake_node = play_snake_head - 1;
	if(snake_node < 0)
		snake_node = 0;

	/* ��ʼ�� */
	if(changed_game_state())
	{
		// ����� ��Ϊ�ڳ�ʼ����ʱ��������tine ������ͣ��ʱ�򲻻�Ӱ��
		last_time = time(NULL);
		// �������
		last_clock = clock();

		/* ȡ�� changed */
		change_game_state(GAME_PLAY);
	}

	/* �������� */
	if(game_key && game_key->state)
	switch(game_key->key)
	{
		case ConKEY_UP:
			if(play_snake[snake_node] == Toward_UP)
				same_toward = true;
			else if(play_snake[snake_node] != Toward_DOWN)
				play_snake[play_snake_head] = Toward_UP;
		break;
		case ConKEY_RIGHT:
			if(play_snake[snake_node] == Toward_RIGHT)
				same_toward = true;
			else if(play_snake[snake_node] != Toward_LEFT)
				play_snake[play_snake_head] = Toward_RIGHT;
		break;
		case ConKEY_DOWN:
			if(play_snake[snake_node] == Toward_DOWN)
				same_toward = true;
			else if(play_snake[snake_node] != Toward_UP)
				play_snake[play_snake_head] = Toward_DOWN;
		break;
		case ConKEY_LEFT:
			if(play_snake[snake_node] == Toward_LEFT)
				same_toward = true;
			else if(play_snake[snake_node] != Toward_RIGHT)
				play_snake[play_snake_head] = Toward_LEFT;
		break;

		// ��ͣ���˳�
		case ConKEY_SPACE:
			change_game_state(GAME_PAUSE);
		break;
		case ConKEY_ESCAPE:
			change_game_state(GAME_BACKTO_MAINMENU);
		break;
		// ���¿�ʼ
		case ConKEY_R:
			change_game_state(GAME_START);
		break;
		// �����л�
		case ConKEY_M:
			beep_switch(!beep_state());
		break;

		// ����Ͷ�ȡ
		case ConKEY_S:
			change_game_state(GAME_SAVE);
		break;
		case ConKEY_L:
			change_game_state(GAME_LOAD);
		break;

		default:
		break;
	}// ������������

	/* �ƶ�����/��ײ�ж� */
	now = clock();
	if(same_toward || (now - last_clock) >= SNAKE_SPEED)
	{
		int x, y;
		x = play_snake_head_x;
		y = play_snake_head_y;
		switch(play_snake[play_snake_head])
		{
			case Toward_UP:
				y -= 1;
			break;
			case Toward_DOWN:
				y += 1;
			break;
			case Toward_LEFT:
				x -= 1;
			break;
			case Toward_RIGHT:
				x += 1;
			break;
		}
		
		play_snake_head_x = x;
		play_snake_head_y = y;
	
		// ��ײ���߽�
		if(x < 0 || x >= SNAKE_AREA_WIDTH/2 || y < 0 || y > SNAKE_AREA_HEIGHT)
		{
			change_game_state(GAME_LOST);
			return ;
		}
		// �Ե�ʳ�� û�гԵ�ʳ����ȥ��β��
		if(x == play_snake_food_x && y == play_snake_food_y)
		{
			play_snake_length += 1;
			generate_food();
		}else
		{
			play_snake_buff[play_snake_tail_y][play_snake_tail_x] = false;
			switch(play_snake[play_snake_tail])
			{
				case Toward_UP:
					play_snake_tail_y -= 1;
				break;
				case Toward_DOWN:
					play_snake_tail_y += 1;
				break;
				case Toward_LEFT:
					play_snake_tail_x -= 1;
				break;
				case Toward_RIGHT:
					play_snake_tail_x += 1;
				break;
			}
			snake_node = play_snake_tail + 1;
			if(snake_node >= MAX_SNAKE_LENGTH)
				snake_node = 0;
			play_snake_tail = snake_node;
		}
		
		// ������ײ
		if(true == play_snake_buff[y][x])
		{
			change_game_state(GAME_LOST);
			return;
		}else
		{
			play_snake_buff[y][x] = true;
			snake_node = play_snake_head + 1;
			if(snake_node >= MAX_SNAKE_LENGTH)
				snake_node = 0;
			play_snake[snake_node] = play_snake[play_snake_head];
			play_snake_head = snake_node;
		}

		// ʤ���ж�
		if(play_snake_length == MAX_SNAKE_LENGTH)
		{
			change_game_state(GAME_WIN);
		}
		// update
		last_clock = now;
	}// �����ƶ�����
	
	/* ʱ������ */
	now = time(NULL);
	if(now > last_time)
	{
		play_time += (now - last_time);
		last_time = now;
	}
	
}
void process_help()
{
	if(changed_game_state())
	{
		/* ȡ�� changed */
		change_game_state(GAME_HELP);
	}
	/* �������� */
	if(game_key && game_key->state)
	switch(game_key->key)
	{
		// �������˵�
		case ConKEY_ESCAPE:
			change_game_state(GAME_MAINMENU);
		break;
		// ����ҳ
		case ConKEY_O:
			system( "start " AUTHOR_WEB_SITE);
		break;

		default:

		break;
	}

}
/* �ܴ����� */
void process()
{
	game_key = getkey();
	game_mouse = getmouse();

	switch(game_state)
	{
		case GAME_INIT:
			process_init();
		break;
		case GAME_MAINMENU:
			process_mainmenu();
		break;
		case GAME_START:
			process_start();
		break;
		case GAME_PLAY:
			process_play();
		break;
		case GAME_LOST:
			process_lost();
		break;
		case GAME_WIN:
			process_win();
		break;
		case GAME_HELP:
			process_help();
		break;
		case GAME_PAUSE:
			process_pause();
		break;
		case GAME_SAVE:
			process_save();
		break;
		case GAME_LOAD:
			process_load();
		break;
		case GAME_BACKTO_MAINMENU:
			process_backto_mainmenu();
		break;
		default:
		break;
	}
}
void show_init()
{
}
void show_mainmenu()
{
	static int last_choice = -1;
	int starty = ConHeight / 3 + 2;// �����л��ֽ���
	int startx = (ConWidth-8) / 2;
	/* ��һ����Ϸ״̬�л�ʱ����������Ļ */
	if(changed_game_state())
	{
		/* ������һ��ѡ�� */
		last_choice = -1;
		
		/* ���Ʊ߿� */
		set_default_color();
		clrscr();

		draw_border(0,0,ConWidth,ConHeight, DoubleBorder);
		// �˵��߿� ��
		int x;
		for(x = ConWidth/2; x > 0; x --)
		{
			// �˵�����ķֽ���
			gotoxy(x*2,starty - 2);
			printf(DoubleBorder[9]);
		}
		// �˵�����ֽ��ߵ�����
		gotoxy(0,starty - 2);
		printf(DoubleBorder[3]);
		gotoxy(ConWidth,starty - 2);
		printf(DoubleBorder[5]);
		/* ��ʾ���� ���� */
		gotoxy((ConWidth - 10)/2,(starty - 2)/2);
		printf("%s v%d.%d", GAME_NAME, MAJOR_VERSION, SUB_VERSION);
		gotoxy((ConWidth - 10)/2 + 6,(starty - 2)/2 + 1);// ����һ��λ�ý���ƫ��
		printf("%s (%s)", AUTHOR_NAME, AUTHOR_EMAIL);
		/* ���Ʋ˵�ѡ�� */
		gotoxy(startx, starty);
		printf("��ʼ��Ϸ");

		gotoxy(startx, starty+1);
		printf("������Ϸ");

		gotoxy(startx, starty+2);
		printf("��Ϸ����");

		gotoxy(startx, starty+3);
		printf("�˳���Ϸ");
	}/* ������ʼҳ����� */

	/* ���Ƶ�ǰѡ�� */
	if(last_choice != mainmenu_choice)
	{
		MenuSwitchBeep();

		gotoxy(startx - 2, starty + last_choice);
		putchar(' ');
		gotoxy(startx + 9, starty + last_choice);
		putchar(' ');

		set_text_color(ConRed);
		gotoxy(startx - 2, starty + mainmenu_choice);
		printf(">");

		gotoxy(startx + 9, starty + mainmenu_choice);
		printf("<");

		last_choice = mainmenu_choice;
	}

}
void show_lost()
{
	static int last_choice = -1;
	/* ��ʼ�� */
	if(changed_game_state())
	{
		// ǿ�Ƹ���
		last_choice = -1;
		// ���ƶԻ���
		set_default_color();
		draw_dialog(10, 4, 40, 10, "��Ϸʧ��","ĩ����,��������������",SingleBorder);
		// ѡ��
		gotoxy( 10 + (40 - 12)/2, 4 + 10 - 2);
		printf(" ����  �˳� ");
	}// ��ɳ�ʼ��
	if(last_choice != lost_choice)
	{
		MenuSwitchBeep();

		set_text_color(ConRed);
		int startx = 10 + (40 - 12)/2;
		gotoxy( startx + 6 * last_choice, 4 + 10 - 2);
		putchar(' ');
		gotoxy( startx + 5 + 6 * last_choice, 4 + 10 - 2);
		putchar(' ');

		last_choice = lost_choice;
		gotoxy( startx + 6 * last_choice, 4 + 10 - 2);
		putchar('>');
		gotoxy( startx + 5 + 6 * last_choice, 4 + 10 - 2);
		putchar('<');
	}

}
void show_start()
{}
void show_help()
{
	/* ��һ�λ��Ƹ�ҳ�� */
	if(changed_game_state())
	{
		/* ���� */
		set_default_color();
		clrscr();

		draw_border(0,0,ConWidth,ConHeight, DoubleBorder);
		draw_border(0,0,ConWidth,3, DoubleBorder);

		// �޸�3�е���������
		gotoxy(0,2);
		printf(DoubleBorder[3]);
		gotoxy(ConWidth,2);
		printf(DoubleBorder[5]);

		// ��ʾ����
		gotoxy(4, 1);
		printf("%s v%d.%d\t", GAME_NAME, MAJOR_VERSION, SUB_VERSION);
		printf("%s (%s)", AUTHOR_NAME, AUTHOR_EMAIL);

		// ��ʾ��������
		int key_color = ConRed;
		gotoxy(4, 6);
		printf("����:");

		gotoxy(8, 7);
		set_text_color(key_color);
		printf("��-��-��-��");
		gotoxy(20, 7);
		set_text_color(ConWhite);
		printf("���Ʒ���/�˵�ѡ��");

		gotoxy(8, 8);
		set_text_color(key_color);
		printf("S");
		gotoxy(20, 8);
		set_text_color(ConWhite);
		printf("��Ϸʱ������Ϸ");

		gotoxy(8, 9);
		set_text_color(key_color);
		printf("<SPACE>");
		gotoxy(20, 9);
		set_text_color(ConWhite);
		printf("��Ϸʱ��ͣ��Ϸ��Ϸ");

		gotoxy(8, 10);
		set_text_color(key_color);
		printf("<ESC>");
		gotoxy(20, 10);
		set_text_color(ConWhite);
		printf("�˳���Ϸ");
		// ʤ������
		gotoxy(4, 12);
		printf("ʤ������:");
		gotoxy(8, 13);
		printf("���ߵĳ��ȴﵽ[");
		set_text_color(key_color);
		printf(" %d ", MAX_SNAKE_LENGTH);
		set_text_color(ConWhite);
		printf("]ʱ�������Ϸʤ��");

		gotoxy(4, 15);
		printf("<ESC>:�������˵� O:���ҵ���ҳ");
		
		printa(0,17,ConWidth,ConAlign_CENTER,"Enjoy It !");
	}
}

void show_play()
{
	// ��¼��Ϣ ����иı������»��� �ռ任ʱ��İ취
	static int snake_head = -1;
	static int last_head_x = -1;
	static int last_head_y = -1;
	static int last_tail_x = -1;
	static int last_tail_y = -1;
	static int last_food_x = -1;
	static int last_food_y = -1;
	static int last_length = -1;
	static int last_time = -1;
	static enum SNAKE_TOWARDS snake_toward = -1;
	/* ��һ�λ��Ƹ�ҳ�� */
	if(changed_game_state())
	{
		/* ���ƽ����� */
		set_default_color();
		clrscr();
		
		draw_border(0,0,ConWidth,ConHeight,DoubleBorder);
		// ��Ϣ�߿�
		// ����� + 2����Ϊ���߿������
		draw_border(SNAKE_AREA_WIDTH + 2,0,ConWidth - 2 - SNAKE_AREA_WIDTH,ConHeight,DoubleBorder);
		// �޸ı߽�
		gotoxy(SNAKE_AREA_WIDTH + 2,0);
		printf(DoubleBorder[1]);
		gotoxy(SNAKE_AREA_WIDTH + 2,ConHeight - 1);
		printf(DoubleBorder[7]);
		// ��Ϣ ����
		printa(SNAKE_AREA_WIDTH + 4, 2, 16, ConAlign_CENTER,"��Ϸ�÷�");
		printa(SNAKE_AREA_WIDTH + 4, 5, 16, ConAlign_CENTER,"��Ϸʱ��");

		gotoxy(SNAKE_AREA_WIDTH + 12, 3);
		printf("/%d", MAX_SNAKE_LENGTH);

		printa(SNAKE_AREA_WIDTH + 4, 8, 16, ConAlign_LEFT,"<R>���¿�ʼ");
		printa(SNAKE_AREA_WIDTH + 4, 9, 16, ConAlign_LEFT,"<S>��Ϸ����");
		printa(SNAKE_AREA_WIDTH + 4, 10, 16, ConAlign_LEFT,"<L>��Ϸ��ȡ");
		printa(SNAKE_AREA_WIDTH + 4, 11, 16, ConAlign_LEFT,"<�ո�>��ͣ");
		printa(SNAKE_AREA_WIDTH + 4, 12, 16, ConAlign_LEFT,"<ESC>�˳���Ϸ");

		printa(SNAKE_AREA_WIDTH + 4, 14, 16, ConAlign_LEFT,"<M>��������");

		/* ��ʼ��ʱ������� */
		int x, y;
		for(y = 0; y < SNAKE_AREA_HEIGHT; y ++)
			for(x = 0; x < SNAKE_AREA_WIDTH; x++)
				if(play_snake_buff[y][x])
				{
					gotoxy(x*2+SNAKE_AREA_OFFSET_X,y+SNAKE_AREA_OFFSET_Y);
					printf(SnakeNodeShape);
				}
		/* ��ʼ���� */
		last_length = play_snake_length;
		last_head_x = play_snake_head_x;
		last_head_y = play_snake_head_y;
		last_tail_x = play_snake_tail_x;
		last_tail_y = play_snake_tail_y;
		snake_head = play_snake_head;

		last_food_x = -1;
		last_food_y = -1;

		// ǿ��������и���
		last_length = -1;
		last_time = -1;
		snake_toward = -1;
	}// ������ʼ����

	/* �����ƶ� */
	if(snake_head != play_snake_head)
	{

		// �������δ������ϴ�βλ��
		if(last_length == play_snake_length)
		{
			gotoxy(last_tail_x*2+SNAKE_AREA_OFFSET_X,last_tail_y+SNAKE_AREA_OFFSET_Y);
			printf("  ");// �����������ո�
			
			// �ƶ�������
			SnakeMoveBeep();
			// ������������� �Ͳ��ųԵ�ʳ�������
		}else
			FoodBeep();

		// ���ϴ���ͷλ�û��Ƴ���ͨ�ڵ�
		gotoxy(last_head_x*2+SNAKE_AREA_OFFSET_X,last_head_y+SNAKE_AREA_OFFSET_Y);
		printf(SnakeNodeShape);

		// ǿ�����������ͷ
		snake_toward = -1;
		
		// update
		last_head_x = play_snake_head_x;
		last_head_y = play_snake_head_y;
		last_tail_x = play_snake_tail_x;
		last_tail_y = play_snake_tail_y;
		snake_head = play_snake_head;
	}// �����ƶ�����

	/* ����ͷ����ĸı� */
	if(play_snake[play_snake_head] != snake_toward)
	{
		snake_toward = play_snake[play_snake_head];
		gotoxy(play_snake_head_x*2+SNAKE_AREA_OFFSET_X,play_snake_head_y+SNAKE_AREA_OFFSET_Y);
		printf(SnakeHeadShape[snake_toward]);
	}// ����ͷ�������

	// ����ʳ����ʾ
	if(last_food_x != play_snake_food_x || last_food_y != play_snake_food_y)
	{
		last_food_x = play_snake_food_x;
		last_food_y = play_snake_food_y;
		// ʳ�ﲻ��Ҫ����ϴ���ʾ ֻ��Ҫ�ӻ�Ϳ�����
		gotoxy(last_food_x*2+SNAKE_AREA_OFFSET_X,last_food_y+SNAKE_AREA_OFFSET_Y);
		printf(SnakeFoodShape);
	}

	// ��ⳤ��
	if(last_length != play_snake_length)
	{
		last_length = play_snake_length;
		gotoxy(SNAKE_AREA_WIDTH + 8, 3);
		printf("%3d",last_length);
	}
	// ���ʱ��
	if(last_time != play_time)
	{
		last_time = play_time;
		gotoxy(SNAKE_AREA_WIDTH + 4, 6);
		printf("%6dms",last_time);
	}

}
void show_pause()
{
	/* ��ʼ���� */
	if(changed_game_state())
	{
		// ���ƶԻ���
		set_default_color();
		clrscr();
		draw_dialog(10, 4, 40, 10, "��Ϸ��ͣ","��<�ո�>��������Ϸ",SingleBorder);
	}
}
void show_save()
{
	/* ��ʼ���� */
	if(changed_game_state())
	{
		// ���ƶԻ���
		set_default_color();
		clrscr();
		draw_dialog(10, 4, 40, 10, "������Ϸ","��Ϸ���ڱ���,���Ե�...",SingleBorder);
	}
}
void show_load()
{
	/* ��ʼ���� */
	if(changed_game_state())
	{
		// ���ƶԻ���
		set_default_color();
		clrscr();
		draw_dialog(10, 4, 40, 10, "��ȡ��Ϸ","���ڶ�ȡ��Ϸ,���Ե�...",SingleBorder);
	}
}
void show_backto_mainmenu()
{
	static int last_choice = -1;
	/* ��ʼ�� */
	if(changed_game_state())
	{
		// ǿ�Ƹ���
		last_choice = -1;
		// ���ƶԻ���
		set_default_color();
		draw_dialog(10, 4, 40, 10, "�˳���Ϸ","ȷ�ϲ�������Ϸ�������˵�ô?",SingleBorder);
		// ѡ��
		gotoxy( 10 + (40 - 12)/2, 4 + 10 - 2);
		printf(" ����  �˳� ");
	}// ��ɳ�ʼ��
	if(last_choice != backto_mainmenu_choice)
	{
		MenuSwitchBeep();

		set_text_color(ConRed);
		int startx = 10 + (40 - 12)/2;
		gotoxy( startx + 6 * last_choice, 4 + 10 - 2);
		putchar(' ');
		gotoxy( startx + 5 + 6 * last_choice, 4 + 10 - 2);
		putchar(' ');

		last_choice = backto_mainmenu_choice;
		gotoxy( startx + 6 * last_choice, 4 + 10 - 2);
		putchar('>');
		gotoxy( startx + 5 + 6 * last_choice, 4 + 10 - 2);
		putchar('<');
	}
}
void show_win()
{
	static int last_choice = -1;
	/* ��ʼ�� */
	if(changed_game_state())
	{
		// ǿ�Ƹ���
		last_choice = -1;
		// ���ƶԻ���
		set_default_color();
		draw_dialog(10, 4, 40, 10, "��Ϸʤ��","������������,����һ��ô?",SingleBorder);
		// ѡ��
		gotoxy( 10 + (40 - 12)/2, 4 + 10 - 2);
		printf(" ����  �˳� ");
	}// ��ɳ�ʼ��
	if(last_choice != win_choice)
	{
		MenuSwitchBeep();

		set_text_color(ConRed);
		int startx = 10 + (40 - 12)/2;
		gotoxy( startx + 6 * last_choice, 4 + 10 - 2);
		putchar(' ');
		gotoxy( startx + 5 + 6 * last_choice, 4 + 10 - 2);
		putchar(' ');

		last_choice = win_choice;
		gotoxy( startx + 6 * last_choice, 4 + 10 - 2);
		putchar('>');
		gotoxy( startx + 5 + 6 * last_choice, 4 + 10 - 2);
		putchar('<');
	}
}
/* ����ʾ���� */
void show()
{
	switch(game_state)
	{
		case GAME_INIT:
			show_init();
		break;
		case GAME_MAINMENU:
			show_mainmenu();
		break;
		case GAME_START:
			show_start();
		break;
		case GAME_PLAY:
			show_play();
		break;
		case GAME_HELP:
			show_help();
		break;
		case GAME_LOST:
			show_lost();
		break;
		case GAME_WIN:
			show_win();
		break;
		case GAME_PAUSE:
			show_pause();
		break;
		case GAME_SAVE:
			show_save();
		break;
		case GAME_LOAD:
			show_load();
		break;
		case GAME_BACKTO_MAINMENU:
			show_backto_mainmenu();
		break;
		default:
		break;
	}

}
