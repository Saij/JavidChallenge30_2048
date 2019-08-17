#include "c2048.h"

c2048::c2048() : m_aGrid(16)
{
	m_sAppName = L"2048";
}

bool c2048::OnUserCreate()
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

	ResetGameData();

	return true;
}

bool c2048::OnUserDestroy()
{
	m_aGrid.clear();
	return true;
}

bool c2048::OnUserUpdate(float fElapsedTime)
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

/**
 * Gets the cells index based on X and Y
 */
int c2048::GetCellIndex(int x, int y, ROTATION nRotation)
{
	auto clamp = [](int val, int min, int max) -> int
	{
		return (val < min ? min : (val > max ? max : val));
	};

	// Clamp position
	x = clamp(x, 0, 3);
	y = clamp(y, 0, 3);

	int nReturn = 0;

	switch (nRotation)
	{
	case LEFT:             			//  0  1  2  3
		nReturn = y * 4 + x;		//  4  5  6  7
		break;						//  8  9 10 11
									// 12 13 14 15

	case RIGHT:               		//  3  2  1  0
		nReturn = y * 4 + (3 - x);	//  7  6  5  4
		break;						// 11 10  9  8
									// 15 14 13 12

	case TOP:               		//  0  4  8 12
		nReturn = x * 4 + x;		//  1  5  9 13
		break;						//  2  6 10 14
									//  3  7 11 15

	case DOWN:               		//  3  7 11 15
		nReturn = x * 4 + (3 - y);	//  2  6 10 14
		break;						//  1  5  9 13
	}								//  0  4  8 12

	return nReturn;
}

/**
 * Draws a cell
 */
void c2048::DrawCell(int x, int y)
{
	sCell oCell = m_aGrid[GetCellIndex(x, y)];

	short nFgColor;
	short nBgColor;
	short nTextColor;

	GetCellColor(oCell.nValue, nFgColor, nBgColor, nTextColor);

	// Just draw a colored rectangle
	Fill(oCell.nPosX, oCell.nPosY, oCell.nPosX + m_nTileSize, oCell.nPosY + m_nTileSize, PIXEL_SOLID, nFgColor);

	// Draw value of cell (only if greater then 0)
	if (oCell.nValue > 0) {
		wstring sCellText = to_wstring(oCell.nValue * (m_nNumberSystem / 2));
		int nTextX = (int)(oCell.nPosX + (m_nTileSize / 2) - sCellText.length() / 2);
		int nTextY = (int)(oCell.nPosY + m_nTileSize / 2);

		DrawString(nTextX, nTextY, sCellText, nTextColor | nBgColor);
	}
}

/**
 * Resets the complete game to the beginning state
 */
void c2048::ResetGameData()
{
	// Seed random number generator
	srand(clock());

	// Reset complete grid
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			int nCellIndex = GetCellIndex(x, y);

			m_aGrid[nCellIndex].nValue = 0;
			m_aGrid[nCellIndex].nPosX = 1 + m_nFieldOffsetX + (x * (m_nTileSize + 1));
			m_aGrid[nCellIndex].nPosY = 1 + y * (m_nTileSize + 1);
			m_aGrid[nCellIndex].nDestinationCellIndex = -1;
		}
	}

	m_nScore = 0;
	m_nGameState = GAME_STATE_TITLE;

	// Add 2 numbers in random cells
	AddNewNumber();
	AddNewNumber();
}

/**
 * Calculate all available cells
 */
vector<int> c2048::GetAvailableCells()
{
	vector<int> aAvailableCells;

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			int nCellIndex = GetCellIndex(x, y);
			if (m_aGrid[nCellIndex].nValue == 0) {
				aAvailableCells.push_back(nCellIndex);
			}
		}
	}

	return aAvailableCells;
}

/**
 * Adds a new number to the grid
 * 90% it should be a 2 and 10% it should be a 4
 */
void c2048::AddNewNumber()
{
	int nCellValue = (rand() < RAND_MAX * 0.9f) ? 2 : 4;
	AddNewNumber(nCellValue);
}

/**
 * Adds the number specified in nValue to a random location to the grid.
 * But only to available spots.
 */
void c2048::AddNewNumber(int nValue)
{
	vector<int> aAvailableCells = GetAvailableCells();

	if (aAvailableCells.empty())
		return;

	// Get random available cell
	int nCellIndex = aAvailableCells[rand() % aAvailableCells.size()];

	m_aGrid[nCellIndex].nValue = nValue;
}

