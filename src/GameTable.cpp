#include <time.h>
#include <stdlib.h>
#include "GameTable.h"
#include "s3e.h"
#include "Iw2D.h"
#include "IwGx.h"
#include "game.h"


int grid_anim_frame[FRAMES_GLOW] =
{
    1, 1, 2, 2, 3, 3, 2, 2, 1, 1,
};

int grid_square_dist[16] =
{
    0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, 144, 169, 196, 225,
};

int get_grid_anim_frame(int i)
{
    if ((i >= 0) && (i < 10))
        return grid_anim_frame[i];
    else
        return 0;
}

int get_grid_anim_frame2(int i)
{
    if ((i >= 0) && (i < 5))
        return grid_anim_frame[i * 2];
    else
        return 0;
}


extern CIw2DImage* g_arrows;
extern CIwSVec2 dimension128;

void GameTable::LeftConnector::RenderConnectorL(int x, int y)
{
    CIwSVec2 scr_p, tex_p;
    //
    Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
    //
    scr_p.x = x;
    scr_p.y = y;
    // BW texcoord
    tex_p.x = 128;
    tex_p.y = 320;
    Iw2DSetColour(0xffffffff);
    Iw2DDrawImageRegion(g_arrows, scr_p, tex_p, dimension128);
    // gold texcoord
    tex_p.x = 384;
    tex_p.y = 0;
    Iw2DSetColour(0x00ffffff + ((capacity * 0xff / (MAX_CAPACITY)) << 24));
    Iw2DDrawImageRegion(g_arrows, scr_p, tex_p, dimension128);
    // red texcoord
    tex_p.x = 256;
    tex_p.y = 320;
    Iw2DSetColour(0x00ffffff + ((accumulator * 0xff / capacity) << 24));
    Iw2DDrawImageRegion(g_arrows, scr_p, tex_p, dimension128);
    //
    if (frames_incoming >= 0)
    {
        Iw2DSetColour(0xff00ff00);
        bitmapStringAt(x + 68 - frames_incoming * 4, y + 48, 20, display_bit);
    }
}

void GameTable::RightConnector::RenderConnectorR(int x, int y)
{
    CIwSVec2 scr_p, tex_p;
    //
    Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
    //
    scr_p.x = x;
    scr_p.y = y;
    // BW texcoord
    tex_p.x = 128;
    tex_p.y = 320;
    Iw2DSetColour(0xffffffff);
    Iw2DDrawImageRegion(g_arrows, scr_p, tex_p, dimension128);
    // gold texcoord
    tex_p.x = 384;
    tex_p.y = 0;
    Iw2DSetColour(0x00ffffff + ((capacity * 0xff / MAX_CAPACITY) << 24));
    Iw2DDrawImageRegion(g_arrows, scr_p, tex_p, dimension128);
    //
    if (frames_sending >= 0)
    {
        Iw2DSetColour(0xff00ff00);
        bitmapStringAt(x + 68 - frames_sending * 4, y + 32, 20, display_bit);
    }
}


// bonus texture positions
CIwSVec2 btexpos[BONUS_TYPES] = {
    CIwSVec2(256, 64),
    CIwSVec2(320, 64),
    CIwSVec2(0, 128),
    CIwSVec2(0, 192),
    CIwSVec2(0, 256),
};

// vertex, strip, UV data
CIwSVec2 bvertices[MAX_BONUS * 4];
CIwSVec2 buvdata[MAX_BONUS * 4];
CIwColour bcolors[MAX_BONUS * 4];
uint32 bsend_vertices = 0;
CIwTexture* bonus_texture = NULL;

void myIwGxInitBonus()
{
    // Create empty texture object
    bonus_texture = new CIwTexture;
    // Load image data from disk into texture
    bonus_texture->LoadFromFile("arrows.png");
    // "Upload" texture to VRAM
    bonus_texture->Upload();
    //
    bsend_vertices = 0;
}

