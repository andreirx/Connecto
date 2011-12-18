/*
 * This file is part of the Marmalade SDK Code Samples.
 *
 * Copyright (C) 2001-2011 Ideaworks3D Ltd.
 * All Rights Reserved.
 *
 * This source code is intended only as a supplement to Ideaworks Labs
 * Development Tools and/or on-line documentation.
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */
#include "s3e.h"
#include "Iw2D.h"
#include "game.h"
#include "GameTable.h"

// updates per second
#define UPS 60

// throttle frame time at 10 fps (i.e. the game will slow down rather
// than having very low frame rate)
#define MAX_UPDATES 6

CIw2DImage* g_tiles;
CIw2DImage* g_emoticons;
CIw2DImage* g_send;
CIw2DImage* g_arrows;
CIw2DImage* g_font;

CIwSVec2 grid_positions[GRID_W][GRID_H];
CIwSVec2 alt_positions[GRID_W][GRID_H];
CIwSVec2 anim_positions[GRID_W][GRID_H];

// bit 0: right, bit 1: up, bit 2: left, bit 3: down
unsigned char grid_codes[16] = {0, 5, 10, 12, 6, 3, 9, 7, 11, 13, 14, 15, 1, 8, 4, 2};
unsigned char grid_codep[16] = {0, 12, 15, 5, 14, 1, 4, 7, 13, 6, 2, 8, 3, 9, 10, 11};

int GetUpdateFrame()
{
    return (int)(s3eTimerGetMs() / (1000/UPS));
}

// "main" is the S3E entry point
int main()
{
	Iw2DInit();
    Iw2DSetUseMipMapping(false);

    g_tiles = Iw2DCreateImage("base_tiles.png");
    g_emoticons = Iw2DCreateImage("emoticons.png");
    g_send = Iw2DCreateImage("send_button.png");
    g_arrows = Iw2DCreateImage("arrows.png");
    g_font = Iw2DCreateImage("font_bitmap.png");

    // create game object
    CGame* pGame = new CGame;

    int currentUpdate = GetUpdateFrame();
    int nextUpdate = currentUpdate;
    int current_frame = 0;

    // to exit correctly, applications should poll for quit requests
	while(!s3eDeviceCheckQuitRequest())
	{
	    // run logic at a fixed frame rate (defined by UPS)
        
        // block until the next frame (don't render unless at 
        // least one update has occurred)
        while(!s3eDeviceCheckQuitRequest())
        {
            nextUpdate = GetUpdateFrame();
            if( nextUpdate != currentUpdate )
                break;
            s3eDeviceYield(1);
        }
        
        // execute update steps
        int frames = nextUpdate - currentUpdate;
        frames = MIN(MAX_UPDATES, frames);
        while(frames--)
        {
            pGame->Update(frames);
        }
        currentUpdate = nextUpdate;
        
        // render the results
        pGame->Render(current_frame);
        current_frame++;

		// if an application uses polling input the application 
        // must call update once per frame
        s3ePointerUpdate();
        s3eKeyboardUpdate();

        // S3E applications should yield frequently
		s3eDeviceYield();
	}

    // clear up game object
    delete pGame;

    delete g_tiles;
    delete g_emoticons;
    delete g_send;
    delete g_arrows;
    delete g_font;

	Iw2DTerminate();

	return 0;
}
