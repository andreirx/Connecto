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


class LightningManager
{
public:
    LightningManager(int sW, int sH);
    ~LightningManager(void);

    CIw2DSurface *destSurface;
    CIw2DImage *destImage;
    s3eSurfaceInfo dsInfo;

    void GenerateLightning(int sx, int sy, int ex, int ey, int update_much);
    void DrawLightning();

private:
    // main strike
    int strike_x[LEN_STRIKE];
    int strike_y[LEN_STRIKE];
    int strike_branch[LEN_STRIKE];
    int refstrike_x[LEN_STRIKE];
    int refstrike_y[LEN_STRIKE];

    // secondary branches 0
    int branch_0_x[BRANCHES_0][LEN_BRANCH_0];
    int branch_0_y[BRANCHES_0][LEN_BRANCH_0];
    int branch_0_branch[BRANCHES_0][LEN_BRANCH_0];
    int generated_branches_0;

    // secondary branches 1
    int branch_1_x[BRANCHES_1][LEN_BRANCH_1];
    int branch_1_y[BRANCHES_1][LEN_BRANCH_1];
    int branch_1_branch[BRANCHES_1][LEN_BRANCH_1];
    int generated_branches_1;

    // secondary branches 2
    int branch_2_x[BRANCHES_2][LEN_BRANCH_2];
    int branch_2_y[BRANCHES_2][LEN_BRANCH_2];
    int generated_branches_2;

    // CAREFUL! this method expects vectors positions_x and positions_y to contain start and end positions
    // also, it's recursive
    void make_branch(int start_index, int end_index, int *positions_x, int *positions_y);
    void blur_image();
};

#endif
