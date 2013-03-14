#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "conlib.h"
#include "game.h"
#include "z.h"

/* 游戏里需要用到的一些按键 */
#define ConKEY_O	0x4F
#define ConKEY_S	0x53
#define ConKEY_L	0x4C
#define ConKEY_R	0x52
#define ConKEY_R	0x52
#define ConKEY_M	0x4D


/* 预定义的窗口的大小 */
#define ConWidth 60
#define ConHeight 20

#define SNAKE_AREA_WIDTH 20 * 2
#define SNAKE_AREA_HEIGHT (ConHeight - 2)
#define SNAKE_AREA_OFFSET_X 2
#define SNAKE_AREA_OFFSET_Y 1
// 蛇的移动速度 即多久移动一次
#define SNAKE_SPEED (500 - play_snake_length * 10)

// 存档文件名
#define SAVE_FILE_NAME "snake.sav"
/* 蛇的最大长度 用来判断游戏胜利 */
#define MAX_SNAKE_LENGTH 30
#define INIT_SNAKE_LENGTH 2

// 预设声音
#define MenuSwitchBeep() beep(600,50)
#define SnakeMoveBeep() beep(1300,50)
#define FoodBeep() beep(800,50)

/* ===== 全局变量 ===== */
/* 全局游戏状态 */
enum GAME_STATE game_state = GAME_INIT;
enum GAME_STATE last_game_state;

const char* DoubleBorder[11] = 
{
 "╔","╦","╗"
,"╠","╬","╣"
,"╚","╩","╝"
,"═","║"
};
const char* SingleBorder[11] = 
{
 "┌","┬","┐"
,"├","┼","┤"
,"└","┴","┘"
,"─","│"
};
/* 全局键盘鼠标控制 */
const ConMouse* game_mouse;
const ConKey* game_key;

// play 状态的共用变量
enum SNAKE_TOWARDS
{
	Toward_UP = 0
	,Toward_DOWN
	,Toward_LEFT
	,Toward_RIGHT
};
// 生成食物
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

/* 菜单的选择 */
static int mainmenu_choice;
// 失败对话框的选择
static int lost_choice;
// 确认对话框的选择
static int backto_mainmenu_choice;
static int win_choice;

// 蛇头形状
const char* SnakeHeadShape[4] = {"∧","∨","﹤","﹥"};
const char* SnakeNodeShape = "●";
const char* SnakeFoodShape = "◎";
/* ======== 函数 ========= */
/* 改变游戏状态 */
void change_game_state( GAME_STATE state)
{
	last_game_state = game_state;
	game_state = state;
}
/* 游戏状态是否被改变 */
bool changed_game_state()
{
	return last_game_state != game_state;
}
/* 获取游戏状态 */
const GAME_STATE get_game_state()
{
	return game_state;
}
/* 设置为默认颜色 */
void set_default_color()
{
	set_text_color(ConWhite);
	set_background_color(ConBlack);
}

/* 绘制边框 */
void draw_border(int sx, int sy, int w, int h, const char* border[])
{
	int x, y;
	int dx, dy;
	dx = w + sx;
	dy = h + sy;
	w += sx;
	h += sy;
	// 横
	for(x = sx; x < dx; x += 2)
	{
		gotoxy(x,sy);
		printf(border[9]);

		gotoxy(x,dy - 1);
		printf(border[9]);
	}
	// 竖
	/* for(y = h - 1; y > sy; y --) */
	for(y = sy; y < dy; y ++)
	{
		gotoxy(sx, y);
		printf(border[10]);
		gotoxy(dx, y);
		printf(border[10]);
	}
	// 绘制四个角
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
	// 清空这片区域
	for(y = sy;y < dy; y ++)
		for(x = sx;x < dx; x ++)
		{
			gotoxy(x, y);
			putchar(' ');
		}
	// 绘制边框
	draw_border(sx, sy, w, h, border);
	draw_border(sx, sy, w, 3, border);
	// 修补交界
	gotoxy(sx, sy + 3 - 1);
	printf(border[3]);
	gotoxy(sx + w, sy + 3 - 1);
	printf(border[5]);
	// 标题
	gotoxy(sx + 4, sy + 1);
	printf(title);
	// 内容
	gotoxy(sx + 4, sy + 3);
	printf(contents);
}
void draw_normal_dialog(const char* title, const char* content)
{
	draw_dialog(10, 4, 40, 10, title,content,SingleBorder);
}
/* 带对齐的输出 !不会自动换行 */
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
		/* 字符检测 */
		switch(str[end])
		{
			// 遇到换行进行一次输出
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

		/* 输出判断 */
		if(need_print)
		{
			memcpy(line, str+start,length);
			// 位置调整
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
		/* 退出循环测试 */
		if(str[end] == '\0')
			break;
		else
			end ++;
	}

}
/* 食物生成 */
void generate_food()
{
	while(true)
	{
		play_snake_food_x = rand() % (SNAKE_AREA_WIDTH/2);
		play_snake_food_y = rand() % SNAKE_AREA_HEIGHT;

		// 如果当前没有和蛇碰撞
		if(false == play_snake_buff[play_snake_food_y][play_snake_food_x])
			break;
	}
}

