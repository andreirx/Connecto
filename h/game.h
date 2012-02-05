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

#include "GameTable.h"
#include "LightningManager.h"
#include "MatrixManager.h"

#ifndef GAME_H
#define GAME_H

#define FPS_AVERAGE 10

#define LEVEL_TIME_RESOLUTION 4096

#define GAMESTATE_SPLASH      0x01
#define GAMESTATE_MAINMENU    0x02
#define GAMESTATE_LEVELSCREEN 0x03
#define GAMESTATE_PLAY        0x04
#define GAMESTATE_PAUSE       0x05
#define GAMESTATE_DEBRIEF     0x06
#define GAMESTATE_GAMEOVER    0x07
#define TRANSITION            0x0100

#define TRANSITION_TIME       2000
#define TRANSITION_IN         1000


void bitmapStringAt(int x, int y, int padding, char *strw);
void myIwGxDrawTile(int x, int y, CIwSVec2 texpos, iwangle rotval);
//void myIwGxDrawLetter(int x, int y, char c, int scale);


class GameLevel
{
public:
    GameLevel()
    {
        time_paused = 1;
        time_elapsed = 0;
        current_charges = 0;
        level_score = 0;
    }
    ~GameLevel()
    {
    }

    inline void SetLevelParameters(int param_time_limit,
        int param_target_charges,
        int param_bonus_frequency,
        int param_worm_length,
        int param_worm_tile_rotations)
    {
        time_limit = param_time_limit;
        target_charges = param_target_charges;
        bonus_frequency = param_bonus_frequency;
        worm_length = param_worm_length;
        worm_tile_rotations = param_worm_tile_rotations;
        time_paused = 1;
        time_elapsed = 0;
        current_charges = 0;
        level_score = 0;
    }

    inline void StartLevel()
    {
        time_paused = 0;
        time_elapsed = 0;
        current_charges = 0;
        level_score = 0;
    }

    inline void PauseLevel()
    {
        time_paused = 1;
    }

    inline void ResumeLevel()
    {
        time_paused = 0;
    }

    inline void IncrementTime(int inct)
    {
        if (!time_paused)
        {
            time_elapsed += inct;
        }
    }

    inline int GetRelativeLevelTime()
    {
        return (int)((LEVEL_TIME_RESOLUTION * time_elapsed) / time_limit);
    }

    inline int GameOver()
    {
        return (time_elapsed > time_limit);
    }

    inline void IncrementCharges(int incc)
    {
        current_charges += incc;
    }

    inline void IncrementScore(int incs)
    {
        level_score += incs;
    }

private:
    int time_limit;
    int target_charges;
    int bonus_frequency;
    int worm_length;
    int worm_tile_rotations;

    int time_elapsed;
    int time_paused;
    int current_charges;
    int level_score;
};


class CGame
{
public:
    CGame();
    ~CGame();

    GameTable *game_table;
    LightningManager *lightning;
    MatrixManager *matrix_text;

    int show_arrows, arrow_x, arrow_y;
    int table_x, table_y, can_send, can_bomb, bombing;
    int internal_frame, animation_frame;
    int send_anim_frame;

    int touchdown_x, touchdown_y;
    int touch_x, touch_y;
    int touchdown;

    int game_state;

    GameLevel current_level;
    int level, level_score, total_score, current_score, bonus, penalty, last_sent, charges;

    void get_anim_fall_y();

    // update will be called a fixed number of times per second 
    // regardless of visual framerate
    void Update(int framex)
    {
        switch (game_state)
        {
        case GAMESTATE_SPLASH:
            Update_SPLASH(framex);
            break;
        case GAMESTATE_MAINMENU:
            Update_MAINMENU(framex);
            break;
        case GAMESTATE_LEVELSCREEN:
            Update_LEVELSCREEN(framex);
            break;
        case GAMESTATE_PLAY:
            Update_PLAY(framex);
            break;
        case GAMESTATE_PAUSE:
            Update_PAUSE(framex);
            break;
        case GAMESTATE_DEBRIEF:
            Update_DEBRIEF(framex);
            break;
        case GAMESTATE_GAMEOVER:
            Update_GAMEOVER(framex);
            break;
        default:
            Update_TRANSITION(framex);
            break;
        }
    }

