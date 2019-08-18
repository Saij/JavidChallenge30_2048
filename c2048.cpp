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

	case GAME_STATE_ANIMATE:
		GameStateAnimate(fElapsedTime);
		break;
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
		nReturn = x * 4 + y;		//  1  5  9 13
		break;						//  2  6 10 14
									//  3  7 11 15

	case DOWN:               		// 12  8  4  0
		nReturn = (3 - x) * 4 + y;	// 13  9  5  1
		break;						// 14 10  6  2
	}								// 15 11  7  3

	return nReturn;
}

/**
 * Draws a cell
 */
void c2048::DrawCell(int nCellIndex, short nChar)
{
	sCell oCell = m_aGrid[nCellIndex];

	short nCellColor;
	short nTextColor;

	GetCellColor(oCell.nValue, nCellColor, nTextColor);

	int nPosX = oCell.nPosX + (int)oCell.fAnimOffsetX;
	int nPosY = oCell.nPosY + (int)oCell.fAnimOffsetY;

	// Just draw a colored rectangle
	Fill(nPosX, nPosY, nPosX + m_nTileSize, nPosY + m_nTileSize, nChar, nCellColor);

	// Draw value of cell (only if greater then 0)
	if (oCell.nValue > 0) {
		wstring sCellText = to_wstring(oCell.nValue * (m_nNumberSystem / 2));
		int nTextX = (int)(nPosX + (m_nTileSize / 2) - sCellText.length() / 2);
		int nTextY = (int)(nPosY + m_nTileSize / 2);

		DrawString(nTextX, nTextY, sCellText, nTextColor);
	}
}

/**
 * Resets the complete game to the beginning state
 */
void c2048::ResetGameData(GAME_STATE state)
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
			m_aGrid[nCellIndex].bNeedsAnimation = false;
			m_aGrid[nCellIndex].fAnimOffsetX = 0.0f;
			m_aGrid[nCellIndex].fAnimOffsetY = 0.0f;
			m_aGrid[nCellIndex].bHasSpecialAnimation = false;
		}
	}

	m_nScore = 0;
	m_nGameState = state;

	// Add 2 numbers in random cells
	AddNewNumber(false);
	AddNewNumber(false);
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
void c2048::AddNewNumber(bool bAnimate)
{
	int nCellValue = (rand() < RAND_MAX * 0.9f) ? 2 : 4;
	AddNewNumber(nCellValue, bAnimate);
}

/**
 * Adds the number specified in nValue to a random location to the grid.
 * But only to available spots.
 */
void c2048::AddNewNumber(int nValue, bool bAnimate)
{
	vector<int> aAvailableCells = GetAvailableCells();

	if (aAvailableCells.empty())
		return;

	// Get random available cell
	int nCellIndex = aAvailableCells[rand() % aAvailableCells.size()];

	m_aGrid[nCellIndex].nValue = nValue;
	m_aGrid[nCellIndex].nDestinationCellIndex = -1;
	m_aGrid[nCellIndex].bHasSpecialAnimation = bAnimate;

	if (bAnimate) {
		m_nGameState = GAME_STATE_ANIMATE;
		m_fAnimationTime = 0;
	}
}

/**
 * Adds the number to a specific cell
 */
void c2048::AddNewNumber(int nValue, int x, int y, bool bAnimate)
{
	int nCellIndex = GetCellIndex(x, y, LEFT);
	m_aGrid[nCellIndex].nValue = nValue;
	m_aGrid[nCellIndex].nDestinationCellIndex = -1;
	m_aGrid[nCellIndex].bHasSpecialAnimation = bAnimate;

	if (bAnimate) {
		m_nGameState = GAME_STATE_ANIMATE;
		m_fAnimationTime = 0;
	}
}

/**
 * Gets the foreground color, background color and text color
 * for a specific value of a cell
 *
 * Hardcoded because we only support till 2048
 */
