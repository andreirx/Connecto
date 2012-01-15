#include <time.h>
#include <stdlib.h>
#include <s3eSurface.h>
#include "GameTable.h"
#include "s3e.h"
#include "Iw2D.h"
#include "LightningManager.h"


LightningBranch::LightningBranch(void)
{
    len_strike = DEFAULT_LEN;
    strike_x = (int *)s3eMalloc(4 * len_strike);
    strike_y = (int *)s3eMalloc(4 * len_strike);
    refstrike_x = (int *)s3eMalloc(4 * len_strike);
    refstrike_y = (int *)s3eMalloc(4 * len_strike);
}

LightningBranch::~LightningBranch(void)
{
    s3eFree(strike_x);
    s3eFree(strike_y);
    s3eFree(refstrike_x);
    s3eFree(refstrike_y);
}

void LightningBranch::SetLength(int branch_length)
{
    s3eFree(strike_x);
    s3eFree(strike_y);
    s3eFree(refstrike_x);
    s3eFree(refstrike_y);
    len_strike = branch_length;
    strike_x = (int *)s3eMalloc(4 * len_strike);
    strike_y = (int *)s3eMalloc(4 * len_strike);
    refstrike_x = (int *)s3eMalloc(4 * len_strike);
    refstrike_y = (int *)s3eMalloc(4 * len_strike);
}

int LightningBranch::GetLength()
{
    return len_strike;
}

void LightningBranch::GenerateBranch(int sx, int sy, int ex, int ey, unsigned int bcolor, int update_much)
{
    int br_0, br_1, br_2;         // how many branches 0, 1, and 2 we are making
    int i, j, k, lindex;          // indices
    int lsx, lsy, lex, ley;       // loop local start, end positions
    int ldx, ldy, lsgn;           // loop local diffs
    int keep_strike;              // on a scale from 1 to 255, how little of the main strike to keep
    int line_x;
    int line_y;
    //
    if (update_much >= UPDATE_STRIKE)
        return;
    keep_strike = update_much;
    //
    osx = sx;
    osy = sy;
    oex = ex;
    oey = ey;
    lightning_color = bcolor;
    //
    // compute internal coordinates = screen coordinates X 256
    strike_x[0] = (sx << 8);
    strike_y[0] = (sy << 8);
    strike_x[len_strike - 1] = (ex << 8);
    strike_y[len_strike - 1] = (ey << 8);
    //
    // make the main lightning branch, the "strike"
    make_branch(0, len_strike - 1, strike_x, strike_y);
    //
    // update the main strike
    if (update_much == UPDATE_ALL)
    {
        for (i = 0; i < len_strike; i++)
        {
            refstrike_x[i] = strike_x[i];
            refstrike_y[i] = strike_y[i];
        }
    }
    else
    {
        for (i = 0; i < len_strike; i++)
        {
            strike_x[i] = (refstrike_x[i]) + (((strike_x[i] - refstrike_x[i]) * (UPDATE_STRIKE - keep_strike)) >> 8);
            strike_y[i] = (refstrike_y[i]) + (((strike_y[i] - refstrike_y[i]) * (UPDATE_STRIKE - keep_strike)) >> 8);
        }
    }
}

void LightningBranch::UpdateBranch(int update_much)
{
    GenerateBranch(osx, osy, oex, oey, update_much);
}

void LightningBranch::DrawBranch_as_Lines()
{
    int j;
    CIwSVec2 start, end;
    //
    Iw2DSetColour(lightning_color);
    for (j = 1; j < len_strike; j++)
    {
        start.x = (strike_x[j - 1] >> 8);
        start.y = (strike_y[j - 1] >> 8);
        end.x = (strike_x[j] >> 8);
        end.y = (strike_y[j] >> 8);
        Iw2DDrawLine(start, end);
    }
}

void LightningBranch::make_branch(int start_index, int end_index, int *positions_x, int *positions_y)
{
    int diff_x, diff_y;                                // differences between end and start positions
    int mid_index, mid_x, mid_y;                       // index and position of the middle point between start and end
    int mid_max_x, mid_max_y, mid_min_x, mid_min_y;    // minimum and maximum displacement positions for the mid point
    int random_displacement;                           // random value to compute final displacement of the mid point
    //
    // there is no mid point, job is done, return
    if (end_index - start_index <= 1)
        return;
    //
    // compute differences between end and start positions, mid point index and initial position
    diff_x = positions_x[end_index] - positions_x[start_index];
    diff_y = positions_y[end_index] - positions_y[start_index];
    mid_index = start_index + ((end_index - start_index) >> 1);
    mid_x = positions_x[start_index] + (diff_x >> 1);
    mid_y = positions_y[start_index] + (diff_y >> 1);
    //
    // compute minimum and maximum displacement positions for the mid point
    // by taking diff_x and diff_y, switching between them and dividing by 4
    // (so that the displacement is perpendicular on the main line from start to end)
    // (but no greater than a quarter of the start to end distance, on either side of the line)
    mid_max_x = mid_x - (diff_y >> 2);
    mid_max_y = mid_y + (diff_x >> 2);
    mid_min_x = mid_x + (diff_y >> 2);
    mid_min_y = mid_y - (diff_x >> 2);
    //
    // get a random value for the displacement and compute the displaced midpoint
    random_displacement = (rand() % 256);
    mid_x = mid_min_x + (((mid_max_x - mid_min_x) * random_displacement) >> 8);
    mid_y = mid_min_y + (((mid_max_y - mid_min_y) * random_displacement) >> 8);
    //
    // update the midpoint in the vectors
    positions_x[mid_index] = mid_x;
    positions_y[mid_index] = mid_y;
    //
    // we computed the only point left between these two indices,
    // so it's pointless to compute further midpoints between consecutive indices
    if ((end_index - start_index) == 2)
        return;
    //
    // compute midpoints for the lines between start to mid, then mid to end
    make_branch(start_index, mid_index, positions_x, positions_y);
    make_branch(mid_index, end_index, positions_x, positions_y);
}


