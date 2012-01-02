#include <time.h>
#include <stdlib.h>
#include <s3eSurface.h>
#include "GameTable.h"
#include "s3e.h"
#include "Iw2D.h"
#include "LightningManager.h"


LightningBranch::LightningBranch(int branch_length)
{
    len_strike = branch_length;
    strike_x = (int *)s3eMalloc(4 * branch_length);
    strike_y = (int *)s3eMalloc(4 * branch_length);
    refstrike_x = (int *)s3eMalloc(4 * branch_length);
    refstrike_y = (int *)s3eMalloc(4 * branch_length);
}

LightningBranch::~LightningBranch(void)
{
    s3eFree(strike_x);
    s3eFree(strike_y);
    s3eFree(refstrike_x);
    s3eFree(refstrike_y);
}

void LightningBranch::GenerateBranch(int sx, int sy, int ex, int ey, int update_much)
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
    srand((unsigned int)s3eTimerGetUTC());
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

void LightningBranch::DrawBranch_as_Lines()
{
    int j;
    //
    for (j = 1; j < len_strike; j++)
    {
        Iw2DDrawLine(CIwSVec2(strike_x[j - 1], strike_y[j - 1]), CIwSVec2(strike_x[j], strike_y[j]));
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
}

LightningManager::~LightningManager(void)
{
    delete destImage;
    delete destSurface;
}

void LightningManager::GenerateLightning(int sx, int sy, int ex, int ey, int update_much)
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
    srand((unsigned int)s3eTimerGetUTC());
    //
    // reset branch pointers
    if (update_much == UPDATE_ALL)
    {
        for (i = 0; i < LEN_STRIKE; i++)
            strike_branch[i] = NO_BRANCH;
        for (i = 0; i < BRANCHES_0; i++)
            for (j = 0; j < LEN_BRANCH_0; j++)
                branch_0_branch[i][j] = NO_BRANCH;
        for (i = 0; i < BRANCHES_1; i++)
            for (j = 0; j < LEN_BRANCH_1; j++)
                branch_1_branch[i][j] = NO_BRANCH;
    }
    //
    // compute internal coordinates = screen coordinates X 256
    strike_x[0] = (sx << 8);
    strike_y[0] = (sy << 8);
    strike_x[LEN_STRIKE - 1] = (ex << 8);
    strike_y[LEN_STRIKE - 1] = (ey << 8);
    //
    // make the main lightning branch, the "strike"
    make_branch(0, LEN_STRIKE - 1, strike_x, strike_y);
    //
    // update the main strike
    if (update_much == UPDATE_ALL)
    {
        for (i = 0; i < LEN_STRIKE; i++)
        {
            refstrike_x[i] = strike_x[i];
            refstrike_y[i] = strike_y[i];
        }
    }
    else
    {
        for (i = 0; i < LEN_STRIKE; i++)
        {
            strike_x[i] = (refstrike_x[i]) + (((strike_x[i] - refstrike_x[i]) * (UPDATE_STRIKE - keep_strike)) >> 8);
            strike_y[i] = (refstrike_y[i]) + (((strike_y[i] - refstrike_y[i]) * (UPDATE_STRIKE - keep_strike)) >> 8);
        }
    }
    //
    // see how many branches 0 to generate
    if (update_much == UPDATE_ALL)
    {
        br_0 = rand() % BRANCHES_0;
        generated_branches_0 = 0;
    }
    else
    {
        br_0 = generated_branches_0;
        generated_branches_0 = 0;
    }
    for (i = 0; i < br_0; i++)
    {
        if (update_much == UPDATE_ALL)
        {
            //
            // get some point between 0.25 and 0.75 of the main strike to branch, make it start position
            do
            {
                lindex = (LEN_STRIKE >> 2) + (rand() % (LEN_STRIKE >> 1));
            }
            while (strike_branch[lindex] != NO_BRANCH);
        }
        else
        {
            lindex = 0;
            while ((strike_branch[lindex] != i + BRANCH_0_OFFSET) && (lindex < LEN_STRIKE))
                lindex++;
        }
        lsx = strike_x[lindex];
        lsy = strike_y[lindex];
        //
        // get some direction perpendicular on the local strike slope, determine end point
        ldx = strike_x[lindex + 2] - strike_x[lindex - 2];
        ldy = strike_y[lindex + 2] - strike_y[lindex - 2];
        lsgn = ((rand() % 2) << 1) - 1;
        lex = lsx - lsgn * (ldy << 4);
        ley = lsy + lsgn * (ldx << 4);
        branch_0_x[i][0] = lsx;
        branch_0_y[i][0] = lsy;
        branch_0_x[i][LEN_BRANCH_0 - 1] = lex;
        branch_0_y[i][LEN_BRANCH_0 - 1] = ley;
        //
        // okay, generate branch and assign it to the main strike
        make_branch(0, LEN_BRANCH_0 - 1, branch_0_x[i], branch_0_y[i]);
        strike_branch[lindex] = BRANCH_0_OFFSET + i;
        //
        // note one more generated branch 0 and move on
        generated_branches_0++;
    }
    //
    // see how many branches 1 to generate
    generated_branches_1 = 0;
    br_1 = rand() % BRANCHES_1;
    //
    // see how many branches 2 to generate
    generated_branches_2 = 0;
    br_2 = rand() % BRANCHES_2;
    //
    // convert everything to screen space = coordinates / 256
    for (i = 0; i < LEN_STRIKE; i++)
    {
        strike_x[i] >>= 8;
        strike_y[i] >>= 8;
        //
        for (j = 0; j < BRANCHES_2; j++)
        {
            if ((j < BRANCHES_0) && (i < LEN_BRANCH_0))
            {
                branch_0_x[j][i] >>= 8;
                branch_0_y[j][i] >>= 8;
            }
            if ((j < BRANCHES_1) && (i < LEN_BRANCH_1))
            {
                branch_1_x[j][i] >>= 8;
                branch_1_y[j][i] >>= 8;
            }
            if (i < LEN_BRANCH_2)
            {
                branch_2_x[j][i] >>= 8;
                branch_2_y[j][i] >>= 8;
            }
        }
    }
}

