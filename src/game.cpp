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
#include <time.h>
#include <stdlib.h>
#include "s3e.h"
#include "Iw2D.h"
#include "game.h"
#include "GameTable.h"

extern CIw2DImage* g_tiles;
extern CIw2DImage* g_emoticons;
extern CIw2DImage* g_send;
extern CIw2DImage* g_arrows;
extern CIw2DImage* g_font;

extern CIwSVec2 grid_positions[GRID_W][GRID_H];
extern CIwSVec2 alt_positions[GRID_W][GRID_H];
extern CIwSVec2 anim_positions[GRID_W][GRID_H];
extern unsigned char grid_codes[16];
extern unsigned char grid_codep[16];

int first_pass = 0;
int grid_shift[GRID_W][GRID_H];
unsigned char left_set[GRID_H];
unsigned char right_set[GRID_H];
int right_multiplier[GRID_H];

// HARDCODED???
int worm_length = 5;
int worm_x[10];
int worm_y[10];


class MySprite
{
public:
    MySprite(CIw2DImage* sprImage, int x, int y, int w, int h)
    {
        myImage = sprImage;
        pos = CIwSVec2(x, y);
        size = CIwSVec2(w, h);
    }
    ~MySprite()
    {
    }

    inline void justDraw(int px, int py)
    {
        temp.x = px;
        temp.y = py;
        Iw2DDrawImageRegion(myImage, temp, pos, size);
    }

private:
    CIw2DImage* myImage;
    CIwSVec2 pos, size, temp;
};


MySprite power_sprites[10] =
{
    MySprite(g_arrows, 256, 64, 32, 32),
    MySprite(g_arrows, 288, 64, 32, 32),
    MySprite(g_arrows, 320, 64, 32, 32),
    MySprite(g_arrows, 352, 64, 32, 32),
    MySprite(g_arrows, 384, 64, 32, 32),
    MySprite(g_arrows, 416, 64, 32, 32),
    MySprite(g_arrows, 448, 64, 32, 32),
    MySprite(g_arrows, 480, 64, 32, 32),
    MySprite(g_arrows, 256, 96, 32, 32),
    MySprite(g_arrows, 288, 96, 32, 32),
};


void update_worm()
{
    int move, i, j, ok_worm, whead;
    if (rand() % 32 == 0)
    {
        whead = 0;
        ok_worm = 0;
        while (!ok_worm)
        {
            whead = rand() % 2;
            move = (1 << (rand() % 4));
            ok_worm = !(((worm_x[whead * (worm_length - 1)] == 0) && (move == CB_LEFT)) ||
                ((worm_x[whead * (worm_length - 1)] == GRID_W - 1) && (move == CB_RIGHT)) ||
                ((worm_y[whead * (worm_length - 1)] == 0) && (move == CB_UP)) ||
                ((worm_y[whead * (worm_length - 1)] == GRID_H - 1) && (move == CB_DOWN)));
            switch(move)
            {
            case CB_LEFT:
                for (i = 0; i < worm_length; i++)
                    if ((worm_x[whead * (worm_length - 1)] - 1 == worm_x[i]) &&
                        (worm_y[whead * (worm_length - 1)] == worm_y[i]))
                        ok_worm = 0;
                break;
            case CB_RIGHT:
                for (i = 0; i < worm_length; i++)
                    if ((worm_x[whead * (worm_length - 1)] + 1 == worm_x[i]) &&
                        (worm_y[whead * (worm_length - 1)] == worm_y[i]))
                        ok_worm = 0;
                break;
            case CB_UP:
                for (i = 0; i < worm_length; i++)
                    if ((worm_y[whead * (worm_length - 1)] - 1 == worm_y[i]) &&
                        (worm_x[whead * (worm_length - 1)] == worm_x[i]))
                        ok_worm = 0;
                break;
            case CB_DOWN:
                for (i = 0; i < worm_length; i++)
                    if ((worm_y[whead * (worm_length - 1)] + 1 == worm_y[i]) &&
                        (worm_x[whead * (worm_length - 1)] == worm_x[i]))
                        ok_worm = 0;
                break;
            }
        }
        if (!whead)
        {
            // move head
            for (i = worm_length - 1; i > 0; i--)
            {
                worm_x[i] = worm_x[i - 1];
                worm_y[i] = worm_y[i - 1];
            }
            switch (move)
            {
            case CB_LEFT:
                worm_x[0] = worm_x[0] - 1;
                worm_y[0] = worm_y[0];
                break;
            case CB_RIGHT:
                worm_x[0] = worm_x[0] + 1;
                worm_y[0] = worm_y[0];
                break;
            case CB_UP:
                worm_x[0] = worm_x[0];
                worm_y[0] = worm_y[0] - 1;
                break;
            case CB_DOWN:
                worm_x[0] = worm_x[0];
                worm_y[0] = worm_y[0] + 1;
                break;
            }
        }
        else
        {
            // move tail
            for (i = 0; i < worm_length - 1; i++)
            {
                worm_x[i] = worm_x[i + 1];
                worm_y[i] = worm_y[i + 1];
            }
            switch (move)
            {
            case CB_LEFT:
                worm_x[worm_length - 1] = worm_x[worm_length - 1] - 1;
                worm_y[worm_length - 1] = worm_y[worm_length - 1];
                break;
            case CB_RIGHT:
                worm_x[worm_length - 1] = worm_x[worm_length - 1] + 1;
                worm_y[worm_length - 1] = worm_y[worm_length - 1];
                break;
            case CB_UP:
                worm_x[worm_length - 1] = worm_x[worm_length - 1];
                worm_y[worm_length - 1] = worm_y[worm_length - 1] - 1;
                break;
            case CB_DOWN:
                worm_x[worm_length - 1] = worm_x[worm_length - 1];
                worm_y[worm_length - 1] = worm_y[worm_length - 1] + 1;
                break;
            }
        }
    }
}