/**
 * Gets the foreground color, background color and text color
 * for a specific value of a cell
 *
 * Hardcoded because we only support till 2048
 */
void c2048::GetCellColor(int nValue, short& fgColor, short& bgColor, short& textColor)
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
 * Moves and combines cells
 *
 * Returns true if something has been done
 */
bool c2048::MoveCells(ROTATION dir)
{
	bool bHasMoved = false;

	// For each row (if we think of a rotated grid)
	for (int y = 0; y < 4; y++) {
		bool bCellCanBeTarget[4] = { true, true, true, true };

		// Ignore the first cell as there is nothing it can do
		for (int x = 1; x < 4; x++) {
			int nCurrentCellIndex = GetCellIndex(x, y, dir);

			// Cell is empty - so nothing todo
			if (m_aGrid[nCurrentCellIndex].nValue == 0)
				continue;

			// Check all previous cells
			bool bCellFinished = false;
			for (int prevX = 0; prevX < x && !bCellFinished; prevX++) {

				// Previous cell can't be target
				if (!bCellCanBeTarget[prevX])
					continue;
				
				int nPreviousCellIndex = GetCellIndex(prevX, y, dir);

				bCellCanBeTarget[prevX] = false;
				if (m_aGrid[nPreviousCellIndex].nValue == 0 || m_aGrid[nPreviousCellIndex].nValue == m_aGrid[nCurrentCellIndex].nValue) {
					// Target cell is empty
					// or has the same value as current cell
					m_aGrid[nCurrentCellIndex].nDestinationCellIndex = nPreviousCellIndex;
					bHasMoved = true;
					bCellFinished = true;
				}
			}
		}
	}

	return bHasMoved;
}

/**
 * Executes the precalculated cell movement
 */
void c2048::CalculateCellMovement()
{
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			int nCurrentCellIndex = GetCellIndex(x, y);
			int nTargetCellIndex = m_aGrid[nCurrentCellIndex].nDestinationCellIndex;
			
			if (nTargetCellIndex == -1)
				continue;

			if (m_aGrid[nTargetCellIndex].nValue == m_aGrid[nCurrentCellIndex].nValue)
				// Combine those two cells
				m_aGrid[nTargetCellIndex].nValue += m_aGrid[nCurrentCellIndex].nValue;
			else
				// Move the cell
				m_aGrid[nTargetCellIndex].nValue = m_aGrid[nCurrentCellIndex].nValue;

			m_aGrid[nCurrentCellIndex].nDestinationCellIndex = -1;
			m_aGrid[nCurrentCellIndex].nValue = 0;
		}
	}
}

/**
 * Update handler for GAME_STATE_START gamestate
 */
void c2048::GameStateStart(float fElapsedTime)
{
	if (GetKey(VK_ESCAPE).bReleased) {
		m_nGameState = GAME_STATE_EXIT;
		return;
	}

	bool bHasMoved = false;

	if (GetKey(VK_RIGHT).bReleased) {
		bHasMoved = MoveCells(RIGHT);
	}
	else if (GetKey(VK_LEFT).bReleased) {
		bHasMoved = MoveCells(LEFT);
	}
	else if (GetKey(VK_UP).bReleased) {
		bHasMoved = MoveCells(TOP);
	}
	else if (GetKey(VK_DOWN).bReleased) {
		bHasMoved = MoveCells(DOWN);
	}

	CalculateCellMovement();

	// Add a new number if something has been moved
	if (bHasMoved)
		AddNewNumber();

	// Draw the field
	Fill(0, 0, 30, m_nFieldSize, PIXEL_SOLID, FG_DARK_GREY);

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			DrawCell(x, y);
		}
	}

	// Print score
	wstring sScoreString = L"Score: ";
	sScoreString.append(to_wstring(m_nScore));
	DrawString(1, m_nFieldSize + 1, sScoreString, FG_WHITE);

	// Print exit help
	DrawString(1, m_nFieldSize + 3, L"Press ESC to exit", FG_WHITE);
}

/**
 * Handler for game state GAME_STATE_TITLE
 *
 * Renders the title screen
 */
void c2048::GameStateTitle(float fElapsedTime)
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