Sparkle::Sparkle(void)
{
    enabled = 0;
}

Sparkle::~Sparkle(void)
{
    enabled = 0;
}

void Sparkle::Enable_SetXY(int px, int py, int vx, int vy, unsigned int scolor)
{
    int is_x = (Iw2DGetSurfaceWidth() - SPARKLE_SPACE_W) / 2;
    int is_y = (Iw2DGetSurfaceHeight() - SPARKLE_SPACE_H) / 2;
    //
    enabled = 1;
    ss_x = (px + is_x) << SPARKLE_SHIFT;
    ss_y = (py + is_y) << SPARKLE_SHIFT;
    os_x = ss_x;
    os_y = ss_y;
    vel_x = vx;
    vel_y = vy;
    sparkle_color = scolor;
}

inline void Sparkle::UpdateSparkle()
{
    if (enabled)
    {
        vel_y = vel_y + SPARKLE_GRAVITY;
        os_x = ss_x;
        ss_x = ss_x + vel_x;
        os_y = ss_y;
        ss_y = ss_y + vel_y;
        // check left - right - bottom boundaries
        if ((ss_x < 0) || (ss_x > (SPARKLE_SPACE_W << SPARKLE_SHIFT)) || (ss_y > (SPARKLE_SPACE_H << SPARKLE_SHIFT)))
            enabled = 0;
    }
}

extern CIw2DImage* g_arrows;
CIwSVec2 dim16 = CIwSVec2(16, 16);
CIwSVec2 dim32 = CIwSVec2(32, 32);
CIwSVec2 dim64 = CIwSVec2(64, 64);
CIwSVec2 sparkles16[12] = {
    CIwSVec2(0, 448),
    CIwSVec2(16, 448),
    CIwSVec2(32, 448),
    CIwSVec2(48, 448),
    CIwSVec2(0, 464),
    CIwSVec2(16, 464),
    CIwSVec2(32, 464),
    CIwSVec2(48, 464),
    CIwSVec2(0, 480),
    CIwSVec2(16, 480),
    CIwSVec2(32, 480),
    CIwSVec2(48, 480),
};

inline void Sparkle::DrawSparkle()
{
    CIwSVec2 scr_p;
    if (enabled && (sparkle_color < 12))
    {
        scr_p.x = ((SPARKLE_SPACE_W - Iw2DGetSurfaceWidth()) >> 1) + (ss_x >> SPARKLE_SHIFT) - 8;
        scr_p.y = ((SPARKLE_SPACE_H - Iw2DGetSurfaceHeight()) >> 1) + (ss_y >> SPARKLE_SHIFT) - 8;
        Iw2DDrawImageRegion(g_arrows,
            scr_p,
            sparkles16[sparkle_color],
            dim16);
    }
}


LightningManager::LightningManager(int sW, int sH)
{
    CIw2DSurface *oldSurface = Iw2DGetSurface();
    //
    Iw2DSetUseMipMapping(false);
    destSurface = Iw2DCreateSurface(sW, sH);
    destImage = Iw2DCreateImage(destSurface);
    Iw2DSetSurface(destSurface);
    Iw2DSurfaceClear(0xff000000);
    Iw2DFinishDrawing();
    Iw2DGetSurfaceInfo(dsInfo);
    Iw2DSetSurface(oldSurface);
    total_branches = 0;
    //
    srand((unsigned int)s3eTimerGetUTC());
}

LightningManager::~LightningManager(void)
{
    delete destImage;
    delete destSurface;
}

void LightningManager::AddBranch_Generate(int branch_len, int sx, int sy, int ex, int ey, unsigned int bcolor)
{
    if (total_branches < MAX_BRANCHES)
    {
        if (lightning_branches[total_branches].GetLength() != branch_len)
        {
            lightning_branches[total_branches].SetLength(branch_len);
        }
        lightning_branches[total_branches].GenerateBranch(sx, sy, ex, ey, bcolor);
        total_branches++;
    }
}

void LightningManager::ResetBranches()
{
    total_branches = 0;
}

void LightningManager::UpdateAllBranches(int update_much)
{
    int i;
    //
    for (i = 0; i < total_branches; i++)
    {
        lightning_branches[i].UpdateBranch(update_much);
    }
}

void LightningManager::DrawLightning()
{
    int i, j, k;
    //CIw2DSurface *oldSurface = Iw2DGetSurface();
    //
    //Iw2DSetSurface(destSurface);
    //Iw2DSurfaceClear(0xff000000);
    //Iw2DSetColour(0x7f000000);
    //Iw2DFillRect(CIwSVec2(0, 0), CIwSVec2(dsInfo.m_Width, dsInfo.m_Height));
    //
    for (i = 0; i < total_branches; i++)
    {
        lightning_branches[i].DrawBranch_as_Lines();
    }
    //
    //Iw2DFinishDrawing();
    //Iw2DSetSurface(oldSurface);
}