CIwSVec2 dimension32 = CIwSVec2(32, 32);
CIwSVec2 dimension64 = CIwSVec2(64, 64);
CIwSVec2 dimension128 = CIwSVec2(128, 128);
CIwSVec2 zerozero = CIwSVec2(0, 0);

void draw_worm(int x, int y)
{
    int i, tcode;
	CIwSVec2 scr_p, tex_p;
    Iw2DSetColour(0xffffffff);
    for (i = 0; i < worm_length; i++)
    {
        tcode = 0;
        if (i == 0)
        {
            if ((worm_x[0] == worm_x[1]) && (worm_y[0] < worm_y[1]))
                tcode = 13;
            if ((worm_x[0] == worm_x[1]) && (worm_y[0] > worm_y[1]))
                tcode = 15;
            if ((worm_y[0] == worm_y[1]) && (worm_x[0] > worm_x[1]))
                tcode = 14;
            if ((worm_y[0] == worm_y[1]) && (worm_x[0] < worm_x[1]))
                tcode = 12;
        }
        if (i == worm_length - 1)
        {
            if ((worm_x[worm_length - 1] == worm_x[worm_length - 2]) &&
                (worm_y[worm_length - 1] < worm_y[worm_length - 2]))
                tcode = 13;
            if ((worm_x[worm_length - 1] == worm_x[worm_length - 2]) &&
                (worm_y[worm_length - 1] > worm_y[worm_length - 2]))
                tcode = 15;
            if ((worm_y[worm_length - 1] == worm_y[worm_length - 2]) &&
                (worm_x[worm_length - 1] < worm_x[worm_length - 2]))
                tcode = 12;
            if ((worm_y[worm_length - 1] == worm_y[worm_length - 2]) &&
                (worm_x[worm_length - 1] > worm_x[worm_length - 2]))
                tcode = 14;
        }
        if ((i > 0) && (i < worm_length - 1))
        {
            if ((worm_x[i - 1] == worm_x[i]) && (worm_x[i + 1] == worm_x[i]))
                tcode = 2;
            if ((worm_y[i - 1] == worm_y[i]) && (worm_y[i + 1] == worm_y[i]))
                tcode = 1;
            if (((worm_x[i - 1] < worm_x[i]) && (worm_y[i + 1] < worm_y[i])) ||
                ((worm_x[i + 1] < worm_x[i]) && (worm_y[i - 1] < worm_y[i])))
                tcode = 4;
            if (((worm_x[i - 1] < worm_x[i]) && (worm_y[i + 1] > worm_y[i])) ||
                ((worm_x[i + 1] < worm_x[i]) && (worm_y[i - 1] > worm_y[i])))
                tcode = 3;
            if (((worm_x[i - 1] > worm_x[i]) && (worm_y[i + 1] < worm_y[i])) ||
                ((worm_x[i + 1] > worm_x[i]) && (worm_y[i - 1] < worm_y[i])))
                tcode = 5;
            if (((worm_x[i - 1] > worm_x[i]) && (worm_y[i + 1] > worm_y[i])) ||
                ((worm_x[i + 1] > worm_x[i]) && (worm_y[i - 1] > worm_y[i])))
                tcode = 6;
        }
        if (tcode == 0)
            tcode = 11;
		scr_p.x = x + (worm_x[i] << 6);
		scr_p.y = y + (worm_y[i] << 6);
		tex_p.x = (tcode << 6);
		tex_p.y = 256;
        Iw2DDrawImageRegion(g_tiles, scr_p,
            tex_p,
            dimension64);
    }
}

