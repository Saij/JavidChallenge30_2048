#include "c2048.h"

int main()
{
	c2048 game;
	game.ConstructConsole(30, 30, 16, 16);
	game.Start();
	return 0;
}