void myIwGxPrepareBonus()
{
    Iw2DFinishDrawing();
    IwGxClear(IW_GX_DEPTH_BUFFER_F);
    //
    bsend_vertices = 0;
}

void myIwGxDrawBonus(int x, int y, CIwSVec2 texpos, iwangle rotval, int alphaf)
{
    CIwMat2D transformMatrix;
    int i;
    //
    if (bsend_vertices + 4 > (MAX_BONUS * 4))
    {
        myIwGxDoneBonus();
        bsend_vertices = 0;
    }
    transformMatrix = CIwMat2D::g_Identity;
    transformMatrix.SetRot(rotval, CIwVec2(x, y));
    //
    // 0 = top-left
    // 1 = bottom-left
    // 2 = bottom-right
    // 3 = top-right
    //
    bvertices[bsend_vertices + 0].x = x - 32;
    bvertices[bsend_vertices + 0].y = y - 32;
    bvertices[bsend_vertices + 1].x = x - 32;
    bvertices[bsend_vertices + 1].y = y + 32;
    bvertices[bsend_vertices + 2].x = x + 32;
    bvertices[bsend_vertices + 2].y = y + 32;
    bvertices[bsend_vertices + 3].x = x + 32;
    bvertices[bsend_vertices + 3].y = y - 32;
    //
    buvdata[bsend_vertices + 0].x = (texpos.x) << 3;
    buvdata[bsend_vertices + 0].y = (texpos.y) << 3;
    buvdata[bsend_vertices + 1].x = (texpos.x) << 3;
    buvdata[bsend_vertices + 1].y = (texpos.y + 64) << 3;
    buvdata[bsend_vertices + 2].x = (texpos.x + 64) << 3;
    buvdata[bsend_vertices + 2].y = (texpos.y + 64) << 3;
    buvdata[bsend_vertices + 3].x = (texpos.x + 64) << 3;
    buvdata[bsend_vertices + 3].y = (texpos.y) << 3;
    //
    bcolors[bsend_vertices + 0].Set(0xFF, 0xFF, 0xFF, (uint8)(alphaf));
    bcolors[bsend_vertices + 1].Set(0xFF, 0xFF, 0xFF, (uint8)(alphaf));
    bcolors[bsend_vertices + 2].Set(0xFF, 0xFF, 0xFF, (uint8)(alphaf));
    bcolors[bsend_vertices + 3].Set(0xFF, 0xFF, 0xFF, (uint8)(alphaf));
    //
    for (i = 0; i < 4; i++)
        bvertices[bsend_vertices + i] = transformMatrix.TransformVec(bvertices[bsend_vertices + i]);
    bsend_vertices += 4;
    //
}

void myIwGxDoneBonus()
{
    IwGxSetScreenSpaceSlot(3);
    IwGxSetVertStreamScreenSpace( bvertices, bsend_vertices );
    CIwMaterial *pMat = IW_GX_ALLOC_MATERIAL();
    pMat->SetAlphaMode( CIwMaterial::ALPHA_BLEND );
    pMat->SetTexture( bonus_texture );
    pMat->SetColAmbient( 0xFF, 0xFF, 0xFF, 0xFF );
    IwGxSetMaterial( pMat );
    IwGxSetUVStream( buvdata );
    IwGxSetColStream( bcolors, bsend_vertices );
    IwGxDrawPrims( IW_GX_QUAD_LIST, NULL, bsend_vertices );
    IwGxFlush();
}


void GameTable::BonusItem::SetBonusItem(int ti, int tj, int btype, int timeout_ms)
{
    if ((btype >= 0) && (btype < BONUS_TYPES))
    {
        enabled = 1;
        target_i = ti;
        target_j = tj;
        bonus_type = btype;
        falling_frame = 0;
        glowing_frame = -1;
        rot = 0;
        time_appeared = (int)s3eTimerGetMs();
        timeout = timeout_ms;
        oi = -1;
        oj = -1;
    }
}

