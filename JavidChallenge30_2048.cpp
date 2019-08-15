#include <iostream>
#include <string>
using namespace std;

#include "olcConsoleGameEngine.h"

enum GAME_STATE {
	GAME_STATE_TITLE = 0x0001,
	GAME_STATE_START = 0x0002
};

class c2048 : public olcConsoleGameEngine
{
public:
	c2048()
	{
		m_sAppName = L"2048";
	}

private:
	GAME_STATE m_nGameState = GAME_STATE_TITLE;
	wstring m_sTitleGraphic = L"";
	int m_nTitleGraphicWidth = 0;

protected:
	virtual bool OnUserCreate()
	{
		// Hide blinking cursor
		CONSOLE_CURSOR_INFO cursorInfo;
		GetConsoleCursorInfo(m_hConsole, &cursorInfo);
		cursorInfo.bVisible = false; // set the cursor visibility
		SetConsoleCursorInfo(m_hConsole, &cursorInfo);

		// Make window not resizable!
		HWND consoleWindow = GetConsoleWindow();
		SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

		// Initialise title graphic
		m_sTitleGraphic += L".####...####......#...####.";
		m_sTitleGraphic += L"#....#.#....#....##..#....#";
		m_sTitleGraphic += L"....#..#....#...#.#..#....#";
		m_sTitleGraphic += L"...#...#....#..#..#...####.";
		m_sTitleGraphic += L"..#....#....#.######.#....#";
		m_sTitleGraphic += L".#.....#....#.....#..#....#";
		m_sTitleGraphic += L"######..####......#...####.";

		// Divide the length by 7 rows
		m_nTitleGraphicWidth = m_sTitleGraphic.length() / 7;

		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime)
	{
		// First we fill the complete screen black
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		// Run code depending on game state
		switch (m_nGameState) {
		case GAME_STATE_TITLE:
			GameStateTitle(fElapsedTime);
			break;

		case GAME_STATE_START:
			GameStateStart(fElapsedTime);
			break;
		}

		return true;
	}

	void GameStateStart(float fElapsedTime)
	{
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_GREY);
	}

	void GameStateTitle(float fElapsedTime)
	{
		// First handle input
		if (GetKey(VK_SPACE).bPressed) {
			m_nGameState = GAME_STATE_START;
			return;
		}

		// Timing for blinking text
		static float fBlinkTiming = 0;

		// Offsets for the title graphics
		// and the other texts
		int nOffsetX = 2;
		int nOffsetY = 5;
		int nOffsetSubtitleY = nOffsetY + 7 + 5;
		int nOffsetBlinkTextY = nOffsetSubtitleY + 5;

		// Draw the title graphic
		for (int x = 0; x < m_nTitleGraphicWidth; x++)
		{
			for (int y = 0; y < 7; y++)
			{
				if (m_sTitleGraphic[y * m_nTitleGraphicWidth + x] == L'#') {
					Draw(x + nOffsetX, y + nOffsetY, PIXEL_SOLID, FG_WHITE);
				}
			}
		}

		// Draw subtitle centered in width and 5 rows below title
		wstring sSubtitle = L"30 Edition";
		DrawString((int)(ScreenWidth() / 2 - sSubtitle.length() / 2), nOffsetSubtitleY, sSubtitle, FG_WHITE);

		fBlinkTiming += fElapsedTime;
		if (fBlinkTiming <= 1.0f) 
		{
			// Draw the text
			wstring sBlinkText = L"Press Space to start";
			DrawString((int)(ScreenWidth() / 2 - sBlinkText.length() / 2), nOffsetBlinkTextY, sBlinkText, FG_WHITE);
		}
		else if (fBlinkTiming > 2.0f) 
		{
			// Reset timer
			fBlinkTiming = 0.0f;
		}
	}
};

int main()
{
	c2048 game;
	game.ConstructConsole(30, 30, 16, 16);
	game.Start();
	return 0;
}
