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
#include "IwGx.h"
#include "IwGeom.h"
#include "game.h"
#include "GameTable.h"


#define SEND_MULTIPLIER 8
#define TOTAL_RANKS     35


extern CIw2DImage* g_tiles;
extern CIw2DImage* g_emoticons;
extern CIw2DImage* g_send;
extern CIw2DImage* g_arrows;
extern CIw2DImage* g_font;
extern CIw2DImage* g_menus;

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
int touchdown_branch = -1;
int send_frame_zero = -1;
int could_send = 0;
int rotated = 0;


const char* const rank_list[TOTAL_RANKS] = {
    "Short-circuit",
    "Searcher of the path",
    "Student of the charge",
    "Bronze charge student",
    "Silver charge student",

    "Gold charge student",
    "Platinum charge student",
    "Most promising student",
    "Engineer of the charge",
    "Perfect engineer",

    "Master of the charge",
    "Swift virtual master",
    "Select pipe master",
    "Most excellent master",
    "Perfect charge master",

    "Illustrious master of the pipes",
    "Grand master charge blaster",
    "Sublime master",
    "Exalted zone master",
    "Supreme grand master",

    "Chief of the charges",
    "Wizard of the pipes",
    "Prince of the charges",
    "Supreme agent of the charge",
    "Ruler of the electrons",

    "Chosen chief",
    "Chosen wizard",
    "Chosen prince",
    "Chosen supreme agent",
    "Swiftest blast champion",

    "Supreme arc champion",
    "Commander of the champions",
    "Champion demigod",
    "God of pipes and charges",
    "Champion of the gods",
};

int generate_bonuses[10][3] = {
    { 1, 0, 0, }, // 1
    { 1, 1, 0, }, // 3
    { 2, 2, 0, }, // 6
    { 2, 2, 1, }, // 11
    { 2, 3, 2, }, // 18
    { 3, 4, 3, }, // 26
    { 4, 5, 4, }, // 34
    { 0, 7, 7, }, // 49
    { 0, 9, 9, }, // 63
    { 0, 0, 16, }, // 80
};


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


CIwSVec2 dimension32 = CIwSVec2(32, 32);
CIwSVec2 dimension64 = CIwSVec2(64, 64);
CIwSVec2 dimension128 = CIwSVec2(128, 128);
CIwSVec2 dimension512 = CIwSVec2(512, 512);
CIwSVec2 zerozero = CIwSVec2(0, 0);

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


// vertex, strip, UV data
CIwSVec2 gvertices[4];
CIwSVec2 guvdata[4];
uint32 gsend_vertices = 4;
CIwTexture* tile_texture = NULL;

void InitTileRotation()
{
    // Create empty texture object
    tile_texture = new CIwTexture;
    // Load image data from disk into texture
    tile_texture->LoadFromFile("base_tiles.png");
    // "Upload" texture to VRAM
    tile_texture->Upload();
}