    // render will be called as fast as possible (but not faster 
    // than the update rate)
    void Render(int framex)
    {
        int i, FPS;
        int FPS_frame = (int)s3eTimerGetMs();
        char strbuf[256];
        //
        for (i = FPS_AVERAGE - 1; i >= 1; i--)
        {
            FPS_frames[i] = FPS_frames[i - 1];
        }
        FPS_frames[0] = FPS_frame;
        if (FPS_frames[FPS_AVERAGE - 1] != FPS_frame)
        {
            FPS = (int)(((FPS_AVERAGE - 1) * 1000) / (FPS_frame - FPS_frames[FPS_AVERAGE - 1]));
        }
        else
        {
            FPS = 0;
        }
        //
        // draw the background, whatever the game state is
        //
        Iw2DSurfaceClear(0xff000000);
        matrix_text->UpdateMatrix();
        matrix_text->DrawMatrix();
        //
        switch (game_state)
        {
        case GAMESTATE_SPLASH:
            Render_SPLASH(framex);
            break;
        case GAMESTATE_MAINMENU:
            Render_MAINMENU(framex);
            break;
        case GAMESTATE_LEVELSCREEN:
            Render_LEVELSCREEN(framex);
            break;
        case GAMESTATE_PLAY:
            Render_PLAY(framex);
            break;
        case GAMESTATE_PAUSE:
            Render_PAUSE(framex);
            break;
        case GAMESTATE_DEBRIEF:
            Render_DEBRIEF(framex);
            break;
        case GAMESTATE_GAMEOVER:
            Render_GAMEOVER(framex);
            break;
        default:
            Render_TRANSITION(framex);
            break;
        }
        Iw2DSetColour(0xff7040bf);
        sprintf(strbuf, "FPS %d", FPS);
        bitmapStringAt(Iw2DGetSurfaceWidth() - 160, 0, 20, strbuf);
        // show the surface
        //Iw2DFinishDrawing();
        Iw2DSurfaceShow();
        internal_frame++;
    }

    void SwitchGameState(int new_game_state)
    {
        // will only allow valid state changes
        switch (game_state)
        {
        case GAMESTATE_SPLASH:
            if (new_game_state == GAMESTATE_MAINMENU)
                game_state = TRANSITION + (game_state << 4) + new_game_state;
            break;
        case GAMESTATE_MAINMENU:
            if (new_game_state == GAMESTATE_LEVELSCREEN)
                game_state = TRANSITION + (game_state << 4) + new_game_state;
            break;
        case GAMESTATE_LEVELSCREEN:
            if (new_game_state == GAMESTATE_PLAY)
                game_state = TRANSITION + (game_state << 4) + new_game_state;
            break;
        case GAMESTATE_PLAY:
            if (new_game_state == GAMESTATE_PAUSE)
                game_state = TRANSITION + (game_state << 4) + new_game_state;
            if (new_game_state == GAMESTATE_DEBRIEF)
                game_state = TRANSITION + (game_state << 4) + new_game_state;
            if (new_game_state == GAMESTATE_GAMEOVER)
                game_state = TRANSITION + (game_state << 4) + new_game_state;
            break;
        case GAMESTATE_PAUSE:
            if (new_game_state == GAMESTATE_PLAY)
                game_state = TRANSITION + (game_state << 4) + new_game_state;
            if (new_game_state == GAMESTATE_MAINMENU)
                game_state = TRANSITION + (game_state << 4) + new_game_state;
            break;
        case GAMESTATE_DEBRIEF:
            if (new_game_state == GAMESTATE_LEVELSCREEN)
                game_state = TRANSITION + (game_state << 4) + new_game_state;
            break;
        case GAMESTATE_GAMEOVER:
            if (new_game_state == GAMESTATE_LEVELSCREEN)
                game_state = TRANSITION + (game_state << 4) + new_game_state;
            if (new_game_state == GAMESTATE_MAINMENU)
                game_state = TRANSITION + (game_state << 4) + new_game_state;
            break;
        default:
            break;
        }
        //
        transition_start = (int)s3eTimerGetMs();
        transition_update = 1;
    }

private:
    CIwFVec2 m_Position;
    CIwSVec2 m_Size;
    int timeout;
    int transition_start;
    int transition_update;
    int transition_shift;
    //
    int FPS_frames[FPS_AVERAGE];

    void Update_SPLASH(int framex);
    void Render_SPLASH(int framex, int shift = 0);

    void Update_MAINMENU(int framex);
    void Render_MAINMENU(int framex, int shift = 0);

    void Update_LEVELSCREEN(int framex);
    void Render_LEVELSCREEN(int framex, int shift = 0);

    void Update_PLAY(int framex);
    void Render_PLAY(int framex, int shift = 0);

    void Update_PAUSE(int framex);
    void Render_PAUSE(int framex, int shift = 0);

    void Update_DEBRIEF(int framex);
    void Render_DEBRIEF(int framex, int shift = 0);

    void Update_GAMEOVER(int framex);
    void Render_GAMEOVER(int framex, int shift = 0);

    void Update_TRANSITION(int framex);
    void Render_TRANSITION(int framex);
};

#endif