void process_init()
{
	/* 重置随机种子 */
	srand(time(NULL));
	/* 初始conlib库 */
	conlib_init();
	settitle("游戏正在初始化...");

	hide_cursor();


	settitle(GAME_NAME);
	/* 切换游戏状态 */
	change_game_state(GAME_MAINMENU);
}

void process_mainmenu()
{
	/* 初始该状态 */
	if(changed_game_state())
	{
		mainmenu_choice = 0;
		// 取消 changed
		change_game_state(GAME_MAINMENU);
	}
	/* 按键处理 */
	if(game_key && game_key->state)
	switch(game_key->key)
	{
		case ConKEY_UP:
			mainmenu_choice -= 1;
		break;
		case ConKEY_DOWN:
			mainmenu_choice += 1;
		break;
		// 退出游戏
		case ConKEY_ESCAPE:
			change_game_state(GAME_EXIT);
		break;
		/* 处理回车事件 选择菜单 */
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

	/* 选项的边界检测 */
	if(mainmenu_choice < 0)
		mainmenu_choice = 3;
	else if(mainmenu_choice > 3)
		mainmenu_choice = 0;
}
void process_lost()
{
	/* 初始化 */
	if(changed_game_state())
	{
		lost_choice = 0;
		/* 取消 changed */
		change_game_state(GAME_LOST);
	}
	/* 按键处理 */
	if(game_key && game_key->state)
	switch(game_key->key)
	{
		case ConKEY_LEFT:
		case ConKEY_RIGHT:
			// 对话框的选项只在 0 1之间切换
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
	/* 初始 */
	if(changed_game_state())
	{
		/* 取消changed */
		change_game_state(GAME_PAUSE);
	}
	/* 按键处理 */
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
	/* 初始化 */
	if(changed_game_state())
	{
		/* 取消 changed */
		change_game_state(GAME_SAVE);
	}

	FILE* fp = fopen(SAVE_FILE_NAME,"w");
	if(fp == NULL)
	{
		clrscr();
		draw_normal_dialog("保存失败","游戏保存失败,继续游戏,按任意键继续");
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
	/* 初始化 */
	if(changed_game_state())
	{
		/* 取消 changed */
		change_game_state(GAME_LOAD);
	}

	FILE* fp = fopen(SAVE_FILE_NAME,"r");
	if(fp == NULL)
	{
		clrscr();
		draw_normal_dialog("游戏读取失败","读取失败,将返回主菜单,按任意键继续");
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
	/* 初始化 */
	if(changed_game_state())
	{
		backto_mainmenu_choice = 0;
		/* 取消 changed */
		change_game_state(GAME_BACKTO_MAINMENU);
	}
	/* 按键处理 */
	if(game_key && game_key->state)
	switch(game_key->key)
	{
		case ConKEY_LEFT:
		case ConKEY_RIGHT:
			// 对话框的选项只在 0 1之间切换
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
	/* 初始化 */
	if(changed_game_state())
	{
		win_choice = 0;
		/* 取消 changed */
		change_game_state(GAME_WIN);
	}
	/* 按键处理 */
	if(game_key && game_key->state)
	switch(game_key->key)
	{
		case ConKEY_LEFT:
		case ConKEY_RIGHT:
			// 对话框的选项只在 0 1之间切换
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
	/* 初始游戏数据 */
	play_snake_length = 2;
	play_time = 0;
	play_snake_head_x = 1;
	play_snake_head_y = 0;
	play_snake_tail_x = 0;
	play_snake_tail_y = 0;
	// 清除buff
	memset(play_snake_buff, '\0', sizeof(play_snake_buff));
	play_snake_buff[0][0] = true;
	play_snake_buff[0][1] = true;
	play_snake[0] = Toward_RIGHT;
	play_snake[1] = Toward_RIGHT;
	play_snake_head = 1;
	play_snake_tail = 0;
	// 初始生成食物
	generate_food();
	/* 切换到play */
	change_game_state(GAME_PLAY);
}
void process_play()
{
	static int last_time = 0;
	static int last_clock = 0;
	int snake_node;
	time_t now;
	bool same_toward = false;
	// 不能往后转向 判断头后面一个
	snake_node = play_snake_head - 1;
	if(snake_node < 0)
		snake_node = 0;

	/* 初始化 */
	if(changed_game_state())
	{
		// 按秒计 因为在初始化的时候重置了tine 所以暂停的时候不会影响
		last_time = time(NULL);
		// 按毫秒计
		last_clock = clock();

		/* 取消 changed */
		change_game_state(GAME_PLAY);
	}

	/* 按键处理 */
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

		// 暂停和退出
		case ConKEY_SPACE:
			change_game_state(GAME_PAUSE);
		break;
		case ConKEY_ESCAPE:
			change_game_state(GAME_BACKTO_MAINMENU);
		break;
		// 重新开始
		case ConKEY_R:
			change_game_state(GAME_START);
		break;
		// 声音切换
		case ConKEY_M:
			beep_switch(!beep_state());
		break;

		// 保存和读取
		case ConKEY_S:
			change_game_state(GAME_SAVE);
		break;
		case ConKEY_L:
			change_game_state(GAME_LOAD);
		break;

		default:
		break;
	}// 结束按键处理

	/* 移动处理/碰撞判断 */
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
	
		// 碰撞到边界
		if(x < 0 || x >= SNAKE_AREA_WIDTH/2 || y < 0 || y > SNAKE_AREA_HEIGHT)
		{
			change_game_state(GAME_LOST);
			return ;
		}
		// 吃到食物 没有吃到食物则去掉尾部
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
		
		// 自身碰撞
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

		// 胜利判断
		if(play_snake_length == MAX_SNAKE_LENGTH)
		{
			change_game_state(GAME_WIN);
		}
		// update
		last_clock = now;
	}// 结束移动操作
	
	/* 时间增加 */
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
		/* 取消 changed */
		change_game_state(GAME_HELP);
	}
	/* 按键处理 */
	if(game_key && game_key->state)
	switch(game_key->key)
	{
		// 返回主菜单
		case ConKEY_ESCAPE:
			change_game_state(GAME_MAINMENU);
		break;
		// 打开网页
		case ConKEY_O:
			system( "start " AUTHOR_WEB_SITE);
		break;

		default:

		break;
	}

}
/* 总处理函数 */
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
	int starty = ConHeight / 3 + 2;// 留两行画分界线
	int startx = (ConWidth-8) / 2;
	/* 第一次游戏状态切换时绘制整个屏幕 */
	if(changed_game_state())
	{
		/* 重置上一次选项 */
		last_choice = -1;
		
		/* 绘制边框 */
		set_default_color();
		clrscr();

		draw_border(0,0,ConWidth,ConHeight, DoubleBorder);
		// 菜单边框 横
		int x;
		for(x = ConWidth/2; x > 0; x --)
		{
			// 菜单上面的分界线
			gotoxy(x*2,starty - 2);
			printf(DoubleBorder[9]);
		}
		// 菜单上面分界线的两边
		gotoxy(0,starty - 2);
		printf(DoubleBorder[3]);
		gotoxy(ConWidth,starty - 2);
		printf(DoubleBorder[5]);
		/* 显示标题 作者 */
		gotoxy((ConWidth - 10)/2,(starty - 2)/2);
		printf("%s v%d.%d", GAME_NAME, MAJOR_VERSION, SUB_VERSION);
		gotoxy((ConWidth - 10)/2 + 6,(starty - 2)/2 + 1);// 对上一个位置进行偏移
		printf("%s (%s)", AUTHOR_NAME, AUTHOR_EMAIL);
		/* 绘制菜单选项 */
		gotoxy(startx, starty);
		printf("开始游戏");

		gotoxy(startx, starty+1);
		printf("继续游戏");

		gotoxy(startx, starty+2);
		printf("游戏帮助");

		gotoxy(startx, starty+3);
		printf("退出游戏");
	}/* 结束初始页面绘制 */

	/* 绘制当前选中 */
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
	/* 初始化 */
	if(changed_game_state())
	{
		// 强制更新
		last_choice = -1;
		// 绘制对话框
		set_default_color();
		draw_dialog(10, 4, 40, 10, "游戏失败","末灰心,少侠请重新来过",SingleBorder);
		// 选项
		gotoxy( 10 + (40 - 12)/2, 4 + 10 - 2);
		printf(" 重来  退出 ");
	}// 完成初始化
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
	/* 第一次绘制该页面 */
	if(changed_game_state())
	{
		/* 绘制 */
		set_default_color();
		clrscr();

		draw_border(0,0,ConWidth,ConHeight, DoubleBorder);
		draw_border(0,0,ConWidth,3, DoubleBorder);

		// 修改3行的左右两个
		gotoxy(0,2);
		printf(DoubleBorder[3]);
		gotoxy(ConWidth,2);
		printf(DoubleBorder[5]);

		// 显示标题
		gotoxy(4, 1);
		printf("%s v%d.%d\t", GAME_NAME, MAJOR_VERSION, SUB_VERSION);
		printf("%s (%s)", AUTHOR_NAME, AUTHOR_EMAIL);

		// 显示帮助文字
		int key_color = ConRed;
		gotoxy(4, 6);
		printf("按键:");

		gotoxy(8, 7);
		set_text_color(key_color);
		printf("上-下-左-右");
		gotoxy(20, 7);
		set_text_color(ConWhite);
		printf("控制方向/菜单选择");

		gotoxy(8, 8);
		set_text_color(key_color);
		printf("S");
		gotoxy(20, 8);
		set_text_color(ConWhite);
		printf("游戏时保存游戏");

		gotoxy(8, 9);
		set_text_color(key_color);
		printf("<SPACE>");
		gotoxy(20, 9);
		set_text_color(ConWhite);
		printf("游戏时暂停游戏游戏");

		gotoxy(8, 10);
		set_text_color(key_color);
		printf("<ESC>");
		gotoxy(20, 10);
		set_text_color(ConWhite);
		printf("退出游戏");
		// 胜利条件
		gotoxy(4, 12);
		printf("胜利条件:");
		gotoxy(8, 13);
		printf("当蛇的长度达到[");
		set_text_color(key_color);
		printf(" %d ", MAX_SNAKE_LENGTH);
		set_text_color(ConWhite);
		printf("]时即获得游戏胜利");

		gotoxy(4, 15);
		printf("<ESC>:返回主菜单 O:打开我的主页");
		
		printa(0,17,ConWidth,ConAlign_CENTER,"Enjoy It !");
	}
}

void show_play()
{
	// 记录信息 如果有改变则重新绘制 空间换时间的办法
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
	/* 第一次绘制该页面 */
	if(changed_game_state())
	{
		/* 绘制界面框架 */
		set_default_color();
		clrscr();
		
		draw_border(0,0,ConWidth,ConHeight,DoubleBorder);
		// 信息边框
		// 这里的 + 2是因为主边框最左边
		draw_border(SNAKE_AREA_WIDTH + 2,0,ConWidth - 2 - SNAKE_AREA_WIDTH,ConHeight,DoubleBorder);
		// 修改边角
		gotoxy(SNAKE_AREA_WIDTH + 2,0);
		printf(DoubleBorder[1]);
		gotoxy(SNAKE_AREA_WIDTH + 2,ConHeight - 1);
		printf(DoubleBorder[7]);
		// 信息 标题
		printa(SNAKE_AREA_WIDTH + 4, 2, 16, ConAlign_CENTER,"游戏得分");
		printa(SNAKE_AREA_WIDTH + 4, 5, 16, ConAlign_CENTER,"游戏时间");

		gotoxy(SNAKE_AREA_WIDTH + 12, 3);
		printf("/%d", MAX_SNAKE_LENGTH);

		printa(SNAKE_AREA_WIDTH + 4, 8, 16, ConAlign_LEFT,"<R>重新开始");
		printa(SNAKE_AREA_WIDTH + 4, 9, 16, ConAlign_LEFT,"<S>游戏保存");
		printa(SNAKE_AREA_WIDTH + 4, 10, 16, ConAlign_LEFT,"<L>游戏读取");
		printa(SNAKE_AREA_WIDTH + 4, 11, 16, ConAlign_LEFT,"<空格>暂停");
		printa(SNAKE_AREA_WIDTH + 4, 12, 16, ConAlign_LEFT,"<ESC>退出游戏");

		printa(SNAKE_AREA_WIDTH + 4, 14, 16, ConAlign_LEFT,"<M>声音开关");

		/* 初始的时候绘制蛇 */
		int x, y;
		for(y = 0; y < SNAKE_AREA_HEIGHT; y ++)
			for(x = 0; x < SNAKE_AREA_WIDTH; x++)
				if(play_snake_buff[y][x])
				{
					gotoxy(x*2+SNAKE_AREA_OFFSET_X,y+SNAKE_AREA_OFFSET_Y);
					printf(SnakeNodeShape);
				}
		/* 初始数据 */
		last_length = play_snake_length;
		last_head_x = play_snake_head_x;
		last_head_y = play_snake_head_y;
		last_tail_x = play_snake_tail_x;
		last_tail_y = play_snake_tail_y;
		snake_head = play_snake_head;

		last_food_x = -1;
		last_food_y = -1;

		// 强制下面进行更新
		last_length = -1;
		last_time = -1;
		snake_toward = -1;
	}// 结束初始操作

	/* 绘制移动 */
	if(snake_head != play_snake_head)
	{

		// 如果长度未变清除上次尾位置
		if(last_length == play_snake_length)
		{
			gotoxy(last_tail_x*2+SNAKE_AREA_OFFSET_X,last_tail_y+SNAKE_AREA_OFFSET_Y);
			printf("  ");// 这里是两个空格
			
			// 移动的声音
			SnakeMoveBeep();
			// 如果长度增长了 就播放吃到食物的声音
		}else
			FoodBeep();

		// 把上次蛇头位置绘制成普通节点
		gotoxy(last_head_x*2+SNAKE_AREA_OFFSET_X,last_head_y+SNAKE_AREA_OFFSET_Y);
		printf(SnakeNodeShape);

		// 强制下面更新蛇头
		snake_toward = -1;
		
		// update
		last_head_x = play_snake_head_x;
		last_head_y = play_snake_head_y;
		last_tail_x = play_snake_tail_x;
		last_tail_y = play_snake_tail_y;
		snake_head = play_snake_head;
	}// 结束移动绘制

	/* 绘制头方向的改变 */
	if(play_snake[play_snake_head] != snake_toward)
	{
		snake_toward = play_snake[play_snake_head];
		gotoxy(play_snake_head_x*2+SNAKE_AREA_OFFSET_X,play_snake_head_y+SNAKE_AREA_OFFSET_Y);
		printf(SnakeHeadShape[snake_toward]);
	}// 结束头方向绘制

	// 更新食物显示
	if(last_food_x != play_snake_food_x || last_food_y != play_snake_food_y)
	{
		last_food_x = play_snake_food_x;
		last_food_y = play_snake_food_y;
		// 食物不需要清除上次显示 只需要从绘就可以了
		gotoxy(last_food_x*2+SNAKE_AREA_OFFSET_X,last_food_y+SNAKE_AREA_OFFSET_Y);
		printf(SnakeFoodShape);
	}

	// 检测长度
	if(last_length != play_snake_length)
	{
		last_length = play_snake_length;
		gotoxy(SNAKE_AREA_WIDTH + 8, 3);
		printf("%3d",last_length);
	}
	// 检测时间
	if(last_time != play_time)
	{
		last_time = play_time;
		gotoxy(SNAKE_AREA_WIDTH + 4, 6);
		printf("%6dms",last_time);
	}

}
void show_pause()
{
	/* 初始绘制 */
	if(changed_game_state())
	{
		// 绘制对话框
		set_default_color();
		clrscr();
		draw_dialog(10, 4, 40, 10, "游戏暂停","按<空格>键返回游戏",SingleBorder);
	}
}
void show_save()
{
	/* 初始绘制 */
	if(changed_game_state())
	{
		// 绘制对话框
		set_default_color();
		clrscr();
		draw_dialog(10, 4, 40, 10, "保存游戏","游戏正在保存,请稍等...",SingleBorder);
	}
}
void show_load()
{
	/* 初始绘制 */
	if(changed_game_state())
	{
		// 绘制对话框
		set_default_color();
		clrscr();
		draw_dialog(10, 4, 40, 10, "读取游戏","正在读取游戏,请稍等...",SingleBorder);
	}
}
void show_backto_mainmenu()
{
	static int last_choice = -1;
	/* 初始化 */
	if(changed_game_state())
	{
		// 强制更新
		last_choice = -1;
		// 绘制对话框
		set_default_color();
		draw_dialog(10, 4, 40, 10, "退出游戏","确认不保存游戏返回主菜单么?",SingleBorder);
		// 选项
		gotoxy( 10 + (40 - 12)/2, 4 + 10 - 2);
		printf(" 继续  退出 ");
	}// 完成初始化
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
	/* 初始化 */
	if(changed_game_state())
	{
		// 强制更新
		last_choice = -1;
		// 绘制对话框
		set_default_color();
		draw_dialog(10, 4, 40, 10, "游戏胜利","少侠好生威武,再来一次么?",SingleBorder);
		// 选项
		gotoxy( 10 + (40 - 12)/2, 4 + 10 - 2);
		printf(" 继续  退出 ");
	}// 完成初始化
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
/* 总显示函数 */
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
