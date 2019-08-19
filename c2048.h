#pragma once

#include <iostream>
#include <string>
using namespace std;

#include "olcConsoleGameEngineOOP.h"

enum GAME_STATE {
	GAME_STATE_TITLE	= 0x01,
	GAME_STATE_START	= 0x02,
	GAME_STATE_EXIT		= 0x03,
	GAME_STATE_ANIMATE	= 0x04
};

enum ROTATION {
	LEFT = 0,
	TOP = 90,
	RIGHT = 180,
	DOWN = 270
};

struct sCell {
	int nValue;
	int nPosX;
	int nPosY;
	int nDestinationCellIndex;
	bool bNeedsAnimation;
	float fAnimOffsetX;
	float fAnimOffsetY;
	bool bHasSpecialAnimation;
	bool bHasBeenMerged;
	float fAnimationTime;
};

class c2048 : public olcConsoleGameEngineOOP
{
public:
	c2048();

private:
	GAME_STATE m_nGameState = GAME_STATE_TITLE;
	vector<sCell> m_aGrid;
	int m_nScore;
	int m_nNumberSystem = 30;
	bool m_bIsMoving = false;

	wstring m_sTitleGraphic = L"";
	int m_nTitleGraphicWidth = 0;
	
	vector<short> m_nBlinkAnimation{ FG_WHITE, FG_WHITE, FG_WHITE, FG_GREY, FG_DARK_GREY, FG_BLACK, FG_BLACK, FG_BLACK, FG_DARK_GREY, FG_GREY };
	vector<short> m_nNewTileAnimation{ L' ', PIXEL_QUARTER, PIXEL_HALF, PIXEL_THREEQUARTERS, PIXEL_SOLID };
	vector<short> m_nMergeAnimation{ L' ', PIXEL_QUARTER, PIXEL_HALF, PIXEL_THREEQUARTERS, PIXEL_SOLID };
	vector<short> m_nExplosionAnimation{ PIXEL_SOLID, PIXEL_THREEQUARTERS, PIXEL_SOLID, PIXEL_THREEQUARTERS, PIXEL_HALF, PIXEL_QUARTER, L' ' };
	
	int m_nBlinkAnimationSpeed = 10;
	int m_nNewTileAnimationSpeed = 10;
	int m_nMergeAnimationSpeed = 10;
	int m_nExplosionAnimationSpeed = 10;

	int m_nTileSize = 5;
	int m_nFieldSize = 0;
	int m_nFieldOffsetX = 0;
	float m_fAnimationTime = 0.0f;

	bool m_bHasMoved = false;
	ROTATION m_nAnimationDirection;

protected:
	virtual bool OnUserCreate();
	virtual bool OnUserDestroy();
	virtual bool OnUserUpdate(float fElapsedTime);

private:
	int GetCellIndex(int x, int y, ROTATION nRotation = LEFT);
	void DrawCell(int nCellIndex, short nChar = PIXEL_SOLID);
	void DrawGameField();
	void ResetGameData(GAME_STATE state = GAME_STATE_TITLE);
	void ResetCell(int nCellIndex);
	vector<int> GetAvailableCells();
	void AddNewNumber(bool bAnimate = true);
	void AddNewNumber(int nValue, bool bAnimate = true);
	void AddNewNumber(int nValue, int x, int y, bool bAnimate = true);
	void GetCellColor(int nValue, short& cellColor, short& textColor, short& prevBgColor);
	void GameStateStart(float fElapsedTime);
	void GameStateTitle(float fElapsedTime);
	void GameStateAnimate(float fElapsedTime);
	bool MoveCells(ROTATION dir);
	void CalculateCellMovement(ROTATION dir);
};