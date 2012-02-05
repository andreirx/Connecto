#include <time.h>
#include <stdlib.h>
#include <s3eSurface.h>
#include "GameTable.h"
#include "s3e.h"
#include "Iw2D.h"
#include "IwGx.h"
#include "MatrixManager.h"


// vertex, strip, UV data
CIwSVec2 svertices[MX_VERTICES];
CIwSVec2 suvdata[MX_VERTICES];
CIwColour scolors[MX_VERTICES];
uint32 ssend_vertices = 0;
CIwTexture* star_texture = NULL;

void myIwGxInitStars()
{
    // Create empty texture object
    star_texture = new CIwTexture;
    // Load image data from disk into texture
    star_texture->LoadFromFile("stars.png");
    // "Upload" texture to VRAM
    star_texture->Upload();
    //
    ssend_vertices = 0;
}

void myIwGxPrepareStars()
{
    Iw2DFinishDrawing();
    IwGxClear(IW_GX_DEPTH_BUFFER_F);
    //
    ssend_vertices = 0;
}

void myIwGxDrawStar(int x, int y, CIwSVec2 texpos, int iscale, iwangle rotval, int alphaf)
{
    CIwMat2D transformMatrix;
    int i;
    //
    if (ssend_vertices + 4 > (MX_VERTICES))
    {
        myIwGxDoneBonus();
        ssend_vertices = 0;
    }
    transformMatrix = CIwMat2D::g_Identity;
    transformMatrix.SetRot(rotval, CIwVec2(x, y));
    //
    // 0 = top-left
    // 1 = bottom-left
    // 2 = bottom-right
    // 3 = top-right
    //
    svertices[ssend_vertices + 0].x = x - iscale;
    svertices[ssend_vertices + 0].y = y - iscale;
    svertices[ssend_vertices + 1].x = x - iscale;
    svertices[ssend_vertices + 1].y = y + iscale;
    svertices[ssend_vertices + 2].x = x + iscale;
    svertices[ssend_vertices + 2].y = y + iscale;
    svertices[ssend_vertices + 3].x = x + iscale;
    svertices[ssend_vertices + 3].y = y - iscale;
    //
    suvdata[ssend_vertices + 0].x = (texpos.x) << 3;
    suvdata[ssend_vertices + 0].y = (texpos.y) << 3;
    suvdata[ssend_vertices + 1].x = (texpos.x) << 3;
    suvdata[ssend_vertices + 1].y = (texpos.y + 32) << 3;
    suvdata[ssend_vertices + 2].x = (texpos.x + 32) << 3;
    suvdata[ssend_vertices + 2].y = (texpos.y + 32) << 3;
    suvdata[ssend_vertices + 3].x = (texpos.x + 32) << 3;
    suvdata[ssend_vertices + 3].y = (texpos.y) << 3;
    //
    scolors[ssend_vertices + 0].Set(0xFF, 0xFF, 0xFF, (uint8)(alphaf));
    scolors[ssend_vertices + 1].Set(0xFF, 0xFF, 0xFF, (uint8)(alphaf));
    scolors[ssend_vertices + 2].Set(0xFF, 0xFF, 0xFF, (uint8)(alphaf));
    scolors[ssend_vertices + 3].Set(0xFF, 0xFF, 0xFF, (uint8)(alphaf));
    //
    for (i = 0; i < 4; i++)
        svertices[ssend_vertices + i] = transformMatrix.TransformVec(svertices[ssend_vertices + i]);
    ssend_vertices += 4;
    //
}

void myIwGxDoneStars()
{
    IwGxSetScreenSpaceSlot(3);
    IwGxSetVertStreamScreenSpace( svertices, ssend_vertices );
    CIwMaterial *pMat = IW_GX_ALLOC_MATERIAL();
    pMat->SetAlphaMode( CIwMaterial::ALPHA_ADD );
    pMat->SetTexture( star_texture );
    pMat->SetColAmbient( 0xFF, 0xFF, 0xFF, 0xFF );
    IwGxSetMaterial( pMat );
    IwGxSetUVStream( suvdata );
    IwGxSetColStream( scolors, ssend_vertices );
    IwGxDrawPrims( IW_GX_QUAD_LIST, NULL, ssend_vertices );
    IwGxFlush();
}


