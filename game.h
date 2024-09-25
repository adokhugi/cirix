#ifndef _GAME_H_
#define _GAME_H_

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>

#include "NeHeGL.h"
#include "NeHe_Window.h"
#include "Bitmap.h"
#include "resource.h"

class Game
{
public:
	bool Initialize (GL_Window *window, Keys *keys, DWORD tickCount);
	void Deinitialize ();
	void Update (DWORD tickCount, DWORD lastTickCount);
	void Draw (DWORD tickCount);

private:
	void NewGame ();
	void SelectColor (int i);
	void ClearMarked ();
	void MarkNeighborsSameColor (int i, int j);
	int CountMarked ();
	void RemoveClearedBlocks ();
	void RemoveBlocks (int x, int y);
	void UpdateScoreDisplay ();
	void MarkedBlocksShallBlink ();
	void EnterGameOver (DWORD tickCount);

	enum GameStates
	{
		ActualGame,
		GameOver,
		TitlePicture,
		Instructions
	} gameState;

	GL_Window *g_window;
	Keys *g_keys;
 	bool spacePressed;
	bool escapePressed;
	bool keyPressed;
	int cursorPosition;
	int matrix [10] [10];
	int currentPlayer;
	int numPlayers;
	bool marked [10] [10];
	int count [10] [10];
	static const int maxNumPlayers = 3;
	int score [maxNumPlayers];
	bool blinking;
	float blinkingDuration;
	bool toBechecked [10] [10];
	TextObject text;
	Bitmap bitmap [maxNumPlayers];
	GLuint texture [maxNumPlayers];
	bool blinkingBlocks [10] [10];
	Bitmap bitmap_gameover;
	GLuint texture_gameover;
	Bitmap bitmap_gameover1;
	GLuint texture_gameover1;
	Bitmap bitmap_title;
	GLuint texture_title;
	Bitmap bitmap_instructions;
	GLuint texture_instructions;
};

#endif