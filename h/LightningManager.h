#ifndef LIGHTNINGMANAGER_H
#define LIGHTNINGMANAGER_H


#include <s3eSurface.h>
#include "Iw2D.h"


#define START_X          0
#define START_Y          0
#define END_X            65535
#define END_Y            0

// must be powers of 2
#define LEN_STRIKE       256
#define LEN_BRANCH_0     128
#define LEN_BRANCH_1     64
#define LEN_BRANCH_2     32

#define BRANCHES_0       4
#define BRANCHES_1       16
#define BRANCHES_2       64

#define NO_BRANCH        -1
#define BRANCH_STEP      10000
#define BRANCH_0_OFFSET  10000
#define BRANCH_1_OFFSET  20000
#define BRANCH_2_OFFSET  30000

#define UPDATE_ALL       0
#define UPDATE_STRIKE    256

// CAREFUL! this is how much memory one object will use
#define OBJ_MEM_USAGE    4*3*LEN_STRIKE + BRANCHES_0*4*3*LEN_BRANCH_0 + BRANCHES_1*4*3*LEN_BRANCH_1 + BRANCHES_2*4*2*LEN_BRANCH_2

#define DEFAULT_LEN      32
#define DEFAULT_COLOR    0xffffffff
#define MAX_BRANCHES     1024


class LightningBranch
{
public:
    LightningBranch(void);
    ~LightningBranch(void);

    void SetLength(int branch_length);
    int GetLength();
    void GenerateBranch(int sx, int sy, int ex, int ey, unsigned int bcolor, int update_much = UPDATE_ALL);
    void UpdateBranch(int update_much);
    void DrawBranch_as_Lines();

private:
    int osx, osy, oex, oey;
    unsigned int lightning_color;
    int len_strike;
    int *strike_x;
    int *strike_y;
    int *refstrike_x;
    int *refstrike_y;

    void make_branch(int start_index, int end_index, int *positions_x, int *positions_y);
};

class LightningManager
{
public:
    LightningManager(int sW, int sH);
    ~LightningManager(void);

    CIw2DSurface *destSurface;
    CIw2DImage *destImage;
    s3eSurfaceInfo dsInfo;

    void ResetBranches();
    void AddBranch_Generate(int branch_len, int sx, int sy, int ex, int ey, unsigned int bcolor);
    void UpdateAllBranches(int update_much);
    void DrawLightning();

private:
    // lightning branches
    LightningBranch lightning_branches[MAX_BRANCHES];
    int total_branches;
};

#endif
