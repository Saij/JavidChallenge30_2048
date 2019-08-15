#include <iostream>
#include <string>
using namespace std;

#include "olcConsoleGameEngine.h"

enum GAME_STATE {
	GAME_STATE_TITLE = 0x0001,
	GAME_STATE_START = 0x0002,
	GAME_STATE_EXIT  = 0x0003
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
	int m_nTileSize = 5;
	int m_nFieldSize = 0;
	int m_nFieldOffsetX = 0;
	int* m_aGrid;
	int m_nScore = 0;

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

		m_nFieldSize = (m_nTileSize + 1) * 4 + 1;
		m_nFieldOffsetX = (int)(ScreenWidth() / 2 - m_nFieldSize / 2);

		m_aGrid = new int[16];
		memset(m_aGrid, 0x00, 16 * sizeof(int));

		return true;
	}

	virtual bool OnUserDestroy()
	{
		delete[] m_aGrid;
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

		case GAME_STATE_EXIT:
			return false;
		}

		return true;
	}

	void GameStateStart(float fElapsedTime)
	{
		if (GetKey(VK_ESCAPE).bReleased) {
			m_nGameState = GAME_STATE_EXIT;
			return;
		}

		// Draw the field
		Fill(0, 0, 30, m_nFieldSize, PIXEL_SOLID, FG_DARK_GREY);

		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				int nValue = m_aGrid[y * 4 + x];

				// Just draw a black rectangle
				int nStartX = 1 + m_nFieldOffsetX + (x * (m_nTileSize + 1));
				int nEndX = nStartX + m_nTileSize;
				int nStartY = 1 + y * (m_nTileSize + 1);
				int nEndY = nStartY + m_nTileSize;
				Fill(nStartX, nStartY, nEndX, nEndY, PIXEL_SOLID, FG_BLACK);
			}
		}

		// Print score
		wstring sScoreString = L"Score: ";
		sScoreString.append(to_wstring(m_nScore));
		DrawString(1, m_nFieldSize + 1, sScoreString, FG_WHITE);

		// Print exit help
		DrawString(1, m_nFieldSize + 3, L"Press ESC to exit", FG_WHITE);
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
