#include "game.h"

bool Game::Initialize (GL_Window* window, Keys* keys, DWORD tickCount)	
{
	int i;

	g_window			= window;
	g_keys				= keys;

	glEnable (GL_POINT_SMOOTH);
	glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable (GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);

	glClearColor (1, 1, 1, 1);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D (0.0, 640.0, 0.0, 480.0);

	int color [3] = {255, 255, 255}; 

	if (!bitmap [0].New (128, 16)) return false;
	if (!bitmap [1].New (128, 16)) return false;
	if (!bitmap [2].New (128, 16)) return false;
	if (!bitmap_gameover.LoadFileFromMemory (IDR_JPG1, "JPG", FALSE, NULL)) return false;
	if (!bitmap_gameover1.New (256, 16)) return false;
	if (!bitmap_title.LoadFileFromMemory (IDR_JPG2, "JPG", FALSE, NULL)) return false;
	if (!bitmap_instructions.LoadFileFromMemory (IDR_JPG3, "JPG", FALSE, NULL)) return false;

	text.SetText ("Player 1: 0");
	bitmap [0].Clear (color);
	bitmap [0].SetRenderTextFont ("Arial", 16);
	bitmap [0].SetRenderTextColor (0xff0000);
	bitmap [0].RenderText (&text, Align_Left);
	bitmap [0].GenerateTexture (texture [0]);

	text.SetText ("Player 2: 0");
	bitmap [1].Clear (color);
	bitmap [1].SetRenderTextFont ("Arial", 16);
	bitmap [1].SetRenderTextColor (0x00ff00);
	bitmap [1].RenderText (&text, Align_Left);
	bitmap [1].GenerateTexture (texture [1]);

	text.SetText ("Player 3: 0");
	bitmap [2].Clear (color);
	bitmap [2].SetRenderTextFont ("Arial", 16);
	bitmap [2].SetRenderTextColor (0x0000ff);
	bitmap [2].RenderText (&text, Align_Left);
	bitmap [2].GenerateTexture (texture [2]);

	bitmap_gameover1.Clear (color);
	bitmap_gameover1.SetRenderTextFont ("Arial", 16);
	bitmap_gameover1.SetRenderTextColor (0x0000ff);
	bitmap_gameover1.GenerateTexture (texture_gameover1);

	bitmap_gameover.GenerateTexture (texture_gameover);
	bitmap_title.GenerateTexture (texture_title);
	bitmap_instructions.GenerateTexture (texture_instructions);

	srand (tickCount);
	gameState = TitlePicture;
	spacePressed = false;
	escapePressed = false;
	keyPressed = false;

	return true;
}