void c2048::GetCellColor(int nValue, short& cellColor, short& textColor)
{
	switch (nValue) {
	case 2:
		cellColor = FG_YELLOW;
		textColor = FG_BLACK | BG_YELLOW;
		break;

	case 4:
		cellColor = FG_DARK_YELLOW;
		textColor = FG_BLACK | BG_DARK_YELLOW;
		break;

	case 8:
		cellColor = FG_RED;
		textColor = FG_BLACK | BG_RED;
		break;

	case 16:
		cellColor = FG_DARK_RED;
		textColor = FG_BLACK | BG_DARK_RED;
		break;

	case 32:
		cellColor = FG_BLUE;
		textColor = FG_BLACK | BG_BLUE;
		break;

	case 64:
		cellColor = FG_DARK_BLUE;
		textColor = FG_BLACK | BG_DARK_BLUE;
		break;

	case 128:
		cellColor = FG_GREEN;
		textColor = FG_BLACK | BG_GREEN;
		break;

	case 256:
		cellColor = FG_DARK_GREEN;
		textColor = FG_BLACK | BG_DARK_GREEN;
		break;

	case 512:
		cellColor = FG_CYAN;
		textColor = FG_BLACK | BG_CYAN;
		break;

	case 1024:
		cellColor = FG_DARK_CYAN;
		textColor = FG_BLACK | BG_DARK_CYAN;
		break;

	case 2048:
		cellColor = FG_WHITE;
		textColor = FG_BLACK | BG_WHITE;
		break;

	default:
		cellColor = FG_BLACK;
		textColor = FG_WHITE | BG_BLACK;
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
		int nLastCellMovedIndex = -1;
		int nLastCellValue = -1;
		int nLastReceiverCellIndex = -1;
		int nStartPrevX = 0;
		bool bLastCellMerged = false;

		for (int x = 0; x < 4; x++) {
			int nCurrentCellIndex = GetCellIndex(x, y, dir);

			// If first cell just safe the values and continue
			if (x == 0) {
				nLastCellMovedIndex = -1;
				nLastCellValue = m_aGrid[nCurrentCellIndex].nValue;
				bLastCellMerged = false;
				continue;
			}

			// Cell is empty - so nothing todo
			if (m_aGrid[nCurrentCellIndex].nValue == 0)
				continue;

			// Iterate over all the previous cells to check if we find a space
			// or a way to merge
			bool bCellFinished = false;
			for (int prevX = nStartPrevX; prevX < x && !bCellFinished; prevX++) {
				int nPreviousCellIndex = GetCellIndex(prevX, y, dir);

				if (m_aGrid[nCurrentCellIndex].nValue == nLastCellValue && !bLastCellMerged) {
					// Last cell value is the same as our current cell
					// So we can merge
					m_aGrid[nCurrentCellIndex].nDestinationCellIndex = nPreviousCellIndex;
					m_aGrid[nCurrentCellIndex].bNeedsAnimation = true;

					nLastReceiverCellIndex = nPreviousCellIndex;

					bHasMoved = true;
					bCellFinished = true;

					nStartPrevX = prevX + 1;
					nLastCellValue = 0;
					bLastCellMerged = true;
				}
				else if (m_aGrid[nPreviousCellIndex].nValue == 0 && (nLastReceiverCellIndex != nPreviousCellIndex || nLastCellValue == 0)) {
					// Cell is empty so we use it directly
					m_aGrid[nCurrentCellIndex].nDestinationCellIndex = nPreviousCellIndex;
					m_aGrid[nCurrentCellIndex].bNeedsAnimation = true;

					nLastReceiverCellIndex = nPreviousCellIndex;

					bHasMoved = true;
					bCellFinished = true;

					nStartPrevX = prevX;
					nLastCellValue = m_aGrid[nCurrentCellIndex].nValue;
					bLastCellMerged = false;
				}
				else if (nLastCellMovedIndex == nPreviousCellIndex) {
					// Cell has already been moved so it is considered empty
					m_aGrid[nCurrentCellIndex].nDestinationCellIndex = nPreviousCellIndex;
					m_aGrid[nCurrentCellIndex].bNeedsAnimation = true;

					nLastReceiverCellIndex = nPreviousCellIndex;

					bHasMoved = true;
					bCellFinished = true;

					nStartPrevX = prevX;
					nLastCellValue = m_aGrid[nCurrentCellIndex].nValue;
					bLastCellMerged = false;
				}
			}

			if (!bCellFinished) {
				// Cell had not been moved
				nLastCellValue = m_aGrid[nCurrentCellIndex].nValue;
				bLastCellMerged = false;
				nLastReceiverCellIndex = nCurrentCellIndex;
				nStartPrevX = x;
			}
			else {
				nLastCellMovedIndex = nCurrentCellIndex;
			}
		}
	}

	return bHasMoved;
}

