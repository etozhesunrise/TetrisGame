#include <iostream>
#include <thread>
#include <vector>
#include <stdio.h>
#include <windows.h>
#include "Header.h"
using namespace std;


wstring tetromino[7];
int BoxWidth = 12;
int BoxHeight = 18;
unsigned char *Box = nullptr;

int ScreenWidth = 80;
int ScreenHeight = 30;

int main()
{
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"....");

	tetromino[4].append(L"..X.");
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"....");
	tetromino[5].append(L".XX.");
	tetromino[5].append(L".X..");
	tetromino[5].append(L".X..");

	tetromino[6].append(L"....");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L"..X.");
	tetromino[6].append(L"..X.");

	Box = new unsigned char[BoxWidth * BoxHeight];
	for (int x = 0; x < BoxWidth; x++)
		for (int y = 0; y < BoxHeight; y++)
			Box[y * BoxWidth + x] = (x == 0 || x == BoxWidth - 1 || y == BoxHeight - 1) ? 9 : 0;

	wchar_t *screen = new wchar_t[ScreenHeight * ScreenWidth];
	for (int i = 0; i < ScreenWidth * ScreenHeight; i++)
		screen[i] = L' ';

	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	bool bKey[4];
	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = BoxWidth / 2;
	int nCurrentY = 0;
	int nSpeed = 20;
	int nSpeedCount = 0;
	bool bForceDown = false;
	bool bRotateHold = true;
	int nPieceCount = 0;
	int nScore = 0;
	vector<int> vLines;

	bool GAMEOVER = false;
	while (!GAMEOVER)
	{
		this_thread::sleep_for(50ms);
		nSpeedCount++;
		bForceDown = (nSpeedCount == nSpeed);

		nCurrentX += (bKey[0] && Fit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && Fit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && Fit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		for (int k = 0; k < 4; k++)
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;


		if (bKey[3])
		{
			nCurrentRotation += (bRotateHold && Fit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = false;
		}
		else
			bRotateHold = true;

		if (bForceDown)
		{
			nSpeedCount = 0;
			nPieceCount++;
			if (nPieceCount % 50 == 0)
				if (nSpeed >= 10) nSpeed--;

			if (Fit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++;
			else
			{
				for (int x = 0; x < 4; x++)
					for (int y = 0; y < 4; y++)
						if (tetromino[nCurrentPiece][Rotate(x, y, nCurrentRotation)] != L'.')
							Box[(nCurrentY + y) * BoxWidth + (nCurrentX + x)] = nCurrentPiece + 1;

				for (int y = 0; y < 4; y++)
					if (nCurrentY + y < BoxHeight - 1)
					{
						bool bLine = true;
						for (int x = 1; x < BoxWidth - 1; x++)
							bLine &= (Box[(nCurrentY + y) * BoxWidth + x]) != 0;

						if (bLine)
						{
							for (int x = 1; x < BoxWidth - 1; x++)
								Box[(nCurrentY + y) * BoxWidth + x] = 8;
							vLines.push_back(nCurrentY + y);
						}
					}

				nScore += 25;
				if (!vLines.empty())	nScore += (1 << vLines.size()) * 100;

				nCurrentX = BoxWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				GAMEOVER = !Fit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
		}
		
		for (int x = 0; x < BoxWidth; x++)
			for (int y = 0; y < BoxHeight; y++)
				screen[(y + 2) * ScreenWidth + (x + 2)] = L" ABCDEFG=#"[Box[y * BoxWidth + x]];

		for (int x = 0; x < 4; x++)
			for (int y = 0; y < 4; y++)
				if (tetromino[nCurrentPiece][Rotate(x, y, nCurrentRotation)] != L'.')
					screen[(nCurrentY + y + 2) * ScreenWidth + (nCurrentX + x + 2)] = nCurrentPiece + 65;

		swprintf_s(&screen[2 * ScreenWidth + BoxWidth + 6], 16, L"SCORE: %8d", nScore);

		if (!vLines.empty())
		{
			WriteConsoleOutputCharacter(hConsole, screen, ScreenWidth * ScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);

			for (auto& v : vLines)
				for (int x = 1; x < BoxWidth - 1; x++)
				{
					for (int y = v; y > 0; y--)
						Box[y * BoxWidth + x] = Box[(y - 1) * BoxWidth + x];
					Box[x] = 0;
				}

			vLines.clear();
		}

		WriteConsoleOutputCharacter(hConsole, screen, ScreenHeight * ScreenWidth, { 0,0 }, &dwBytesWritten);
	}

	
	CloseHandle(hConsole);
	cout << "Game Over! Score:" << nScore << endl;
	system("pause");
	return 0;
}

int Rotate(int pX, int pY, int pos)
{
	int p = 0;
	switch (pos % 4)
	{
	case 0: 
		p = pY * 4 + pX;
		break;
	case 1:
		p = 12 + pY - (pX * 4);
		break;
	case 2:
		p = 15 - (pY * 4) - pX;
		break;
	case 3:
		p = 3 - pY + (pX * 4);
		break;
	}
	return p;
}

bool Fit(int nTetromino, int nRotation, int posX, int posY)
{
	for(int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++)
		{
			int pi = Rotate(x, y, nRotation);

			int b = (posY + y) * BoxWidth + (posX + x);

			if (posX + x >= 0 && posX + x < BoxWidth)
			{
				if (posY + y >= 0 && posY + y < BoxHeight)
				{
					if (tetromino[nTetromino][pi] != L'.' && Box[b] != 0)
						return false;
				}
			}
		}
	return true;
}