void myIwGxDrawTile(int x, int y, CIwSVec2 texpos, iwangle rotval)
{
    CIwMat2D transformMatrix;
    int i;
    //
    IwGxClear(IW_GX_DEPTH_BUFFER_F);
    gsend_vertices = 4;
    //
    transformMatrix = CIwMat2D::g_Identity;
    transformMatrix.SetRot(rotval, CIwVec2(x, y));
    //
    // 0 = top-left
    // 1 = bottom-left
    // 2 = bottom-right
    // 3 = top-right
    //
    gvertices[0].x = x - 40;
    gvertices[0].y = y - 40;
    gvertices[1].x = x - 40;
    gvertices[1].y = y + 40;
    gvertices[2].x = x + 40;
    gvertices[2].y = y + 40;
    gvertices[3].x = x + 40;
    gvertices[3].y = y - 40;
    //
    guvdata[0].x = (texpos.x) << 2;
    guvdata[0].y = (texpos.y) << 3;
    guvdata[1].x = (texpos.x) << 2;
    guvdata[1].y = (texpos.y + 64) << 3;
    guvdata[2].x = (texpos.x + 64) << 2;
    guvdata[2].y = (texpos.y + 64) << 3;
    guvdata[3].x = (texpos.x + 64) << 2;
    guvdata[3].y = (texpos.y) << 3;
    //
    for (i = 0; i < 4; i++)
        gvertices[i] = transformMatrix.TransformVec(gvertices[i]);
    //
    IwGxSetScreenSpaceSlot(3);
    IwGxSetVertStreamScreenSpace( gvertices, gsend_vertices );
    CIwMaterial *pMat = IW_GX_ALLOC_MATERIAL();
    pMat->SetAlphaMode( CIwMaterial::ALPHA_BLEND );
    pMat->SetTexture( tile_texture );
    pMat->SetColAmbient( 0xFF, 0xFF, 0xFF, 0xFF );
    IwGxSetMaterial( pMat );
    IwGxSetUVStream( guvdata );
    // IwGxSetColStream( colors, quads * 4 );
    IwGxSetColStream( NULL );
    IwGxDrawPrims( IW_GX_QUAD_LIST, NULL, gsend_vertices );
    IwGxFlush();
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
    game_table->SetWormParams(7, 0, 10000);
    can_send = 0;
    can_bomb = 1;
    bombing = 0;
    show_arrows = 0;
    game_state = GAMESTATE_SPLASH;
    //
    lightning = new LightningManager(1024, 1024);
    matrix_text = new MatrixManager();
    //
    level = 1;
    level_score = 0;
    total_score = 0;
    current_score = 0;
    charges = 0;
    bonus = 0;
    penalty = 0;
    last_sent = 0;
    touchdown = 0;
    //
    InitTileRotation();
    myIwGxInitBonus();
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
    //
    for (i = 0; i < FPS_AVERAGE; i++)
    {
        FPS_frames[i] = (int)s3eTimerGetMs();
    }
}


CGame::~CGame()
{
    delete tile_texture;
    delete game_table;
    delete lightning;
}