/**
 * Executes the precalculated cell movement
 */
void c2048::CalculateCellMovement(ROTATION dir)
{
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			int nCurrentCellIndex = GetCellIndex(x, y, dir);
			int nTargetCellIndex = m_aGrid[nCurrentCellIndex].nDestinationCellIndex;
			
			if (nTargetCellIndex == -1)
				continue;

			if (m_aGrid[nTargetCellIndex].nValue == m_aGrid[nCurrentCellIndex].nValue)
				// Combine those two cells
				m_aGrid[nTargetCellIndex].
				nValue += m_aGrid[nCurrentCellIndex].nValue;
			else
				// Move the cell
				m_aGrid[nTargetCellIndex].nValue = m_aGrid[nCurrentCellIndex].nValue;

			m_aGrid[nCurrentCellIndex].nDestinationCellIndex = -1;
			m_aGrid[nCurrentCellIndex].nValue = 0;
			m_aGrid[nCurrentCellIndex].bNeedsAnimation = false;
			m_aGrid[nCurrentCellIndex].fAnimOffsetX = 0.0f;
			m_aGrid[nCurrentCellIndex].fAnimOffsetY = 0.0f;
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

	if (GetKey(L'R').bReleased) {
		ResetGameData(GAME_STATE_START);
		return;
	}

	if (GetKey(VK_RIGHT).bReleased) {
		m_nAnimationDirection = RIGHT;
		m_bHasMoved = MoveCells(RIGHT);
	}
	else if (GetKey(VK_LEFT).bReleased) {
		m_nAnimationDirection = LEFT;
		m_bHasMoved = MoveCells(LEFT);
	}
	else if (GetKey(VK_UP).bReleased) {
		m_nAnimationDirection = TOP;
		m_bHasMoved = MoveCells(TOP);
	}
	else if (GetKey(VK_DOWN).bReleased) {
		m_nAnimationDirection = DOWN;
		m_bHasMoved = MoveCells(DOWN);
	}

	// If something has moved start the animation
	if (m_bHasMoved) {
		m_nGameState = GAME_STATE_ANIMATE;
		return;
	}

	DrawGameField();
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++)
			DrawCell(GetCellIndex(x, y));
}

/**
 * Draws the game field
 */
void c2048::DrawGameField()
{
	// Draw the field
	Fill(0, 0, 30, m_nFieldSize, PIXEL_SOLID, FG_DARK_GREY);

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			int nPosX = 1 + m_nFieldOffsetX + (x * (m_nTileSize + 1));
			int nPosY = 1 + y * (m_nTileSize + 1);

			Fill(nPosX, nPosY, nPosX + m_nTileSize, nPosY + m_nTileSize, PIXEL_SOLID, FG_BLACK);
		}
	}

	// Print score
	wstring sScoreString = L"Score: ";
	sScoreString.append(to_wstring(m_nScore));
	DrawString(1, m_nFieldSize + 1, sScoreString, FG_WHITE);

	// Print exit help
	DrawString(1, m_nFieldSize + 3, L"Press ESC to exit", FG_WHITE);

	// Print restart help
	DrawString(1, m_nFieldSize + 4, L"Press R to restart", FG_WHITE);
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

	// First we fill the complete screen black
	Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

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

	m_fAnimationTime += fElapsedTime * m_nBlinkAnimationSpeed;
	int nAnimationIndex = ((int)m_fAnimationTime) % m_nBlinkAnimation.size();

	// Draw the text
	wstring sBlinkText = L"Press Space to start";
	DrawString((int)(ScreenWidth() / 2 - sBlinkText.length() / 2), nOffsetBlinkTextY, sBlinkText, m_nBlinkAnimation[nAnimationIndex]);
}

/**
 * Handle for the game state GAME_STATE_ANIMATE
 *
 * Animates the moving tiles
 */