CIwSVec2 destroy_frame[6] = 
{
    CIwSVec2(192, 64),
    CIwSVec2(128, 64),
    CIwSVec2(192, 64),
    CIwSVec2(128, 64),
    CIwSVec2(192, 0),
    CIwSVec2(192, 0)
};

void bitmapStringAt(int x, int y, int padding, char *strw)
{
    int currx = x, i, k, fx, fy;
	CIwSVec2 scr_pos, bmp_pos;
    if (strw)
    {
        i = 0;
        while (strw[i] != 0)
        {
            k = ((int)strw[i]) & 0x00ff;
            fx = (k & 0x0f) << 5;
            fy = (k & 0xf0) << 1;
			scr_pos.x = currx;
			scr_pos.y = y;
			bmp_pos.x = fx;
			bmp_pos.y = fy;
            Iw2DDrawImageRegion(g_font, scr_pos, bmp_pos, dimension32);
            currx += padding;
            i++;
        }
    }
}

CGame::CGame()
: m_Position(0,0)
, m_Size(Iw2DGetSurfaceHeight() / 10, Iw2DGetSurfaceHeight() / 10)
{
    int i, j;
    internal_frame = 0;
    animation_frame = 0;
    game_table = new GameTable();
    game_table->reset_table(10);
    can_send = 0;
    can_bomb = 1;
    bombing = 0;
    show_arrows = 0;
    //
    lightning = new LightningManager(1024, 1024);
    matrix_text = new MatrixManager();
    //
    level = 1;
    level_score = 0;
    total_score = 0;
    current_score = 0;
    bonus = 0;
    penalty = 0;
    last_sent = 0;
    //
    table_x = (Iw2DGetSurfaceWidth() - 640) / 2;
    table_y = (Iw2DGetSurfaceHeight() - 640) / 2;
    for (j = 0; j < GRID_H; j++)
    {
        left_set[j] = (rand() & 0x07) + ((rand() % 5) << 3);
        right_set[j] = (rand() & 0x07) + ((rand() % 5) << 3);
        right_multiplier[j] = 1;
    }
    for (i = 0; i < GRID_W; i++)
        for (j = 0; j < GRID_H; j++)
        {
            grid_positions[i][j] = CIwSVec2((i << 6) + (Iw2DGetSurfaceWidth() - 640) / 2,
                (j << 6) + (Iw2DGetSurfaceHeight() - 640) / 2);
            alt_positions[i][j] = CIwSVec2((i << 6) + (Iw2DGetSurfaceWidth() - 640) / 2,
                (j << 6) + (Iw2DGetSurfaceHeight() - 640) / 2 - 640 - ((GRID_H - j) << 6));
            anim_positions[i][j] = alt_positions[i][j];
        }
    for (i = 0; i < worm_length; i++)
    {
        worm_x[i] = 1 + i;
        worm_y[i] = 5;
    }
}


CGame::~CGame()
{
    delete game_table;
    delete lightning;
}