void MatrixManager::Star::SetStar()
{
    {
        enabled = 1;
        multiplier = 8 + (rand() % 24);
        rot_inc = (rand() % 64);
        star_hue = (rand() % 16);
        rot = 0;
        time_appeared = (int)s3eTimerGetMs();
        timeout = 2000 + (rand() % 10000);
        fx = (rand() % FIELD_RES);
        fy = (rand() % FIELD_RES);
        size_frame = 15;
    }
}

void MatrixManager::Star::UpdateStar()
{
    if (enabled <= 0)
        return;
    //
    if (((int)s3eTimerGetMs() - time_appeared) < 1000)
    {
        size_frame = ((1000 - (int)s3eTimerGetMs() + time_appeared) * 15) / 1000;
    }
    else if (((int)s3eTimerGetMs() - time_appeared) >= (timeout - 1000))
    {
        size_frame = (((int)s3eTimerGetMs() - time_appeared - timeout + 1000) * 15) / 1000;
    }
    else
    {
        size_frame = 0;
    }
    rot = rot + rot_inc;
    rot = rot & 0x0fff;
    //
    if (((int)s3eTimerGetMs() - time_appeared) > timeout)
        enabled = 0;
}

void MatrixManager::Star::DrawStar(int alphaf)
{
    CIwSVec2 texpos = CIwSVec2(star_hue << 5, size_frame << 5);
    if (enabled <= 0)
        return;
    //
    myIwGxDrawStar(fx >> 4, fy >> 4, texpos, multiplier, rot, alphaf);
}

void MatrixManager::Star::MoveStar(int dx)
{
    if (enabled <= 0)
        return;
    //
    fx = fx + (dx * multiplier);
    //
    if (fx < 0)
    {
        fx = FIELD_RES - 1;
        fy = (rand() % FIELD_RES);
        time_appeared = (int)s3eTimerGetMs() - 1000;
        timeout = 2000 + (rand() % 10000);
        rot_inc = (rand() % 64);
        star_hue = (rand() % 16);
        size_frame = 0;
    }
    if (fx >= FIELD_RES)
    {
        fx = 0;
        fy = (rand() % FIELD_RES);
        time_appeared = (int)s3eTimerGetMs() - 1000;
        timeout = 2000 + (rand() % 10000);
        rot_inc = (rand() % 64);
        star_hue = (rand() % 16);
        size_frame = 0;
    }
}


extern CIw2DImage* g_font;

MatrixManager::MatrixManager(void)
{
    int i, j;
    //
    matrix_color = MATRIX_COLOR;
    //
    for (i = 0; i < MATRIX_W; i++)
    {
        for (j = 0; j < MATRIX_H; j++)
        {
            matrix_chars[i][j] = 0;
            matrix_intensity[i][j] = 0;
            //
            matrix_keep_intensity[i][j] = 0;
            matrix_keep_text[i][j] = 0;
        }
        anim_point[i] = 0;
        anim_start[i] = (rand() % MAX_FRAME_DELAY);
    }
    //
    star_counter = 0;
    //
    myIwGxInitStars();
}

MatrixManager::~MatrixManager(void)
{
    delete star_texture;
}

void MatrixManager::SetMatrixColor(unsigned int matrixc)
{
    matrix_color = matrixc;
}

void MatrixManager::MarkingBigText(int x, int y, char *strw)
{
    int i, j, k;
    //
}

void MatrixManager::MarkingSmallText(int x, int y, char *strw)
{
    int i;
    //
    i = 0;
    while (((i + x) < MATRIX_W) && (strw[i] != 0))
    {
        matrix_keep_intensity[i + x][y] = MAX_INTENSITY;
        matrix_keep_text[i + x][y] = strw[i];
        i++;
    }
}

