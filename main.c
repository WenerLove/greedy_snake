#include <stdio.h>
#include <stdlib.h>

#include "game.h"


int main(int argc, char* argv[])
{
	while(1)
	{
		process();
		show();

		/* 游戏退出条件 */
		if(changed_game_state() && get_game_state() == GAME_EXIT)
			break;
	}
return EXIT_SUCCESS;
}
