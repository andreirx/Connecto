#include <time.h>
#include <stdlib.h>
#include <s3eSurface.h>
#include "GameTable.h"
#include "s3e.h"
#include "Iw2D.h"
#include "MatrixManager.h"


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
}

MatrixManager::~MatrixManager(void)
{
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

void MatrixManager::UpdateMatrix()
{
    int i, j;
    //
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
            matrix_intensity[i][anim_point[i] - 1] = MAX_INTENSITY / 3;
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
}

void MatrixManager::DrawMatrix()
{
    int i, j, k, mx, my, fx, fy;
    CIwSVec2 scr_pos, bmp_pos, dimension32;
    //
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
            Iw2DSetColour(0xff000000 | (matrix_intensity[i][j] << 8));
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
}

