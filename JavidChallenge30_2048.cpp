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
	int* m_aGrid;
	int m_nScore;
	int m_nNumberSystem = 30;

	wstring m_sTitleGraphic = L"";
	int m_nTitleGraphicWidth = 0;
	vector<short> m_nBlinkAnimation{ FG_WHITE, FG_WHITE, FG_WHITE, FG_GREY, FG_DARK_GREY, FG_BLACK, FG_BLACK, FG_BLACK, FG_DARK_GREY, FG_GREY };
	int m_nBlinkAnimationSpeed = 10;

	int m_nTileSize = 5;
	int m_nFieldSize = 0;
	int m_nFieldOffsetX = 0;


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
		m_nTitleGraphicWidth = (int)m_sTitleGraphic.length() / 7;

		m_nFieldSize = (m_nTileSize + 1) * 4 + 1;
		m_nFieldOffsetX = (int)(ScreenWidth() / 2 - m_nFieldSize / 2);

		m_aGrid = new int[16];

		ResetGameData();
		
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

private:

	int Rotate(int px, int py, int r)
	{
		int pi = 0;
		switch (r % 4)
		{
		case 0: // 0 degrees			//  0  1  2  3
			pi = py * 4 + px;			//  4  5  6  7
			break;						//  8  9 10 11
										// 12 13 14 15

		case 1: // 90 degrees			// 12  8  4  0
			pi = 12 + py - (px * 4);	// 13  9  5  1
			break;						// 14 10  6  2
										// 15 11  7  3

		case 2: // 180 degrees			// 15 14 13 12
			pi = 15 - (py * 4) - px;	// 11 10  9  8
			break;						//  7  6  5  4
										//  3  2  1  0

		case 3: // 270 degrees			//  3  7 11 15
			pi = 3 - py + (px * 4);		//  2  6 10 14
			break;						//  1  5  9 13
		}								//  0  4  8 12

		return pi;
	}

	/**
	 * Resets the complete game to the beginning state
	 */
	void ResetGameData()
	{
		// Reset complete grid
		memset(m_aGrid, 0x00, 16 * sizeof(int));

		m_nScore = 0;
		m_nGameState = GAME_STATE_TITLE;

		// Add 2 numbers in random cells
		AddNewNumber();
		AddNewNumber();
	}
	
	/**
	 * Calculate all available cells
	 */
	vector<pair<int, int>> GetAvailableCells()
	{
		vector<pair<int, int>> aAvailableCells;

		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				int nCellIndex = y * 4 + x;
				if (m_aGrid[nCellIndex] == 0) {
					aAvailableCells.push_back(make_pair(x, y));
				}
			}
		}
		
		return aAvailableCells;
	}

	/**
	 * Adds a new number to the grid
	 * 90% it should be a 2 and 10% it should be a 4
	 */
	void AddNewNumber()
	{
		int nCellValue = (rand() < RAND_MAX * 0.9f) ? 2 : 4;
		AddNewNumber(nCellValue);
	}

	/**
	 * Adds the number specified in nValue to a random location to the grid.
	 * But only to available spots.
	 */
	void AddNewNumber(int nValue) 
	{
		vector<pair<int, int>> aAvailableCells = GetAvailableCells();

		// Get random available cell
		pair<int, int> oCell = aAvailableCells[rand() % aAvailableCells.size()];
		int nCellIndex = oCell.second * 4 + oCell.first;

		m_aGrid[nCellIndex] = nValue;
	}

	/**
	 * Gets the foreground color, background color and text color
	 * for a specific value of a cell
	 *
	 * Hardcoded because we only support till 2048
	 */
	void GetCellColor(int nValue, short &fgColor, short &bgColor, short &textColor)
	{
		switch (nValue) {
		case 2:
			fgColor = FG_YELLOW;
			bgColor = BG_YELLOW;
			textColor = FG_BLACK;
			break;

		case 4:
			fgColor = FG_DARK_YELLOW;
			bgColor = BG_DARK_YELLOW;
			textColor = FG_BLACK;
			break;

		case 8:
			fgColor = FG_RED;
			bgColor = BG_RED;
			textColor = FG_WHITE;
			break;

		case 16:
			fgColor = FG_DARK_RED;
			bgColor = BG_DARK_RED;
			textColor = FG_WHITE;
			break;

		case 32:
			fgColor = FG_GREEN;
			bgColor = BG_GREEN;
			textColor = FG_BLACK;
			break;

		case 64:
			fgColor = FG_DARK_GREEN;
			bgColor = BG_DARK_GREEN;
			textColor = FG_WHITE;
			break;

		case 128:
			fgColor = FG_BLUE;
			bgColor = BG_BLUE;
			textColor = FG_WHITE;
			break;

		case 256:
			fgColor = FG_DARK_BLUE;
			bgColor = BG_DARK_BLUE;
			textColor = FG_WHITE;
			break;

		case 512:
			fgColor = FG_CYAN;
			bgColor = BG_CYAN;
			textColor = FG_BLACK;
			break;

		case 1024:
			fgColor = FG_DARK_CYAN;
			bgColor = BG_DARK_CYAN;
			textColor = FG_WHITE;
			break;

		case 2048:
			fgColor = FG_WHITE;
			bgColor = BG_WHITE;
			textColor = FG_BLACK;
			break;

		default:
			fgColor = FG_BLACK;
			bgColor = BG_BLACK;
			textColor = FG_BLACK;
			break;
		}
	}

	/**
	 * Update handler for GAME_STATE_START gamestate
	 */
	void GameStateStart(float fElapsedTime)
	{
		if (GetKey(VK_ESCAPE).bReleased) {
			m_nGameState = GAME_STATE_EXIT;
			return;
		}

#ifdef _DEBUG
		// DEBUG STUFF
		if (GetKey(L'D').bHeld) {
			if (GetKey(L'P').bReleased) {
				AddNewNumber(2048);
			}
		}

		if (GetKey(L'R').bReleased) {
			ResetGameData();
		}
#endif

		bool bHasMoved = false;

		if (GetKey(VK_RIGHT).bReleased) {
			for (int i = 0; i < 4; i++) {
				bool bHasSmthMoved = MoveCells(make_pair(3, i), make_pair(2, i), make_pair(1, i), make_pair(0, i));
				bool bHasSmthCombined = CombineCells(make_pair(3, i), make_pair(2, i), make_pair(1, i), make_pair(0, i));
				
				if (bHasSmthMoved || bHasSmthCombined)
					bHasSmthMoved |= MoveCells(make_pair(3, i), make_pair(2, i), make_pair(1, i), make_pair(0, i));

				bHasMoved |= bHasSmthMoved | bHasSmthCombined;
			}
		}
		else if (GetKey(VK_LEFT).bReleased) {
			for (int i = 0; i < 4; i++) {
				bool bHasSmthMoved = MoveCells(make_pair(0, i), make_pair(1, i), make_pair(2, i), make_pair(3, i));
				bool bHasSmthCombined = CombineCells(make_pair(0, i), make_pair(1, i), make_pair(2, i), make_pair(3, i));
					
				if (bHasSmthMoved || bHasSmthCombined)
					bHasSmthMoved |= MoveCells(make_pair(0, i), make_pair(1, i), make_pair(2, i), make_pair(3, i));

				bHasMoved |= bHasSmthMoved | bHasSmthCombined;
			}
		}
		else if (GetKey(VK_UP).bReleased) {
			for (int i = 0; i < 4; i++) {
				bool bHasSmthMoved = MoveCells(make_pair(i, 0), make_pair(i, 1), make_pair(i, 2), make_pair(i, 3));
				bool bHasSmthCombined = CombineCells(make_pair(i, 0), make_pair(i, 1), make_pair(i, 2), make_pair(i, 3));

				if (bHasSmthMoved || bHasSmthCombined)
					bHasSmthMoved |= MoveCells(make_pair(i, 0), make_pair(i, 1), make_pair(i, 2), make_pair(i, 3));

				bHasMoved |= bHasSmthMoved | bHasSmthCombined;
			}
		}
		else if (GetKey(VK_DOWN).bReleased) {
			for (int i = 0; i < 4; i++) {
				bool bHasSmthMoved = MoveCells(make_pair(i, 3), make_pair(i, 2), make_pair(i, 1), make_pair(i, 0));
				bool bHasSmthCombined = CombineCells(make_pair(i, 3), make_pair(i, 2), make_pair(i, 1), make_pair(i, 0));

				if (bHasSmthMoved || bHasSmthCombined)
					bHasSmthMoved |= MoveCells(make_pair(i, 3), make_pair(i, 2), make_pair(i, 1), make_pair(i, 0));

				bHasMoved |= bHasSmthMoved | bHasSmthCombined;
			}
		}

		if (bHasMoved) {
			// Add new number
			AddNewNumber();
		}

		// Draw the field
		Fill(0, 0, 30, m_nFieldSize, PIXEL_SOLID, FG_DARK_GREY);

		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				int nValue = GetCellValue(make_pair(x, y));

				short nFgColor;
				short nBgColor;
				short nTextColor;

				GetCellColor(nValue, nFgColor, nBgColor, nTextColor);

				// Just draw a black rectangle
				int nStartX = 1 + m_nFieldOffsetX + (x * (m_nTileSize + 1));
				int nEndX = nStartX + m_nTileSize;
				int nStartY = 1 + y * (m_nTileSize + 1);
				int nEndY = nStartY + m_nTileSize;
				Fill(nStartX, nStartY, nEndX, nEndY, PIXEL_SOLID, nFgColor);

				// Draw value of cell (only if greater then 0
				if (nValue > 0) {
					wstring sCell = to_wstring(nValue * (m_nNumberSystem / 2));
					int nTextY = (int)(nStartY + m_nTileSize / 2);
					int nTextX = (int)(nStartX + (m_nTileSize / 2) - sCell.length() / 2);
					DrawString(nTextX, nTextY, sCell, nTextColor | nBgColor);
				}
			}
		}

		// Print score
		wstring sScoreString = L"Score: ";
		sScoreString.append(to_wstring(m_nScore));
		DrawString(1, m_nFieldSize + 1, sScoreString, FG_WHITE);

		// Print exit help
		DrawString(1, m_nFieldSize + 3, L"Press ESC to exit", FG_WHITE);
	}

	void SwitchCells(pair<int, int> oCell1, pair<int, int> oCell2)
	{
		SetCellValue(oCell1, GetCellValue(oCell2));
		SetCellValue(oCell2, 0);
	}

	bool MoveCells(pair<int, int> oCell1, pair<int, int> oCell2, pair<int, int> oCell3, pair<int, int> oCell4)
	{
		bool bHasMoved = false;
		// Ignore cell 1

		// Process cell 2
		if (GetCellValue(oCell2) > 0 && GetCellValue(oCell1) == 0) {
			SwitchCells(oCell1, oCell2);
			bHasMoved = true;
		}

		// Process cell 3
		if (GetCellValue(oCell3) > 0) {
			if (GetCellValue(oCell1) == 0) {
				SwitchCells(oCell1, oCell3);
				bHasMoved = true;
			}
			else if (GetCellValue(oCell2) == 0) {
				SwitchCells(oCell2, oCell3);
				bHasMoved = true;
			}
		}

		// Process cell 4
		if (GetCellValue(oCell4) > 0) {
			if (GetCellValue(oCell1) == 0) {
				SwitchCells(oCell1, oCell4);
				bHasMoved = true;
			}
			else if (GetCellValue(oCell2) == 0) {
				SwitchCells(oCell2, oCell4);
				bHasMoved = true;
			}
			else if (GetCellValue(oCell3) == 0) {
				SwitchCells(oCell3, oCell4);
				bHasMoved = true;
			}
		}

		return bHasMoved;
	}

	int AddCells(pair<int, int> oCell1, pair<int, int> oCell2)
	{
		SetCellValue(oCell1, GetCellValue(oCell1) + GetCellValue(oCell2));
		SetCellValue(oCell2, 0);

		return GetCellValue(oCell1);
	}

	bool CombineCells(pair<int, int> oCell1, pair<int, int> oCell2, pair<int, int> oCell3, pair<int, int> oCell4)
	{
		bool bHasCombined = false;
		int nResult = 0;

		if (GetCellValue(oCell1) > 0 && GetCellValue(oCell1) == GetCellValue(oCell2)) {
			nResult = AddCells(oCell1, oCell2);
			if (nResult == 4096) {
				// Boooooooom
				SetCellValue(oCell1, 0);
				SetCellValue(oCell2, 0);
			}

			bHasCombined = true;

			if (GetCellValue(oCell3) > 0 && GetCellValue(oCell3) == GetCellValue(oCell4)) {
				int nNextResult = AddCells(oCell3, oCell4);
				if (nNextResult == 4096) {
					// Boooooooom
					SetCellValue(oCell3, 0);
					SetCellValue(oCell4, 0);
				}

				nResult += nNextResult;
			}
		}
		else if (GetCellValue(oCell2) > 0 && GetCellValue(oCell2) == GetCellValue(oCell3)) {
			nResult = AddCells(oCell2, oCell3);
			if (nResult == 4096) {
				// Boooooooom
				SetCellValue(oCell2, 0);
				SetCellValue(oCell3, 0);
			}

			bHasCombined = true;
		}
		else if (GetCellValue(oCell3) > 0 && GetCellValue(oCell3) == GetCellValue(oCell4)) {
			nResult = AddCells(oCell3, oCell4);
			if (nResult == 4096) {
				// Boooooooom
				SetCellValue(oCell3, 0);
				SetCellValue(oCell4, 0);
			}

			bHasCombined = true;
		}

		m_nScore += nResult * (m_nNumberSystem / 2);
		return bHasCombined;
	}

	int GetCellValue(pair<int, int> oCell)
	{
		return m_aGrid[oCell.second * 4 + oCell.first];
	}

	void SetCellValue(pair<int, int> oCell, int nValue)
	{
		m_aGrid[oCell.second * 4 + oCell.first] = nValue;
	}

	/**
	 * Handler for game state GAME_STATE_TITLE
	 *
	 * Renders the title screen
	 */
	void GameStateTitle(float fElapsedTime)
	{
		// First handle input
		if (GetKey(VK_SPACE).bPressed) {
			m_nGameState = GAME_STATE_START;
			return;
		}

		// Offsets for the title graphics
		// and the other texts
		int nOffsetX = 2;
		int nOffsetY = 5;
		int nOffsetSubtitleY = nOffsetY + 7 + 5;
		int nOffsetBlinkTextY = nOffsetSubtitleY + 5;

		// Draw the title graphic
		for (int x = 0; x < m_nTitleGraphicWidth; x++) {
			for (int y = 0; y < 7; y++) {
				if (m_sTitleGraphic[(size_t)y * m_nTitleGraphicWidth + x] == L'#') {
					Draw(x + nOffsetX, y + nOffsetY, PIXEL_SOLID, FG_WHITE);
				}
			}
		}

		// Draw subtitle centered in width and 5 rows below title
		wstring sSubtitle = L"30 Edition";
		DrawString((int)(ScreenWidth() / 2 - sSubtitle.length() / 2), nOffsetSubtitleY, sSubtitle, FG_WHITE);

		// Timing for blinking text
		static float fBlinkTiming = 0;

		fBlinkTiming += fElapsedTime * m_nBlinkAnimationSpeed;
		int nAnimationIndex = ((int)fBlinkTiming) % m_nBlinkAnimation.size();

		// Draw the text
		wstring sBlinkText = L"Press Space to start";
		DrawString((int)(ScreenWidth() / 2 - sBlinkText.length() / 2), nOffsetBlinkTextY, sBlinkText, m_nBlinkAnimation[nAnimationIndex]);
	}
};

int main()
{
	c2048 game;
	game.ConstructConsole(30, 30, 16, 16);
	game.Start();
	return 0;
}