void LightningManager::DrawLightning()
{
    int i, j, k;
    CIwSVec2 localBox[4];
    CIw2DSurface *oldSurface = Iw2DGetSurface();
    //
    Iw2DSetSurface(destSurface);
    //Iw2DSurfaceClear(0xff000000);
    //blur_image();
    Iw2DSetColour(0x7f000000);
    Iw2DFillRect(CIwSVec2(0, 0), CIwSVec2(dsInfo.m_Width, dsInfo.m_Height));
    Iw2DSetColour(0xffffffff);
    //
    i = 1;
    localBox[0] = CIwSVec2(strike_x[i - 1], strike_y[i - 1]);
    localBox[1] = CIwSVec2(strike_x[i - 1], strike_y[i - 1]);
    localBox[2] = CIwSVec2(strike_x[i] - (strike_y[i] - strike_y[i - 1]), strike_y[i] + (strike_x[i] - strike_x[i - 1]));
    localBox[3] = CIwSVec2(strike_x[i], strike_y[i]);
    Iw2DFillPolygon(localBox, 4);
    for (i = 2; i < LEN_STRIKE; i++)
    {
        localBox[0] = CIwSVec2(strike_x[i - 1], strike_y[i - 1]);
        localBox[1] = CIwSVec2(strike_x[i - 1] - (strike_y[i - 1] - strike_y[i - 2]), strike_y[i - 1] + (strike_x[i - 1] - strike_x[i - 2]));
        localBox[2] = CIwSVec2(strike_x[i] - (strike_y[i] - strike_y[i - 1]), strike_y[i] + (strike_x[i] - strike_x[i - 1]));
        localBox[3] = CIwSVec2(strike_x[i], strike_y[i]);
        Iw2DFillPolygon(localBox, 4);
        //Iw2DDrawLine(CIwSVec2(strike_x[i - 1], strike_y[i - 1]), CIwSVec2(strike_x[i], strike_y[i]));
        if ((strike_branch[i] / BRANCH_STEP) == (BRANCH_0_OFFSET / BRANCH_STEP))
        {
            k = strike_branch[i] - BRANCH_0_OFFSET;
            for (j = 1; j < LEN_BRANCH_0; j++)
            {
                Iw2DDrawLine(CIwSVec2(branch_0_x[k][j - 1], branch_0_y[k][j - 1]), CIwSVec2(branch_0_x[k][j], branch_0_y[k][j]));
            }
        }
    }
    Iw2DSetSurface(oldSurface);
}

void LightningManager::make_branch(int start_index, int end_index, int *positions_x, int *positions_y)
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

void LightningManager::blur_image()
{
    int sw, sh, sp;
    int *data;
    int i, j, idx, idxl;
    int R, G, B, cdata;
    /*
    CIwImage *iR;
    //
    iR = new CIwImage();
    iR->SetFormat(CIwImage::RGB_888);
    iR->SetWidth(destImage->GetWidth());
    iR->SetHeight(destImage->GetHeight());
    iR->ConvertToImage(iR);
    */
    //
    sw = dsInfo.m_Width;
    sh = dsInfo.m_Height;
    sp = dsInfo.m_Pitch;
    data = (int*)dsInfo.m_Data;
    if (data == NULL)
        return;
    //
    idx = 0;
    for (j = 0; j < sh; j++)
    {
        for (i = 0; i < sw; i++)
        {
            R = 0;
            G = 0;
            B = 0;
            // get pixel at (x, y-1)
            idxl = idx - sw - sp;
            if (idxl >= 0)
            {
                cdata = data[idxl];
                R += ((cdata & 0x00ff0000) >> 16);
                G += ((cdata & 0x0000ff00) >> 8);
                B += ((cdata & 0x000000ff));
            }
            // get pixel at (x-1, y)
            idxl = idx - 1;
            if (idxl >= 0)
            {
                cdata = data[idxl];
                R += ((cdata & 0x00ff0000) >> 16);
                G += ((cdata & 0x0000ff00) >> 8);
                B += ((cdata & 0x000000ff));
            }
            // get pixel at (x+1, y)
            idxl = idx + 1;
            if (idxl < ((sw + sp) * sh))
            {
                cdata = data[idxl];
                R += ((cdata & 0x00ff0000) >> 16);
                G += ((cdata & 0x0000ff00) >> 8);
                B += ((cdata & 0x000000ff));
            }
            // get pixel at (x, y+1)
            idxl = idx + sw + sp;
            if (idxl < ((sw + sp) * sh))
            {
                cdata = data[idxl];
                R += ((cdata & 0x00ff0000) >> 16);
                G += ((cdata & 0x0000ff00) >> 8);
                B += ((cdata & 0x000000ff));
            }
            //
            R >>= 2;
            G >>= 2;
            B >>= 2;
            data[idx] = ((R & 0x0ff) << 16) + ((G & 0x0ff) << 8) + ((B & 0x0ff));
            //
            idx++;
        }
        idx += sp;
    }
}