void CGame::Update_PLAY(int framex)
{
    int i, j, k;
    unsigned int add_color = 0;
    // game logic goes here
    //
    // kill existing lightning
    lightning->ResetBranches();
    //
    // update bonus items
    game_table->UpdateBonusItems();
    //
    if (game_table->is_animating())
    {
        // don't take input, but update the necessary tasks
        //
        add_color = 0;
        // make particles for destroyed tiles
        for (i = 0; i < GRID_W; i++)
        {
            for (j = 0; j < GRID_H; j++)
            {
                if (game_table->grid_anim_type[i][j] == ANIM_DESTROY)
                {
                    //for (k = 0; k < 2; k++)
                        lightning->AddSparkle_SetXYCS(grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                        (rand() % 5120) - 2560, (rand() % 5120) - 3840,
                        ((rand() % 3) + 11) % 12,
                        (rand() % 2) + 1);
                    lightning->AddBranch_Generate(DEFAULT_LEN / 2,
                        grid_positions[i][j].x, grid_positions[i][j].y,
                        grid_positions[i][j].x + 64, grid_positions[i][j].y + 64,
                        add_color);
                    lightning->AddBranch_Generate(DEFAULT_LEN / 2,
                        grid_positions[i][j].x + 64, grid_positions[i][j].y,
                        grid_positions[i][j].x, grid_positions[i][j].y + 64,
                        add_color);
                }
            }
        }
        //
        touchdown = 0;
        return;
    }
    game_table->update_the_worm();
    if ((s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_PRESSED))// && (framex == 0))
    {
        touchdown_x = s3ePointerGetX() - table_x;
        touchdown_y = s3ePointerGetY() - table_y;
        touchdown = 1;
        if((s3ePointerGetX() >= table_x) && (s3ePointerGetY() >= table_y) &&
            (s3ePointerGetX() < table_x + 640) && (s3ePointerGetY() < table_y + 640))
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
                if (game_table->the_worm.can_click(gx, gy))
                {
                    //game_table->click_element(gx, gy);
                    //show_arrows = 1;
                    arrow_x = gx;
                    arrow_y = gy;
                }
            }
        }
    }
    if ((s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_PRESSED))
    {
        if((s3ePointerGetX() >= table_x) && (s3ePointerGetY() >= table_y + 640) &&
            (s3ePointerGetX() <= table_x + 640) && (s3ePointerGetY() <= table_y + 640 + 64))
        {
            // rewrite entire column
            int gx = (s3ePointerGetX() - table_x) >> 6;
            game_table->new_column(gx);
        }
    }
    if ((s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_PRESSED) ||
        ((framex - send_frame_zero) >= 12 * SEND_MULTIPLIER))
    {
        if (can_send)
            if (((s3ePointerGetX() >= 10) && (s3ePointerGetX() <= 118) &&
                (s3ePointerGetY() >= Iw2DGetSurfaceHeight() - 118) &&
                (s3ePointerGetY() <= Iw2DGetSurfaceHeight() - 10)) ||
                ((framex - send_frame_zero) >= 12 * SEND_MULTIPLIER))
            {
                int i, j, k;
                bonus = 0;
                for (j = 0; j < GRID_H; j++)
                {
                    if ((game_table->get_grid_connector(GRID_W - 1, j) & CB_RIGHT != 0) &&
                        (game_table->get_grid_state(GRID_W - 1, j) == CONNECT_OK))
                    {
                        bonus += right_multiplier[j];
                        right_multiplier[j]++;
                        for (k = 0; k < 3 * right_multiplier[j]; k++)
                            lightning->AddSparkle_SetXYCS(grid_positions[GRID_W - 1][j].x + 64, grid_positions[GRID_W - 1][j].y + 32,
                            (rand() % 5120) - 3840, (rand() % 5120) - 3840,
                            (rand() % 12),
                            (rand() % 3) + 1);
                    }
                }
                current_score = game_table->send_connections();
                total_score += (current_score & 0x0000ff) * bonus;
                charges += ((current_score & 0x00ff00) >> 8);
                last_sent = ((current_score & 0xff0000) >> 16);
                //
                for (i = 0; i < generate_bonuses[last_sent - 1][0]; i++)
                    game_table->AddBonusItem(BONUS_CHARGE1, 60000 - (rand() % 10000));
                //
                for (i = 0; i < generate_bonuses[last_sent - 1][1]; i++)
                    game_table->AddBonusItem(BONUS_CHARGE2, 60000 - (rand() % 10000));
                //
                for (i = 0; i < generate_bonuses[last_sent - 1][2]; i++)
                    game_table->AddBonusItem(BONUS_CHARGE3, 60000 - (rand() % 10000));
                //
                can_send = 0;
            }
    }
    if ((s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_PRESSED))
    {
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
    rotated = 0;
    if ((s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_DOWN))
        touchdown = 1;
    else
    {
        if (touchdown)
        {
            // rotate that tile!
            if (touchdown && (touchdown_x >= 0) && (touchdown_y >= 0) &&
                (touchdown_x < 640) && (touchdown_y < 640))
            {
                int dx, dy, rotations = 0;
                iwangle line_angle;
                dx = touch_x - touchdown_x;
                dy = touch_y - touchdown_y;
                if ((dx == 0 && dy == 0) || ((abs(dx) < 16) && (abs(dy) < 16)))
                    line_angle = 0x0800;
                else
                    line_angle = IwGeomAtan2(dy, dx);
                i = touchdown_x / 64;
                j = touchdown_y / 64;
                if (game_table->the_worm.can_click(i, j))
                {
                    line_angle = (line_angle + 0x600) & 0x0fff;
                    if ((line_angle >= 0x0000) && (line_angle < 0x0400))
                        rotations = 0;
                    if ((line_angle >= 0x0400) && (line_angle < 0x0800))
                        rotations = 3;
                    if ((line_angle >= 0x0800) && (line_angle < 0x0b00))
                        rotations = 2;
                    if ((line_angle >= 0x0c00) && (line_angle < 0x1000))
                        rotations = 1;
                    for (k = 0; k < rotations; k++)
                        game_table->click_element(i, j);
                    rotated = 1;
                }
            }
        }
        touchdown = 0;
    }
    if (can_send == 0)
        could_send = can_send;
    can_send = game_table->check_connections();
    if (can_send && (!could_send))
        send_frame_zero = framex;
    if (can_send && rotated)
        send_frame_zero = framex;
    could_send = can_send;
    //
    // OK NOW make some lightning
    //
    //if (show_arrows || game_table->is_animating())
    if (game_table->is_animating() == ANIM_NONE)
    {
        for (i = 0; i < GRID_W; i++)
        {
            for (j = 0; j < GRID_H; j++)
            {
                add_color = 0;
                if (game_table->get_grid_state(i, j) == CONNECT_LEFT)
                {
                    add_color = 8;//0xffff9955;
                }
                if (game_table->get_grid_state(i, j) == CONNECT_RIGHT)
                {
                    add_color = 11;//0xffaa44ff;
                }
                if (game_table->get_grid_state(i, j) == CONNECT_OK)
                {
                    add_color = ((framex - send_frame_zero) / SEND_MULTIPLIER) % 12;//0xff00bf00;
                }
                if (add_color != 0)
                {
                    if (i < GRID_W - 1)
                    {
                        if (((game_table->get_grid_connector(i, j) & CB_RIGHT) != 0) &&
                            ((game_table->get_grid_connector(i + 1, j) & CB_LEFT) != 0))
                        {
                            lightning->AddBranch_Generate(DEFAULT_LEN / 2,
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
                            lightning->AddBranch_Generate(DEFAULT_LEN / 2,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                grid_positions[i][j + 1].x + 32, grid_positions[i][j + 1].y + 32,
                                add_color);
                        }
                    }
                    if (i == 0)
                    {
                        if ((game_table->get_grid_connector(i, j) & CB_LEFT) != 0)
                        {
                            lightning->AddBranch_Generate(DEFAULT_LEN / 2,
                                grid_positions[i][j].x, grid_positions[i][j].y + 32,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                add_color);
                        }
                    }
                    if (i == GRID_W - 1)
                    {
                        if ((game_table->get_grid_connector(i, j) & CB_RIGHT) != 0)
                        {
                            lightning->AddBranch_Generate(DEFAULT_LEN / 2,
                                grid_positions[i][j].x + 63, grid_positions[i][j].y + 32,
                                grid_positions[i][j].x + 32, grid_positions[i][j].y + 32,
                                add_color);
                        }
                    }
                }
            }
        }
    }
    //else
    //{
    //    lightning->UpdateAllBranches(UPDATE_STRIKE - (UPDATE_STRIKE / 4));
    //}
    if (touchdown)
    {
        touch_x = s3ePointerGetX() - table_x;
        touch_y = s3ePointerGetY() - table_y;
        //if (touchdown_branch >= 0)
        //{
        //    lightning->KillBranch(touchdown_branch);
        //}
        if (abs(touchdown_x - touch_x) > abs(touchdown_y - touch_y))
        {
            int blen = abs(touchdown_x - touch_x) / 4;
            if (blen < 4)
                blen = 4;
            touchdown_branch = lightning->AddBranch_Generate(blen,
                touchdown_x + table_x, touchdown_y + table_y,
                touch_x + table_x, touch_y + table_y,
                0);
        }
        else
        {
            int blen = abs(touchdown_y - touch_y) / 4;
            if (blen < 4)
                blen = 4;
            touchdown_branch = lightning->AddBranch_Generate(blen,
                touchdown_x + table_x, touchdown_y + table_y,
                touch_x + table_x, touch_y + table_y,
                0);
        }
        /*
        touchdown_branch = lightning->AddBranch_Generate(DEFAULT_LEN,
            touchdown_x + table_x, touchdown_y + table_y,
            touch_x + table_x, touch_y + table_y,
            0);
        */
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

void CGame::Render_PLAY(int framex)
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
    //
    // draw and update matrix text effect
    // Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
    matrix_text->UpdateMatrix();
    matrix_text->DrawMatrix();
    // Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);

    Iw2DSetColour(0xffffffff);

    for (j = 0; j < GRID_H; j++)
    {
        if ((rand() & 0x1f) == 0)
            left_set[j] = (rand() & 0x07) + ((rand() % 5) << 3);
        if ((rand() & 0x1f) == 0)
            right_set[j] = (rand() & 0x07) + ((rand() % 5) << 3);
    }
    //
    // draw the tiles
    Iw2DFinishDrawing();
    IwGxSetScissorScreenSpace(table_x, table_y, 640, 640);
    //Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
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
                /*
                if (game_table->grid_anim_type[i][j] == ANIM_DESTROY)
                    Iw2DDrawImageRegion(g_arrows, grid_positions[i][j],
                        destroy_frame[game_table->grid_anim_frame[i][j]],
                        dimension64);
                */
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
                if (touchdown && (touchdown_x >= 0) && (touchdown_y >= 0) &&
                    (touchdown_x < 640) && (touchdown_y < 640) &&
                    ((touchdown_x / 64) == i) && ((touchdown_y / 64) == j))
                    continue;
				tex_p.x = (grid_codep[game_table->get_grid_connector(i, j)]) << 6;
				tex_p.y = (CONNECT_NONE - game_table->get_grid_color_shift(i, j)) << 6;//CONNECT_NONE << 6; //(game_table->get_grid_state(i, j)) << 6;
                Iw2DDrawImageRegion(g_tiles, grid_positions[i][j],
                    tex_p,
                    dimension64);
			}
        if (touchdown && (touchdown_x >= 0) && (touchdown_y >= 0) &&
            (touchdown_x < 640) && (touchdown_y < 640))
        {
            int dx, dy;
            iwangle line_angle;
            Iw2DFinishDrawing();
            IwGxSetScissorScreenSpace(0, 0, Iw2DGetSurfaceWidth(), Iw2DGetSurfaceHeight());
            dx = touch_x - touchdown_x;
            dy = touch_y - touchdown_y;
            if ((dx == 0 && dy == 0) || ((abs(dx) < 16) && (abs(dy) < 16)))
                line_angle = 0x800;
            else
                line_angle = IwGeomAtan2(dy, dx);
            i = touchdown_x / 64;
            j = touchdown_y / 64;
            tex_p.x = (grid_codep[game_table->get_grid_connector(i, j)]) << 6;
            tex_p.y = (0) << 6;//CONNECT_NONE << 6; //(game_table->get_grid_state(i, j)) << 6;
            myIwGxDrawTile(grid_positions[i][j].x + 32, grid_positions[i][j].y + 32, tex_p, (line_angle + 0x400) & 0x0fff);
        }
    }
    if ((framex % 2) == 0)
        game_table->update_color_shifts();
    Iw2DFinishDrawing();
    IwGxSetScissorScreenSpace(0, 0, Iw2DGetSurfaceWidth(), Iw2DGetSurfaceHeight());
    //Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
    // draw the smileys and arrows
    Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
    for (j = 0; j < GRID_H; j++)
    {
        c = left_set[j];
        Iw2DSetColour(0xffffffff);
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
    Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);

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

    //
    // draw some lightning
    lightning->DrawLightning(1);
    //
    // draw bonuses
    myIwGxPrepareBonus();
    for (j = 0; j < MAX_BONUS; j++)
    {
        GameTable::BonusItem *curr;
        curr = game_table->GetBonusItem(j);
        if (curr->enabled)
            curr->DrawBonusItem(table_x + curr->getOffsetX(), table_y + curr->getOffsetY());
    }
    myIwGxDoneBonus();

    // draw worm
    game_table->the_worm.draw_worm((Iw2DGetSurfaceWidth() - 640) / 2, (Iw2DGetSurfaceHeight() - 640) / 2);

    // draw buttons
    Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
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
    // add some particles and draw
    /*
    lightning->AddSparkle_SetXYC(Iw2DGetSurfaceWidth() / 2, Iw2DGetSurfaceHeight() / 2,
        (rand() % 5120) - 2560, (rand() % 5120) - 2560,
        (rand() % 12));
    */
    lightning->UpdateAllSparkles();
    lightning->DrawSparkles();
    Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);

    // draw the top bar
    Iw2DSetColour(0xff000000);
    // Iw2DFillRect(zerozero, CIwSVec2(Iw2DGetSurfaceWidth(), (Iw2DGetSurfaceHeight() - 640) / 2));

    // draw the strings
    Iw2DSetColour(0xffff7040);
    sprintf(strbuf, "Level %d", level);
    bitmapStringAt(16, 0, 20, strbuf);
    Iw2DSetColour(0xffffa080);
    sprintf(strbuf, "Charges %d", charges);
    bitmapStringAt(Iw2DGetSurfaceWidth() / 2, 0, 20, strbuf);
    Iw2DSetColour(0xffffa080);
    sprintf(strbuf, "Last sent %d", last_sent);
    bitmapStringAt(Iw2DGetSurfaceWidth() / 2, 32, 20, strbuf);
    Iw2DSetColour(0xffffffff);
    sprintf(strbuf, "Score %d", total_score);
    bitmapStringAt(16, 32, 20, strbuf);
    Iw2DSetColour(0xff60ff60);
    for (j = 0; j < GRID_H; j++)
    {
        sprintf(strbuf, "x%d", right_multiplier[j]);
        bitmapStringAt(32 + (Iw2DGetSurfaceWidth() + 640) / 2, 16 + (j << 6) + (Iw2DGetSurfaceHeight() - 640) / 2, 20, strbuf);
    }
}