void Game::NewGame ()
{
	int i, j;

	for (i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
		{
			matrix [i] [j] = 0;
			count [i] [j] = 0;
			blinkingBlocks [i] [j] = false;
		}

	for (i = 0; i < maxNumPlayers; i++)
		score [i] = 0;
	UpdateScoreDisplay ();

	cursorPosition = 0;
	currentPlayer = 1;
	numPlayers = maxNumPlayers; 
	blinking = false;
	gameState = ActualGame;
}

void Game::Deinitialize ()
{
}

void Game::Update (DWORD tickCount, DWORD lastTickCount)
{
	int i, j;

	if (escapePressed && !g_keys->keyDown [VK_ESCAPE])
	{
		if (gameState == TitlePicture)
			exit (1);
		else if (gameState == ActualGame)
		{
			escapePressed = false;
			EnterGameOver (tickCount);
		}
		else
		{
			escapePressed = false;
			gameState = TitlePicture;
		}
	}
	else if (!escapePressed && g_keys->keyDown [VK_ESCAPE])
		escapePressed = true;

	if (spacePressed && !g_keys->keyDown [VK_SPACE])
	{
		spacePressed = false;
		switch (gameState)
		{
		case GameOver:
			if (tickCount > blinkingDuration)
				gameState = TitlePicture;
			break;

		case TitlePicture:
			gameState = Instructions;
			break;

		case Instructions:
			NewGame ();
			break;
		}
	}
	else if (!spacePressed && g_keys->keyDown [VK_SPACE])
		spacePressed = true;

	switch (gameState)
	{
	case ActualGame:
		if (blinking)
		{
			if (blinkingDuration < tickCount)
			{
				int c;

				RemoveClearedBlocks ();
				for (i = 0, c = 0; i < 10 && c <= 3; i++)
					for (j = 0; j < 10 && c <= 3; j++)
					{
						ClearMarked ();
						MarkNeighborsSameColor (i, j);
						if ((c = CountMarked ()) > 3)
						{
							score [matrix [i] [j] - 1] += c * (c - 3);
							UpdateScoreDisplay ();
							MarkedBlocksShallBlink ();
							blinkingDuration = tickCount + 1000;
							blinking = true;
						}
					}
				if (c <= 3) blinking = false;
			}
			break;
		}

		if (g_keys->keyDown [VK_LEFT] && !keyPressed)
		{
			keyPressed = true;
			cursorPosition--;
			if (cursorPosition < 0)
				cursorPosition = 9;
		}
		else if (g_keys->keyDown [VK_RIGHT] && !keyPressed)
		{
			keyPressed = true;
			cursorPosition++;
			if (cursorPosition > 9)
				cursorPosition = 0;
		}
		else if (g_keys->keyDown [VK_SPACE] && !keyPressed)
		{
			keyPressed = true;
			for (i = 0; i < 10 && !matrix [i] [cursorPosition]; i++);
			i--;
			if (i >= 0)
			{
				int c;

				matrix [i] [cursorPosition] = currentPlayer;
				ClearMarked ();
				MarkNeighborsSameColor (i, cursorPosition);
				if ((c = CountMarked ()) > 3)
				{
					score [currentPlayer - 1] += c * (c - 3);
					UpdateScoreDisplay ();
					MarkedBlocksShallBlink ();
					blinkingDuration = tickCount + 1000;
					blinking = true;
				}
				currentPlayer++;
				if (currentPlayer > numPlayers)
					currentPlayer = 1;
			}
			for (i = 0; i < 10 && matrix [0] [i]; i++);
			if (i == 10) EnterGameOver (tickCount);
		}
		else if (keyPressed && !g_keys->keyDown [VK_LEFT] && !g_keys->keyDown [VK_RIGHT] && !g_keys->keyDown [VK_SPACE])
			keyPressed = false;
		break;
	}
}

void Game::Draw (DWORD tickCount)
{
	int i, j;

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (gameState)
	{
	case Instructions:
		glEnable (GL_TEXTURE_2D);
		glBindTexture (GL_TEXTURE_2D, texture_instructions);
		glColor3f (1.0f, 1.0f, 1.0f);
		glBegin (GL_QUADS);
			glTexCoord2f (0.0f, 0.0f); glVertex2f ((640 - 512) / 2, (480 - 256) / 2);
			glTexCoord2f (1.0f, 0.0f); glVertex2f ((640 - 512) / 2 + 512, (480 - 256) / 2);
			glTexCoord2f (1.0f, 1.0f); glVertex2f ((640 - 512) / 2 + 512, (480 - 256) / 2 + 256);
			glTexCoord2f (0.0f, 1.0f); glVertex2f ((640 - 512) / 2, (480 - 256) / 2 + 256);
		glEnd ();
		glDisable (GL_TEXTURE_2D);
		break;

	case TitlePicture:
		glEnable (GL_TEXTURE_2D);
		glBindTexture (GL_TEXTURE_2D, texture_title);
		glColor3f (1.0f, 1.0f, 1.0f);
		glBegin (GL_QUADS);
			glTexCoord2f (0.0f, 0.0f); glVertex2f ((640 - 256) / 2, (480 - 128) / 2);
			glTexCoord2f (1.0f, 0.0f); glVertex2f ((640 - 256) / 2 + 256, (480 - 128) / 2);
			glTexCoord2f (1.0f, 1.0f); glVertex2f ((640 - 256) / 2 + 256, (480 - 128) / 2 + 128);
			glTexCoord2f (0.0f, 1.0f); glVertex2f ((640 - 256) / 2, (480 - 128) / 2 + 128);
		glEnd ();
		glDisable (GL_TEXTURE_2D);
		break;

	case GameOver:
	case ActualGame:
		glColor3f (0, 0, 0);
		glBegin (GL_LINES);
			for (i = 0; i <= 400; i += 40)
			{
				glVertex2f (i + (640 - 400) / 2, (480 - 400) / 2);
				glVertex2f (i + (640 - 400) / 2, 400 + (480 - 400) / 2);
				glVertex2f ((640 - 400) / 2, i + (480 - 400) / 2);
				glVertex2f (400 + (640 - 400) / 2, i + (480 - 400) / 2);
			}
			SelectColor (currentPlayer);
			glVertex2f (20 + cursorPosition * 40 + (640 - 400) / 2, 470);
			glVertex2f (20 + cursorPosition * 40 + (640 - 400) / 2, 450);
			glVertex2f (20 + cursorPosition * 40 + (640 - 400) / 2, 450);
			glVertex2f (20 + cursorPosition * 40 + (640 - 400) / 2 - 5, 460);
			glVertex2f (20 + cursorPosition * 40 + (640 - 400) / 2, 450);
			glVertex2f (20 + cursorPosition * 40 + (640 - 400) / 2 + 5, 460);
		glEnd ();
		for (i = 0; i < 10; i++)
			for (j = 0; j < 10; j++)
				if (matrix [i] [j])
				{
					if (!blinking || !blinkingBlocks [i] [j] || ((int) (tickCount - blinkingDuration + 1000) / 200) % 2)
					{
						SelectColor (matrix [i] [j]);
						glBegin (GL_QUADS);
							glVertex2f (j * 40 + (640 - 400) / 2 + 1, 40 + (9 - i) * 40 + (480 - 400) / 2 - 1);
							glVertex2f (40 + j * 40 + (640 - 400) / 2 - 1, 40 + (9 - i) * 40 + (480 - 400) / 2 - 1);
							glVertex2f (40 + j * 40 + (640 - 400) / 2 - 1, (9 - i) * 40 + (480 - 400) / 2 + 1);
							glVertex2f (j * 40 + (640 - 400) / 2 + 1, (9 - i) * 40 + (480 - 400) / 2 + 1);
						glEnd ();
					}
				}
		for (i = 0; i < numPlayers; i++)
		{
			glEnable (GL_TEXTURE_2D);
			glBindTexture (GL_TEXTURE_2D, texture [i]);
			glColor3f (1.0f, 1.0f, 1.0f);
			glBegin (GL_QUADS);
				glTexCoord2f (0.0f, 0.0f); glVertex2f ((640 - 400) / 2 + 128 * i, 0);
				glTexCoord2f (1.0f, 0.0f); glVertex2f ((640 - 400) / 2 + 128 * i + 128, 0);
				glTexCoord2f (1.0f, 1.0f); glVertex2f ((640 - 400) / 2 + 128 * i + 128, 16);
				glTexCoord2f (0.0f, 1.0f); glVertex2f ((640 - 400) / 2 + 128 * i, 16);
			glEnd ();
			glDisable (GL_TEXTURE_2D);
		}
		if (gameState == GameOver)
		{
			glEnable (GL_TEXTURE_2D);
			glBindTexture (GL_TEXTURE_2D, texture_gameover);
			glColor3f (1.0f, 1.0f, 1.0f);
			glBegin (GL_QUADS);
				glTexCoord2f (0.0f, 0.0f); glVertex2f ((640 - 256) / 2, (480 - 64) / 2);
				glTexCoord2f (1.0f, 0.0f); glVertex2f (256 + (640 - 256) / 2, (480 - 64) / 2);
				glTexCoord2f (1.0f, 1.0f); glVertex2f (256 + (640 - 256) / 2, 64 + (480 - 64) / 2);
				glTexCoord2f (0.0f, 1.0f); glVertex2f ((640 - 256) / 2, 64 + (480 - 64) / 2);
			glEnd ();
			glDisable (GL_TEXTURE_2D);
			glEnable (GL_TEXTURE_2D);
			glBindTexture (GL_TEXTURE_2D, texture_gameover1);
			glColor3f (1.0f, 1.0f, 1.0f);
			glBegin (GL_QUADS);
				glTexCoord2f (0.0f, 0.0f); glVertex2f ((640 - 256) / 2, (480 - 64) / 2 - 16);
				glTexCoord2f (1.0f, 0.0f); glVertex2f (256 + (640 - 256) / 2, (480 - 64) / 2 - 16);
				glTexCoord2f (1.0f, 1.0f); glVertex2f (256 + (640 - 256) / 2, (480 - 64) / 2);
				glTexCoord2f (0.0f, 1.0f); glVertex2f ((640 - 256) / 2, (480 - 64) / 2);
			glEnd ();
			glDisable (GL_TEXTURE_2D);
		}
		break;
	}
}

void Game::SelectColor (int i)
{
	switch (i)
	{
	case 1:
		glColor3f (1, 0, 0);
		break;

	case 2:
		glColor3f (0, .8, 0);
		break;

	case 3:
		glColor3f (0, 0, .6);
		break;
	}
}

void Game::ClearMarked ()
{
	int i, j;

	for (i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
			marked [i] [j] = false;
}

void Game::MarkNeighborsSameColor (int i, int j)
{
	if (matrix [i] [j])
	{
		marked [i] [j] = true;
		if (i && matrix [i] [j] == matrix [i - 1] [j] && !marked [i - 1] [j])
			MarkNeighborsSameColor (i - 1, j);
		if (i < 9 && matrix [i] [j] == matrix [i + 1] [j] && !marked [i + 1] [j])
			MarkNeighborsSameColor (i + 1, j);
		if (j && matrix [i] [j] == matrix [i] [j - 1] && !marked [i] [j - 1])
			MarkNeighborsSameColor (i, j - 1);
		if (j < 9 && matrix [i] [j] == matrix [i] [j + 1] && !marked [i] [j + 1])
			MarkNeighborsSameColor (i, j + 1);
	}
}

int Game::CountMarked ()
{
	int i, j, n;

	for (i = 0, n = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
			if (marked [i] [j])
				n++;

	for (i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
			if (marked [i] [j])
				count [i] [j] = n;

	return n;
}

void Game::RemoveClearedBlocks ()
{
	int i, j;

	for (j = 0; j < 10; j++)
		for (i = 0; i < 10; i++)
			if (blinkingBlocks [i] [j])
				RemoveBlocks (i, j);

	for (i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
		{
			ClearMarked ();
			MarkNeighborsSameColor (i, j);
			CountMarked ();
		}
}

void Game::RemoveBlocks (int y, int x)
{
	int i;

	for (i = y; i; i--)
	{
		matrix [i] [x] = matrix [i - 1] [x];
		blinkingBlocks [i] [x] = blinkingBlocks [i - 1] [x];
	}
	matrix [0] [x] = 0;
	blinkingBlocks [0] [x] = false;
}

void Game::UpdateScoreDisplay ()
{
	int i;
	char display [20], number [20], temp [2];

	for (i = 0; i < numPlayers; i++)
	{
		strcpy (display, "Player ");
		temp [0] = i + '1';
		temp [1] = '\0';
		strcat (display, temp);
		strcat (display, ": ");
		strcat (display, itoa (score [i], number, 10));
		int color [3] = {255, 255, 255}; 
		bitmap [i].Clear (color);
		text.SetText (display);
		bitmap [i].RenderText (&text, Align_Left);
		bitmap [i].UpdateTexture (texture [i]);
	}
}

void Game::MarkedBlocksShallBlink ()
{
	int i, j;

	for (i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
			blinkingBlocks [i] [j] = marked [i] [j];
}

void Game::EnterGameOver (DWORD tickCount)
{
	char display [100], number [2];
	int max, maxIdx, i;

	strcpy (display, "And the winner is: Player ");
	for (i = 0, max = 0, maxIdx = 0; i < numPlayers; i++)
		if (score [i] > max)
		{
			max = score [i];
			maxIdx = i;
		}
	switch (maxIdx)
	{
	case 0:
		bitmap_gameover1.SetRenderTextColor (0xff0000);
		break;

	case 1:
		bitmap_gameover1.SetRenderTextColor (0x00ff00);
		break;

	case 2:
		bitmap_gameover1.SetRenderTextColor (0x0000ff);
		break;
	}
	number [0] = maxIdx + '1';
	number [1] = '\0';
	strcat (display, number);
	strcat (display, ". Congratulations!");
	int color [3] = {255, 255, 255}; 
	bitmap_gameover1.Clear (color);
	text.SetText (display);
	bitmap_gameover1.RenderText (&text, Align_Center);
	bitmap_gameover1.UpdateTexture (texture_gameover1);
	blinkingDuration = tickCount + 10000;
	gameState = GameOver;
}