void c2048::GameStateAnimate(float fElapsedTime)
{
	bool bNeedsMoreAnimation = false;
	bool bNeedNewNumber = true;

	auto CellIsFinished = [&](int nCurrent, int nTarget, float fOffset) -> bool {
		int nBoundLeft = nTarget - 1;
		int nBoundRight = nTarget + 1;
		float fNewCurrent = nCurrent + fOffset;

		return nBoundLeft < fNewCurrent && fNewCurrent < nBoundRight;
	};

	DrawGameField();

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			int nCurrentCellIndex = GetCellIndex(x, y, m_nAnimationDirection);

			if (m_aGrid[nCurrentCellIndex].bNeedsAnimation == false && m_aGrid[nCurrentCellIndex].nValue > 0 && !m_aGrid[nCurrentCellIndex].bHasSpecialAnimation) {
				// Cell needs no animation but has a value
				DrawCell(nCurrentCellIndex);
			}
			else if (m_aGrid[nCurrentCellIndex].bNeedsAnimation) {
				// Cell needs to be animated
				float fSpeedFactor = 6.0f;
				float fMovementSpeed = 1.0f;
				int nTargetCellIndex = m_aGrid[nCurrentCellIndex].nDestinationCellIndex;

				if (m_nAnimationDirection == LEFT || m_nAnimationDirection == RIGHT)
					fMovementSpeed = (float)abs(m_aGrid[nCurrentCellIndex].nPosX - m_aGrid[nTargetCellIndex].nPosX);
				else
					fMovementSpeed = (float)abs(m_aGrid[nCurrentCellIndex].nPosY - m_aGrid[nTargetCellIndex].nPosY);

				float fAddition = fMovementSpeed * fElapsedTime * fSpeedFactor;
				bool bIsFinished = false;

				switch (m_nAnimationDirection) {
				case LEFT:
					m_aGrid[nCurrentCellIndex].fAnimOffsetX -= fAddition;
					DrawCell(nCurrentCellIndex);
					bIsFinished = CellIsFinished(m_aGrid[nCurrentCellIndex].nPosX, m_aGrid[nTargetCellIndex].nPosX, m_aGrid[nCurrentCellIndex].fAnimOffsetX);
					break;

				case RIGHT:
					m_aGrid[nCurrentCellIndex].fAnimOffsetX += fAddition;
					DrawCell(nCurrentCellIndex);
					bIsFinished = CellIsFinished(m_aGrid[nCurrentCellIndex].nPosX, m_aGrid[nTargetCellIndex].nPosX, m_aGrid[nCurrentCellIndex].fAnimOffsetX);
					break;

				case TOP:
					m_aGrid[nCurrentCellIndex].fAnimOffsetY -= fAddition;
					DrawCell(nCurrentCellIndex);
					bIsFinished = CellIsFinished(m_aGrid[nCurrentCellIndex].nPosY, m_aGrid[nTargetCellIndex].nPosY, m_aGrid[nCurrentCellIndex].fAnimOffsetY);
					break;

				case DOWN:
					m_aGrid[nCurrentCellIndex].fAnimOffsetY += fAddition;
					DrawCell(nCurrentCellIndex);
					bIsFinished = CellIsFinished(m_aGrid[nCurrentCellIndex].nPosY, m_aGrid[nTargetCellIndex].nPosY, m_aGrid[nCurrentCellIndex].fAnimOffsetY);
					break;
				}

				if (bIsFinished)
					m_aGrid[nCurrentCellIndex].bNeedsAnimation = false;
				else
					bNeedsMoreAnimation = true;				
			}
			else if (m_aGrid[nCurrentCellIndex].bHasSpecialAnimation) {
				m_fAnimationTime += fElapsedTime * m_nNewTileAnimationSpeed;
				int nAnimationIndex = ((int)m_fAnimationTime) % m_nNewTileAnimation.size();

				DrawCell(nCurrentCellIndex, m_nNewTileAnimation[nAnimationIndex]);

				if (nAnimationIndex == m_nNewTileAnimation.size() - 1)
					m_aGrid[nCurrentCellIndex].bHasSpecialAnimation = false;
				else
					bNeedsMoreAnimation = true;

				bNeedNewNumber = false;
			}
		}
	}

	if (!bNeedsMoreAnimation && bNeedNewNumber) {
		CalculateCellMovement(m_nAnimationDirection);
		AddNewNumber();
	} 
	else if (!bNeedsMoreAnimation) {
		m_nGameState = GAME_STATE_START;
		m_bHasMoved = false;
	}
}