void MatrixManager::ClearMarkings()
{
    int i, j;
    //
    for (i = 0; i < MATRIX_W; i++)
    {
        for (j = 0; j < MATRIX_H; j++)
        {
            matrix_keep_intensity[i][j] = 0;
            matrix_keep_text[i][j] = 0;
        }
    }
}

void MatrixManager::AddStar()
{
    int internal_counter = 0;
    while (starfield[star_counter].enabled == 1)
    {
        star_counter = (star_counter + 1) % MAX_STARS;
        internal_counter++;
        // if all sparkles are already enabled, do nothing
        if (internal_counter > MAX_STARS)
            return;
    }
    starfield[star_counter].SetStar();
}

void MatrixManager::UpdateMatrix()
{
    int i, j;
    //
    if ((rand() % 4) == 0)
        AddStar();
    //
    for (i = 0; i < MAX_STARS; i++)
    {
        starfield[i].UpdateStar();
        starfield[i].MoveStar(1);
    }
    //
    /*
    for (i = 0; i < MATRIX_W; i++)
    {
        for (j = 0; j < MATRIX_H; j++)
        {
            // decrease intensity for existing chars
            if (((matrix_keep_text[i][j] != 0) || (matrix_keep_intensity[i][j] != 0)) &&
                (matrix_intensity[i][j] == MAX_INTENSITY))
            {
                // matrix_intensity[i][j] = MAX_INTENSITY;
            }
            else
            {
                if (matrix_intensity[i][j] >= 2)
                {
                    matrix_intensity[i][j] -= 2;
                }
                else
                {
                    matrix_intensity[i][j] = 0;
                }
            }
        }
        //
        if (anim_point[i] == 0)
        {
            // animation on hold
            if (anim_start[i] > 0)
            {
                // if delay is still on, decrease delay
                anim_start[i]--;
            }
            else
            {
                // if delay is elapsed, start animation
                anim_point[i]++;
            }
        }
        else
        {
            // animation in progress
            if (matrix_keep_text[i][anim_point[i] - 1] != 0)
            {
                matrix_chars[i][anim_point[i] - 1] = matrix_keep_text[i][anim_point[i] - 1];
            }
            else
            {
                matrix_chars[i][anim_point[i] - 1] = (rand() % 0xfe) + 1;
            }
            matrix_intensity[i][anim_point[i] - 1] = MAX_INTENSITY / 2;
            //
            if (anim_point[i] >= MATRIX_H)
            {
                // stop animation, allocate delay
                anim_point[i] = 0;
                anim_start[i] = (rand() % MAX_FRAME_DELAY);
            }
            else
            {
                anim_point[i]++;
            }
        }
    }
    */
}

void MatrixManager::DrawMatrix()
{
    int i, j, k, mx, my, fx, fy;
    CIwSVec2 scr_pos, bmp_pos, dimension32;
    //
    myIwGxPrepareStars();
    for (i = 0; i < MAX_STARS; i++)
    {
        starfield[i].DrawStar(0xff);
    }
    myIwGxDoneStars();
    //
    /*
    mx = ((Iw2DGetSurfaceWidth() - (MATRIX_W * CHAR_W) - (32 - CHAR_W)) / 2);
    my = ((Iw2DGetSurfaceHeight() - (MATRIX_H * CHAR_H) - (32 - CHAR_H)) / 2);
    dimension32.x = CHAR_W;
    dimension32.y = CHAR_H;
    //
    // Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
    //
    for (j = 0; j < MATRIX_H; j++)
    {
        for (i = 0; i < MATRIX_W; i++)
        {
            Iw2DSetColour(0xff000000 | (matrix_intensity[i][j] << 16));
            k = ((int)matrix_chars[i][j]) & 0x00ff;
            fx = (k & 0x0f) << 5;
            fy = (k & 0xf0) << 1;
            scr_pos.x = mx + i * CHAR_W;
            scr_pos.y = my + j * CHAR_H;
            bmp_pos.x = fx + (32 - CHAR_W) / 2;
            bmp_pos.y = fy + (32 - CHAR_H) / 2;
            Iw2DDrawImageRegion(g_font, scr_pos, bmp_pos, dimension32);
        }
    }
    // Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
    */
}