void CGame::Update_SPLASH(int framex)
{
    if ((s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_PRESSED))
    {
        SwitchGameState(GAMESTATE_MAINMENU);
    }
}

void CGame::Render_SPLASH(int framex)
{
    CIwSVec2 scr_p, tex_p;
    //
    Iw2DSurfaceClear(0xff000000);
    //
    // draw and update matrix text effect
    // Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
    matrix_text->UpdateMatrix();
    matrix_text->DrawMatrix();
    // Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
    //
    Iw2DSetColour(0xffffffff);
    scr_p.x = (Iw2DGetSurfaceWidth() - 512) / 2;
    scr_p.y = (Iw2DGetSurfaceHeight() - 512) / 2;
    tex_p.x = 0;
    tex_p.y = 0;
    Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
    Iw2DDrawImageRegion(g_menus,
        scr_p,
        tex_p,
        dimension512);
    Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
}

void CGame::Update_MAINMENU(int framex)
{
    if ((s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_PRESSED))
    {
        SwitchGameState(GAMESTATE_LEVELSCREEN);
    }
}

void CGame::Render_MAINMENU(int framex)
{
    CIwSVec2 scr_p, tex_p;
    //
    Iw2DSurfaceClear(0xff000000);
    //
    // draw and update matrix text effect
    // Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
    matrix_text->UpdateMatrix();
    matrix_text->DrawMatrix();
    // Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
    //
    Iw2DSetColour(0xffffffff);
    scr_p.x = (Iw2DGetSurfaceWidth() - 512) / 2;
    scr_p.y = (Iw2DGetSurfaceHeight() - 512) / 2;
    tex_p.x = 512;
    tex_p.y = 0;
    Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
    Iw2DDrawImageRegion(g_menus,
        scr_p,
        tex_p,
        dimension512);
    Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
}