void GameTable::BonusItem::UpdateBonusItem()
{
    if (enabled <= 0)
        return;
    //
    if (falling_frame >= 0)
    {
        falling_frame++;
        if (falling_frame >= (FRAMES_FALL * 2))
            falling_frame = -1;
    }
    if (glowing_frame >= 0)
    {
        glowing_frame++;
        if (glowing_frame >= FRAMES_GLOW)
            glowing_frame = -1;
    }
    else if ((rand() % 256) == 0)
    {
        glowing_frame = 0;
    }
    rot = rot + 0x1f;
    rot = rot & 0x0fff;
    if (((int)s3eTimerGetMs() - time_appeared) > timeout)
        enabled = 0;
}

void GameTable::BonusItem::DrawBonusItem(int x, int y)
{
    CIwSVec2 texpos = btexpos[bonus_type];
    int alphaf = 0xff;
    //
    if (enabled <= 0)
        return;
    //
    // adjust texture for glowing
    if ((glowing_frame >= 0) && (glowing_frame < FRAMES_GLOW) &&
        (bonus_type >= BONUS_CHARGE1) && (bonus_type <= BONUS_CHARGE3))
        texpos.x = texpos.x + get_grid_anim_frame(glowing_frame) * 64;
    //
    if (((int)s3eTimerGetMs() - time_appeared) > (timeout - 8192))
        alphaf = (time_appeared + timeout - (int)s3eTimerGetMs()) >> 5;
    if (alphaf > 0xff)
        alphaf = 0xff;
    if (alphaf < 0)
        alphaf = 0;
    //
    myIwGxDrawBonus(x, y, texpos, rot, alphaf);
}


extern CIw2DImage* g_tiles;

