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

#define LEVEL_TIME_RESOLUTION 4096

class GameLevel
{
public:
    GameLevel()
    {
        time_paused = 1;
        time_elapsed = 0;
        current_charges = 0;
    }
    ~GameLevel()
    {
    }

    inline void StartLevel()
    {
        time_paused = 0;
        time_elapsed = 0;
        current_charges = 0;
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

    inline int IncrementCharges(int incc)
    {
        current_charges += incc;
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

    int level, level_score, total_score, current_score, bonus, penalty, last_sent;

    void get_anim_fall_y();
    // update will be called a fixed number of times per second 
    // regardless of visual framerate
    void Update(int framex);
    // render will be called as fast as possible (but not faster 
    // than the update rate)
    void Render(int framex);

private:
    CIwFVec2 m_Position;
    CIwSVec2 m_Size;
};

#endif