void CGame::Update(int framex)
{
    int i, j;
    unsigned int add_color = 0;
    // game logic goes here
    // for example, move a red square towards any touch event...
    if (game_table->is_animating())
        return;
    update_worm();
    if ((s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_PRESSED))// && (framex == 0))
    {
        if((s3ePointerGetX() >= table_x) && (s3ePointerGetY() >= table_y) &&
            (s3ePointerGetX() <= table_x + 640) && (s3ePointerGetY() <= table_y + 640))
        {
            // click on one square
            int gx = (s3ePointerGetX() - table_x) >> 6;
            int gy = (s3ePointerGetY() - table_y) >> 6;
            if (bombing)
            {
                game_table->bomb_table(gx, gy);
                bombing = 0;
                can_bomb = 0;
            }
            else
            {
                int can_click = 1, i;
                for (i = 0; i < worm_length; i++)
                    if ((gx == worm_x[i]) && (gy == worm_y[i]))
                        can_click = 0;
                if (can_click)
                {
                    game_table->click_element(gx, gy);
                    show_arrows = 1;
                    arrow_x = gx;
                    arrow_y = gy;
                }
            }
        }
        if((s3ePointerGetX() >= table_x) && (s3ePointerGetY() >= table_y + 640) &&
            (s3ePointerGetX() <= table_x + 640) && (s3ePointerGetY() <= table_y + 640 + 64))
        {
            // rewrite entire column
            int gx = (s3ePointerGetX() - table_x) >> 6;
            game_table->new_column(gx);
        }
        if (can_send)
            if ((s3ePointerGetX() >= 10) && (s3ePointerGetX() <= 118) &&
                (s3ePointerGetY() >= Iw2DGetSurfaceHeight() - 118) &&
                (s3ePointerGetY() <= Iw2DGetSurfaceHeight() - 10))
            {
                int j;
                bonus = 0;
                for (j = 0; j < GRID_H; j++)
                {
                    if (game_table->get_grid_connector(GRID_W - 1, j) & CB_RIGHT != 0)
                    {
                        bonus += right_multiplier[j];
                        right_multiplier[j]++;
                    }
                }
                current_score = game_table->send_connections();
                total_score += (current_score & 0x00ffff) * bonus;
                last_sent = ((current_score & 0xff0000) >> 16);
            }
        if (can_bomb || bombing)
            if ((s3ePointerGetX() >= Iw2DGetSurfaceWidth() - 118) &&
                (s3ePointerGetX() <= Iw2DGetSurfaceWidth() - 10) &&
                (s3ePointerGetY() >= Iw2DGetSurfaceHeight() - 118) &&
                (s3ePointerGetY() <= Iw2DGetSurfaceHeight() - 10))
            {
                if (can_bomb)
                {
                    // start bombing
                    bombing = 1;
                    can_bomb = 0;
                }
                else
                {
                    // cancel if you press again
                    bombing = 0;
                    can_bomb = 1;
                }
            }
    }
    can_send = game_table->check_connections();
    //
    // OK NOW make some lightning
    //
    //if (show_arrows || framex == 0 || internal_frame == 0)
    {
        lightning->ResetBranches();
        for (i = 0; i < GRID_W; i++)
        {
            for (j = 0; j < GRID_H; j++)
            {
                add_color = 0;
                if (game_table->get_grid_state(i, j) == CONNECT_LEFT)
                {
                    add_color = 0xffff5f00;
                }
                if (game_table->get_grid_state(i, j) == CONNECT_RIGHT)
                {
                    add_color = 0xff5f00ff;
                }
                if (game_table->get_grid_state(i, j) == CONNECT_OK)
                {
                    add_color = 0xff00bf00;
                }
                if (add_color != 0)
                {
                    if (i < GRID_W - 1)
                    {
                        if (((game_table->get_grid_connector(i, j) & CB_RIGHT) != 0) &&
                            ((game_table->get_grid_connector(i + 1, j) & CB_LEFT) != 0))
                        {
                            lightning->AddBranch_Generate(DEFAULT_LEN,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                grid_positions[i + 1][j].x + 32, grid_positions[i + 1][j].y + 32,
                                add_color);
                            lightning->AddBranch_Generate(DEFAULT_LEN,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                grid_positions[i + 1][j].x + 32, grid_positions[i + 1][j].y + 32,
                                add_color);
                        }
                    }
                    if (j < GRID_H - 1)
                    {
                        if (((game_table->get_grid_connector(i, j) & CB_DOWN) != 0) &&
                            ((game_table->get_grid_connector(i, j + 1) & CB_UP) != 0))
                        {
                            lightning->AddBranch_Generate(DEFAULT_LEN,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                grid_positions[i][j + 1].x + 32, grid_positions[i][j + 1].y + 32,
                                add_color);
                            lightning->AddBranch_Generate(DEFAULT_LEN,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                grid_positions[i][j + 1].x + 32, grid_positions[i][j + 1].y + 32,
                                add_color);
                        }
                    }
                    if (i == 0)
                    {
                        if ((game_table->get_grid_connector(i, j) & CB_LEFT) != 0)
                        {
                            lightning->AddBranch_Generate(DEFAULT_LEN,
                                grid_positions[i][j].x, grid_positions[i][j].y + 16,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                add_color);
                            lightning->AddBranch_Generate(DEFAULT_LEN,
                                grid_positions[i][j].x, grid_positions[i][j].y + 32,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                add_color);
                            lightning->AddBranch_Generate(DEFAULT_LEN,
                                grid_positions[i][j].x, grid_positions[i][j].y + 48,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                add_color);
                        }
                    }
                    if (i == GRID_W - 1)
                    {
                        if ((game_table->get_grid_connector(i, j) & CB_RIGHT) != 0)
                        {
                            lightning->AddBranch_Generate(DEFAULT_LEN,
                                grid_positions[i][j].x + 63, grid_positions[i][j].y + 16,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                add_color);
                            lightning->AddBranch_Generate(DEFAULT_LEN,
                                grid_positions[i][j].x + 63, grid_positions[i][j].y + 32,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                add_color);
                            lightning->AddBranch_Generate(DEFAULT_LEN,
                                grid_positions[i][j].x + 63, grid_positions[i][j].y + 48,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                add_color);
                        }
                    }
                }
            }
        }
    }
}