void CGame::Update_LEVELSCREEN(int framex)
{
    if ((s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_PRESSED))
    {
        SwitchGameState(GAMESTATE_PLAY);
    }
}

void CGame::Render_LEVELSCREEN(int framex)
{
    CIwSVec2 scr_p, tex_p;
    //
    Iw2DSurfaceClear(0xff000000);
    //
    // draw and update matrix text effect
    // Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
    matrix_text->UpdateMatrix();
    matrix_text->DrawMatrix();
    // Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
    //
    Iw2DSetColour(0xffffffff);
    scr_p.x = (Iw2DGetSurfaceWidth() - 512) / 2;
    scr_p.y = (Iw2DGetSurfaceHeight() - 512) / 2;
    tex_p.x = 0;
    tex_p.y = 512;
    Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
    Iw2DDrawImageRegion(g_menus,
        scr_p,
        tex_p,
        dimension512);
    Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
}

void CGame::Update_PAUSE(int framex)
{
    if ((s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_PRESSED))
    {
        SwitchGameState(GAMESTATE_PLAY);
    }
}

void CGame::Render_PAUSE(int framex)
{
}

void CGame::Update_DEBRIEF(int framex)
{
}

void CGame::Render_DEBRIEF(int framex)
{
}

void CGame::Update_GAMEOVER(int framex)
{
}

void CGame::Render_GAMEOVER(int framex)
{
}

void CGame::Update_TRANSITION(int framex)
{
}

void CGame::Render_TRANSITION(int framex)
{
    int trans_from = (game_state & 0x00f0) >> 4;
    int trans_to = (game_state & 0x000f);
}
