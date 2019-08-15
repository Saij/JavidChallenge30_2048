#include <iostream>
#include "olcConsoleGameEngine.h"

enum GAME_STATE {
	GAME_STATE_START = 0x0001,
};

class c2048 : public olcConsoleGameEngine
{
public:
	c2048()
	{
		m_sAppName = L"2048";
	}

private:
	GAME_STATE m_nGameState = GAME_STATE_START;

protected:
	void ShowConsoleCursor(bool showFlag)
	{
		CONSOLE_CURSOR_INFO cursorInfo;

		GetConsoleCursorInfo(m_hConsole, &cursorInfo);
		cursorInfo.bVisible = showFlag; // set the cursor visibility
		SetConsoleCursorInfo(m_hConsole, &cursorInfo);
	}

	virtual bool OnUserCreate()
	{
		// Hide blinking cursor
		ShowConsoleCursor(false);

		// Make window not resizable!
		HWND consoleWindow = GetConsoleWindow();
		SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime)
	{
		// First we fill the complete screen black
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		// Run code depending on game state
		switch (m_nGameState) {
		case GAME_STATE_START:
			break;
		}

		return true;
	}
};

int main()
{
	c2048 game;
	game.ConstructConsole(30, 30, 16, 16);
	game.Start();
	return 0;
}