void CGame::get_anim_fall_y()
{
    int i, j, k, rVal, max, min, prevShift;
    int destroy_indices[GRID_H];
    //
    for (i = 0; i < GRID_W; i++)
    {
        for (j = 0; j < GRID_H; j++)
        {
            destroy_indices[j] = (game_table->grid_anim_type[i][j] == ANIM_DESTROY);
            grid_shift[i][j] = j;
        }
        for (j = GRID_H - 1; j >= 0; j--)
        {
            if (destroy_indices[j])
            {
                for (k = GRID_H - 1; k >= 0; k--)
                    if (grid_shift[i][k] <= j)
                        grid_shift[i][k]--;
            }
        }
    }
}

void CGame::Render(int framex)
{
    // game render goes here
    int i, j, k, c, start_destroy, end_destroy;
	int ntable_x, ntable_y;
    char strbuf[256];
	CIwSVec2 table_pos = CIwSVec2((Iw2DGetSurfaceWidth() - 640) / 2, (Iw2DGetSurfaceHeight() + 640) / 2);
	CIwSVec2 scr_p, tex_p;

    //
    ntable_x = (Iw2DGetSurfaceWidth() - 640) / 2;
    ntable_y = (Iw2DGetSurfaceHeight() - 640) / 2;
	//
	// IS IT ROTATING??
	//
	if ((ntable_x != table_x) || (ntable_y != table_y))
	{
		table_x = ntable_x;
		table_y = ntable_y;
		//
		for (i = 0; i < GRID_W; i++)
		{
			for (j = 0; j < GRID_H; j++)
			{
				grid_positions[i][j].x = (i << 6) + (Iw2DGetSurfaceWidth() - 640) / 2;
				grid_positions[i][j].y = (j << 6) + (Iw2DGetSurfaceHeight() - 640) / 2;
			}
		}
		if ((game_table->is_animating() == ANIM_DESTROY) || (game_table->is_animating() == ANIM_FALL))
		{
            for (i = 0; i < GRID_W; i++)
            {
                for (j = GRID_H - 1; j >= 0; j--)
                {
                    alt_positions[i][j].x = grid_positions[i][j].x;
                    alt_positions[i][j].y = grid_positions[i][j].y - ((j - grid_shift[i][j]) << 6);
                }
            }
		}
	}
	//
    // for example, clear to black (the order of components is ABGR)
    Iw2DSurfaceClear(0xff000000);

    Iw2DSetColour(0xffffffff);

    for (j = 0; j < GRID_H; j++)
    {
        if ((rand() & 0x1f) == 0)
            left_set[j] = (rand() & 0x07) + ((rand() % 5) << 3);
        if ((rand() & 0x1f) == 0)
            right_set[j] = (rand() & 0x07) + ((rand() % 5) << 3);
    }
    // draw the tiles
    switch (game_table->is_animating())
    {
    case ANIM_DESTROY:
        // establish starting positions for fall animations
        if (!first_pass)
        {
            get_anim_fall_y();
            for (i = 0; i < GRID_W; i++)
            {
                c = 0;
                start_destroy = -1;
                for (j = GRID_H - 1; j >= 0; j--)
                {
                    alt_positions[i][j].x = grid_positions[i][j].x;
                    alt_positions[i][j].y = grid_positions[i][j].y - ((j - grid_shift[i][j]) << 6);
                }
            }
            first_pass = 1;
        }
        // draw tiles
        for (j = 0; j < GRID_H; j++)
            for (i = 0; i < GRID_W; i++)
            {
				tex_p.x = (grid_codep[game_table->get_grid_connector(i, j)]) << 6;
				tex_p.y = (game_table->get_grid_state(i, j)) << 6;
                Iw2DDrawImageRegion(g_tiles, alt_positions[i][j],
                    tex_p,
                    dimension64);
                if (game_table->grid_anim_type[i][j] == ANIM_DESTROY)
                    Iw2DDrawImageRegion(g_arrows, grid_positions[i][j],
                        destroy_frame[game_table->grid_anim_frame[i][j]],
                        dimension64);
            }
        game_table->update_anims();
        break;
    case ANIM_FALL:
        for (j = 0; j < GRID_H; j++)
            for (i = 0; i < GRID_W; i++)
                if (game_table->grid_anim_type[i][j] == ANIM_FALL)
                {
                    anim_positions[i][j].x = grid_positions[i][j].x +
                        game_table->grid_anim_frame[i][j] * game_table->grid_anim_frame[i][j] *
                        (alt_positions[i][j].x - grid_positions[i][j].x) / (FRAMES_FALL * FRAMES_FALL);
                    anim_positions[i][j].y = grid_positions[i][j].y +
                        game_table->grid_anim_frame[i][j] * game_table->grid_anim_frame[i][j] *
                        (alt_positions[i][j].y - grid_positions[i][j].y) / (FRAMES_FALL * FRAMES_FALL);
					tex_p.x = (grid_codep[game_table->get_grid_connector(i, j)]) << 6;
					tex_p.y = (game_table->get_grid_state(i, j)) << 6;
                    Iw2DDrawImageRegion(g_tiles, anim_positions[i][j],
                        tex_p,
                        dimension64);
                }
                else
				{
					tex_p.x = (grid_codep[game_table->get_grid_connector(i, j)]) << 6;
					tex_p.y = (game_table->get_grid_state(i, j)) << 6;
                    Iw2DDrawImageRegion(g_tiles, grid_positions[i][j],
                        tex_p,
                        dimension64);
				}
        game_table->update_anims();
        break;
    default:
        first_pass = 0;
        for (j = 0; j < GRID_H; j++)
            for (i = 0; i < GRID_W; i++)
			{
				tex_p.x = (grid_codep[game_table->get_grid_connector(i, j)]) << 6;
				tex_p.y = CONNECT_NONE << 6; //(game_table->get_grid_state(i, j)) << 6;
                Iw2DDrawImageRegion(g_tiles, grid_positions[i][j],
                    tex_p,
                    dimension64);
			}
    }
    //
    // draw some lightning
    if (game_table->is_animating() == ANIM_NONE)
    {
        //Iw2DSetColour(0xffffffff);
        Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
        lightning->DrawLightning();
        //Iw2DDrawImage(lightning->destImage, CIwSVec2(0, 0), CIwSVec2(1024, 1024));
        Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
    }
    //
    // draw and update matrix text effect
    Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
    matrix_text->UpdateMatrix();
    matrix_text->DrawMatrix();
    Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
    //
    // draw the smileys and arrows
    for (j = 0; j < GRID_H; j++)
    {
        c = left_set[j];
        Iw2DSetColour(0xffff0000);
		scr_p.x = (Iw2DGetSurfaceWidth() - 640) / 2 - 64;
		scr_p.y = (j << 6) + (Iw2DGetSurfaceHeight() - 640) / 2;
		tex_p.x = (c & 0x07) << 6;
		tex_p.y = (c >> 3) << 6;
        Iw2DDrawImageRegion(g_emoticons,
            scr_p,
            tex_p,
            dimension64);
        Iw2DSetColour(0xffffffff);
        c = right_set[j];
		scr_p.x = (Iw2DGetSurfaceWidth() + 640) / 2;
		scr_p.y = (j << 6) + (Iw2DGetSurfaceHeight() - 640) / 2;
		tex_p.x = 256;
		tex_p.y = 0;
        Iw2DDrawImageRegion(g_arrows,
            scr_p,
            tex_p,
            dimension64);
        Iw2DSetColour(0xffffffff);
    }

    // draw the arrows
    for (j = 0; j < GRID_W; j++)
    {
		scr_p.x = (j << 6) + (Iw2DGetSurfaceWidth() - 640) / 2;
		scr_p.y = (Iw2DGetSurfaceHeight() + 640) / 2;
		tex_p.x = 128;
		tex_p.y = 0;
        Iw2DDrawImageRegion(g_arrows,
            scr_p,
            tex_p,
			dimension64);
    }

    // draw other elements
    if (show_arrows)
    {
		scr_p.x = (arrow_x << 6) - 32 + (Iw2DGetSurfaceWidth() - 640) / 2;
		scr_p.y = (arrow_y << 6) - 32 + (Iw2DGetSurfaceHeight() - 640) / 2;
        Iw2DDrawImageRegion(g_arrows,
            scr_p,
            zerozero,
			dimension128);
        show_arrows = 0;
    }

    // draw worm
    draw_worm((Iw2DGetSurfaceWidth() - 640) / 2, (Iw2DGetSurfaceHeight() - 640) / 2);

    // draw buttons
    if (can_send)
    {
        c = ((internal_frame & 0x0f) << 4);
        Iw2DSetColour(0xff000000 + (c << 16) + (c << 8) + c);
		scr_p.x = 0;
		scr_p.y = Iw2DGetSurfaceHeight() - 128;
        Iw2DDrawImageRegion(g_send, scr_p, zerozero, dimension128);
        Iw2DSetColour(0xffffffff);
    }
    else
	{
		scr_p.x = 0;
		scr_p.y = Iw2DGetSurfaceHeight() - 128;
		tex_p.x = 128;
		tex_p.y = 0;
        Iw2DDrawImageRegion(g_send, scr_p, tex_p, dimension128);
	}

    if (can_bomb)
	{
		scr_p.x = Iw2DGetSurfaceWidth() - 128;
		scr_p.y = Iw2DGetSurfaceHeight() - 128;
		tex_p.x = 256;
		tex_p.y = 0;
        Iw2DDrawImageRegion(g_send, scr_p, tex_p, dimension128);
	}
    else
	{
		scr_p.x = Iw2DGetSurfaceWidth() - 128;
		scr_p.y = Iw2DGetSurfaceHeight() - 128;
        if (bombing)
        {
            c = ((internal_frame & 0x0f) << 4);
            Iw2DSetColour(0xff000000 + (c << 16) + (c << 8) + c);
			tex_p.x = 256;
			tex_p.y = 0;
            Iw2DDrawImageRegion(g_send, scr_p, tex_p, dimension128);
            Iw2DSetColour(0xffffffff);
        }
        else
		{
			tex_p.x = 384;
			tex_p.y = 0;
            Iw2DDrawImageRegion(g_send, scr_p, tex_p, dimension128);
		}
	}

    // draw the top bar
    Iw2DSetColour(0xff000000);
    Iw2DFillRect(zerozero, CIwSVec2(Iw2DGetSurfaceWidth(), (Iw2DGetSurfaceHeight() - 640) / 2));

    // draw the strings
    Iw2DSetColour(0xffff7040);
    sprintf(strbuf, "Level %d", level);
    bitmapStringAt(16, 0, 20, strbuf);
    Iw2DSetColour(0xffffffff);
    sprintf(strbuf, "Score %d", total_score);
    bitmapStringAt(16, 32, 20, strbuf);
    Iw2DSetColour(0xff60ff60);
    for (j = 0; j < GRID_H; j++)
    {
        sprintf(strbuf, "x%d", right_multiplier[j]);
        bitmapStringAt(32 + (Iw2DGetSurfaceWidth() + 640) / 2, 16 + (j << 6) + (Iw2DGetSurfaceHeight() - 640) / 2, 20, strbuf);
    }

    // show the surface
    //Iw2DFinishDrawing();
    Iw2DSurfaceShow();
    internal_frame++;
}