void GameTable::Worm::update_worm()
{
    int move, i, j, ok_worm, whead;
    if (worm_length <= 0)
    {
        return;
    }
    if (moving > 0)
    {
        moving -= 4;
        return;
    }
    if (rand() % 8 == 0)
    {
        moving = 64;
        for (i = 0; i < worm_length; i++)
        {
            worm_ox[i] = worm_x[i];
            worm_oy[i] = worm_y[i];
        }
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

void GameTable::Worm::draw_worm(int x, int y)
{
    int i, tcode;
    CIwSVec2 scr_p, tex_p;
    CIwSVec2 dimension64 = CIwSVec2(64, 64);
    if (worm_length <= 0)
        return;
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
        scr_p.x = x + (worm_x[i] << 6) - (worm_x[i] - worm_ox[i]) * moving;
        scr_p.y = y + (worm_y[i] << 6) - (worm_y[i] - worm_oy[i]) * moving;
        tex_p.x = (tcode << 6);
        tex_p.y = 256;
        Iw2DDrawImageRegion(g_tiles, scr_p,
            tex_p,
            dimension64);
    }
}


GameTable::GameTable(void)
{
    new_elements = 0;
    missing_link_elements = 0;
    can_send_connections = 0;
    fall_finished = 0;
    trigger_anim = -1;
    trigger_circle = -1;
    the_worm.SetWorm(0, 0);
    myIwGxInitBonus();
    bonus_counter = 0;
}

GameTable::~GameTable(void)
{
    delete bonus_texture;
}

void GameTable::reset_table(int percent_missing_links)
{
    int i, j;
    srand((unsigned int)s3eTimerGetUTC());
    missing_links = percent_missing_links;
    new_elements = 0;
    missing_link_elements = 0;
    can_send_connections = 0;
    fall_finished = 0;
    trigger_anim = -1;
    trigger_circle = -1;
    still_animating = ANIM_FALL;
    for (i = 0; i < GRID_W; i++)
        for (j = 0; j < GRID_H; j++)
        {
            grid_connectors[i][j] = get_new_element();
            grid_state[i][j] = CONNECT_NONE;
            grid_clickable[i][j] = 1;
            grid_anim_frame[i][j] = FRAMES_FALL;
            grid_anim_type[i][j] = ANIM_FALL;
            grid_color_shift[i][j] = 0;
        }
    for (i = 0; i < MAX_BONUS; i++)
        bonuses[i].enabled = 0;
}

// public function for returning new elements on a column
void GameTable::new_column(int x)
{
    int k, i, j;
    BonusItem *cross;
    //
    for (k = 0; k < GRID_H; k++)
    {
        grid_connectors[x][k] = get_new_element();
        grid_state[x][k] = CONNECT_NONE;
        grid_anim_frame[x][k] = FRAMES_DESTROY;
        grid_anim_type[x][k] = ANIM_DESTROY;
        //
        // kill the worm, set timeout
        if (the_worm.touches_xy(x, k))
        {
            the_worm.SetWorm(0, 0);
            worm_wait_time = (int)s3eTimerGetMs();
        }
        //
        // if there's a bonus item, destroy it
        cross = GetBonusItemAt(x, k);
        if (cross != NULL)
            cross->enabled = 0;
    }
    for (i = 0; i < GRID_W; i++)
        for (j = 0; j < GRID_H; j++)
            grid_color_shift[i][j] = 0;
    still_animating = ANIM_DESTROY;
}

// this function tries to make sure the percentage of missing links generated remains fairly constant
unsigned char GameTable::get_new_element()
{
    unsigned char k = (rand() % 15) + 1;
    new_elements++;
    // too many missing links generated so far?
    if ((100 * missing_link_elements / new_elements) > missing_links)
        // make sure this is not a missing link
        while ((k ^ 1) == 0 || (k ^ 2) == 0 || (k ^ 4) == 0 || (k ^ 8) == 0)
            k = (rand() % 15) + 1;
    // count it if it's a missing link
    if ((k ^ 1) == 0 || (k ^ 2) == 0 || (k ^ 4) == 0 || (k ^ 8) == 0)
        missing_link_elements++;
    return k;
}

// public function which "clicks" a grid element, rotating it counterclockwise
void GameTable::click_element(int x, int y)
{
    grid_connectors[x][y] = grid_element_rotate(grid_connectors[x][y]);
    trigger_circle = 0;
    tc_x = x;
    tc_y = y;
}

// public function which deletes the connected paths, and inserts new elements
// returns how many tiles were part of the "circuit" (lower 16 bits) and how many connected to the right (upper 16 bits)
int GameTable::send_connections()
{
    int i, j, k;
    int rVal = 0;
    BonusItem *cross;
    //
    if (!can_send_connections)
        return rVal;
    for (i = 0; i < GRID_W; i++)
    {
        for (j = 0; j < GRID_H; j++)
        {
            if (grid_state[i][j] == CONNECT_OK)
            {
                //
                // kill the worm, set timeout
                if (the_worm.touches_xy(i, j))
                {
                    the_worm.SetWorm(0, 0);
                    worm_wait_time = (int)s3eTimerGetMs();
                }
                //
                // if there's a bonus item, GET it
                cross = GetBonusItemAt(i, j);
                if (cross != NULL)
                {
                    switch (cross->bonus_type)
                    {
                    case BONUS_CLOCK:
                        // TODO: see what you can do with the clock
                        break;
                    case BONUS_BOMB:
                        // TODO: see what you can do with the bomb
                        break;
                    case BONUS_CHARGE1:
                        rVal += 0x000100;
                        break;
                    case BONUS_CHARGE2:
                        rVal += 0x000200;
                        break;
                    case BONUS_CHARGE3:
                        rVal += 0x000500;
                        break;
                    }
                    cross->enabled = 0;
                }
                rVal += 0x000001;
                if (i == (GRID_W - 1))
                    if (grid_connectors[i][j] & CB_RIGHT != 0)
                        rVal += 0x010000;
                // move one position down
                for (k = j; k > 0; k--)
                {
                    grid_connectors[i][k] = grid_connectors[i][k - 1];
                    grid_state[i][k] = grid_state[i][k - 1];
                    //
                    // also move bonuses down
                    cross = GetBonusItemAt(i, k);
                    if (cross != NULL)
                    {
                        if (cross->falling_frame < 0)
                        {
                            cross->falling_frame = 0;
                            cross->oi = i;
                            cross->oj = k;
                        }
                        cross->target_j++;
                    }
                }
                grid_anim_frame[i][j] = FRAMES_DESTROY;
                grid_anim_type[i][j] = ANIM_DESTROY;
                grid_connectors[i][0] = get_new_element();
                grid_state[i][0] = CONNECT_NONE;
                still_animating = ANIM_DESTROY;
            }
            grid_color_shift[i][j] = 0;
        }
    }
    return rVal;
}


// public function which deletes tiles around x, y and inserts new elements
void GameTable::bomb_table(int x, int y)
{
    int i, j, k;
    BonusItem *cross;
    //
    for (i = (x - 2); i <= (x + 2); i++)
        for (j = (y - 2); j <= (y + 2); j++)
        {
            // check boundaries
            if ((i >= 0) && (i < GRID_W) && (j >= 0) && (j < GRID_H) &&
                !((i == (x - 2)) && (j == y - 2)) &&
                !((i == (x + 2)) && (j == y - 2)) &&
                !((i == (x - 2)) && (j == y + 2)) &&
                !((i == (x + 2)) && (j == y + 2)))
            {
                //
                // kill the worm, set timeout
                if (the_worm.touches_xy(i, j))
                {
                    the_worm.SetWorm(0, 0);
                    worm_wait_time = (int)s3eTimerGetMs();
                }
                //
                // if there's a bonus item, destroy it
                cross = GetBonusItemAt(i, j);
                if (cross != NULL)
                    cross->enabled = 0;
                // move one position down
                for (k = j; k > 0; k--)
                {
                    grid_connectors[i][k] = grid_connectors[i][k - 1];
                    grid_state[i][k] = grid_state[i][k - 1];
                    //
                    // also move bonuses down
                    cross = GetBonusItemAt(i, k);
                    if (cross != NULL)
                    {
                        if (cross->falling_frame < 0)
                        {
                            cross->falling_frame = 0;
                            cross->oi = i;
                            cross->oj = k;
                        }
                        cross->target_j++;
                    }
                }
                grid_anim_frame[i][j] = FRAMES_DESTROY;
                grid_anim_type[i][j] = ANIM_DESTROY;
                grid_connectors[i][0] = get_new_element();
                grid_state[i][0] = CONNECT_NONE;
                still_animating = ANIM_DESTROY;
            }
        }
    for (i = 0; i < GRID_W; i++)
        for (j = 0; j < GRID_H; j++)
            grid_color_shift[i][j] = 0;
}

// public function which updates animations
void GameTable::update_anims()
{
    int i, j, k, anim = ANIM_NONE;
    for (j = 0; j < GRID_H; j++)
        for (i = 0; i < GRID_W; i++)
        {
            if (grid_anim_frame[i][j] > 0)
            {
                grid_anim_frame[i][j]--;
                anim = grid_anim_type[i][j];
            }
            else
            {
                // first animate destroy, then animate the fall
                if (grid_anim_type[i][j] == ANIM_DESTROY)
                {
                    // fall all of the tiles above
                    for (k = 0; k <= j; k++)
                    {
                        grid_anim_frame[i][k] = FRAMES_FALL;
                        grid_anim_type[i][k] = ANIM_FALL;
                    }
                    anim = ANIM_FALL;
                }
                else
                    grid_anim_type[i][j] = ANIM_NONE;
            }
        }
    if ((still_animating == ANIM_FALL) && (anim == ANIM_NONE))
    {
        fall_finished = 1;
    }
    still_animating = anim;
}

void GameTable::update_color_shifts()
{
    int i, j, k;
    CIwSVec2 temp;
    //
    if (fall_finished)
    {
        trigger_anim = 0;
        fall_finished = 0;
    }
    if ((trigger_anim < 40) && (trigger_anim >= 0))
    {
        for (i = 0; i < GRID_W; i++)
            for (j = 0; j < GRID_H; j++)
            {
                grid_color_shift[i][j] = get_grid_anim_frame(15 + i + j - trigger_anim);
            }
            trigger_anim++;
    }
    if (trigger_anim >= 40)
    {
        trigger_anim = -1;
        for (i = 0; i < GRID_W; i++)
            for (j = 0; j < GRID_H; j++)
                grid_color_shift[i][j] = 0;
    }
    if ((trigger_circle < 40) && (trigger_circle >= 0))
    {
        trigger_anim = -1;
        for (i = 0; i < GRID_W; i++)
            for (j = 0; j < GRID_H; j++)
            {
                // square of the distance to the center
                //k = (i - tc_x) * (i - tc_x) + (j - tc_y) * (j - tc_y);
                temp.x = (i - tc_x) << 12;
                temp.y = (j - tc_y) << 12;
                k = (temp.GetLength()) >> 12;
                grid_color_shift[i][j] = get_grid_anim_frame2(k - trigger_circle + 10);
            }
            trigger_circle++;
    }
    if (trigger_circle >= 40)
    {
        trigger_circle = -1;
        for (i = 0; i < GRID_W; i++)
            for (j = 0; j < GRID_H; j++)
                grid_color_shift[i][j] = 0;
    }
}

unsigned char GameTable::grid_element_rotate(unsigned char grid_code)
{
    unsigned char rVal = grid_code << 1;
    rVal = (rVal & 0x0f) + (rVal >> 4);
    return rVal;
}

void GameTable::expand_connections(int cx, int cy, unsigned char ctype, unsigned char marker)
{
    // test boundaries
    if ((cx < 0) || (cy < 0) || (cx >= GRID_W) || (cy >= GRID_H) || (marker >= 4))
        return;
    // check whether this square has already been visited
    if (grid_state[cx][cy] == marker)
        return;
    // mark the square, if it connects
    if ((grid_connectors[cx][cy] & ctype) != 0)
    {
        grid_state[cx][cy] = marker;
        // expand to other squares
        if ((grid_connectors[cx][cy] & CB_LEFT) != 0)
            expand_connections(cx - 1, cy, CB_RIGHT, marker);
        if ((grid_connectors[cx][cy] & CB_UP) != 0)
            expand_connections(cx, cy - 1, CB_DOWN, marker);
        if ((grid_connectors[cx][cy] & CB_RIGHT) != 0)
            expand_connections(cx + 1, cy, CB_LEFT, marker);
        if ((grid_connectors[cx][cy] & CB_DOWN) != 0)
            expand_connections(cx, cy + 1, CB_UP, marker);
    }
}

int GameTable::check_connections()
{
    int i, j, rVal = 0;
    // first, reset the grid states
    for (j = 0; j < GRID_H; j++)
        for (i = 0; i < GRID_W; i++)
            grid_state[i][j] = CONNECT_NONE;
    if (still_animating)
        return rVal;
    // check connections to the right
    for (j = 0; j < GRID_H; j++)
        if (RC[j].enabled)
            expand_connections(GRID_W - 1, j, CB_RIGHT, CONNECT_RIGHT);
    // check connections to the left
    for (j = 0; j < GRID_H; j++)
        if (LC[j].enabled)
        {
            if ((grid_state[0][j] == CONNECT_RIGHT) || (grid_state[0][j] == CONNECT_OK))
                expand_connections(0, j, CB_LEFT, CONNECT_OK);
            else
                expand_connections(0, j, CB_LEFT, CONNECT_LEFT);
            if (grid_state[0][j] == CONNECT_OK)
                rVal = 1;
        }
    can_send_connections = rVal;
    return rVal;
}
