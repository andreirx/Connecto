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

#define SPARKLE_SPACE_W  1024
#define SPARKLE_SPACE_H  1024
#define SPARKLE_SHIFT    8
#define MAX_SPARKLES     0x02000
#define MAX_GENERATORS   0x00100
#define SPARKLES_MASK    0x00fff
#define SPARKLE_GRAVITY  (0xaa)

#define DGX_VERTICES     (2048)


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
    void DrawBranch_as_Sparkles(unsigned int bcolor, int size);

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


class Sparkle
{
public:
    int enabled;

    Sparkle(void);
    ~Sparkle(void);

    void Enable_SetXYCS(int px, int py, int vx, int vy, unsigned int scolor, int size);
    void UpdateSparkle();
    void DrawSparkle();

private:
    unsigned int sparkle_color;
    int sparkle_size;
    int ss_x, ss_y;
    int os_x, os_y;
    int vel_x, vel_y;
};


void InitSparklingLines();
void PrepareSparklingLines();
void DrawSparklingLine(int sx, int sy, int ex, int ey, unsigned int lcolor, int size);
void DoneSparklingLines();


class LightningManager
{
public:
    LightningManager(int sW, int sH);
    ~LightningManager(void);

    class SparkleGenerator
    {
    public:
        int enabled;

        SparkleGenerator() {}
        ~SparkleGenerator(void) {}

        void StartGenerator_XYT(int x, int y, int duration_ms, int intensity_is, int size, LightningManager *lm)
        {
            px = x;
            py = y;
            duration = duration_ms;
            intensity = intensity_is;
            total_generated = 0;
            max_size = size;
            lightning = lm;
            start_timer = (int)s3eTimerGetMs();
        }

        void UpdateGenerator()
        {
            int delta_t;
            if (enabled)
            {
                delta_t = (int)s3eTimerGetMs() - start_timer;
                if (delta_t >= duration)
                    intensity--;
                if (intensity <= 0)
                    enabled = 0;
                if (enabled)
                {
                    while (total_generated < (intensity * delta_t / 1000))
                    {
                        lightning->AddSparkle_SetXYCS(px, py, (rand() % 5120) - 2560, (rand() % 5120) - 3840,
                            (rand() % 12), (rand() % max_size) + 1);
                        total_generated++;
                    }
                }
            }
        }

    private:
        LightningManager *lightning;
        int px, py;
        int start_timer;
        int duration;
        int intensity;
        int total_generated;
        int max_size;
    };

    CIw2DSurface *destSurface;
    CIw2DImage *destImage;
    s3eSurfaceInfo dsInfo;

    void ResetBranches();
    int AddBranch_Generate(int branch_len, int sx, int sy, int ex, int ey, unsigned int bcolor);
    void KillBranch(int which_branch);
    void UpdateAllBranches(int update_much);
    void DrawLightning(int size);

    void AddSparkle_SetXYCS(int px, int py, int vx, int vy, unsigned int scolor, int size);
    void UpdateAllSparkles();
    void DrawSparkles();

    void AddGenerator_SetXYT(int x, int y, int duration_ms, int intensity_is, int size);
    void UpdateAllGenerators();

private:
    // lightning branches
    LightningBranch lightning_branches[MAX_BRANCHES];
    int total_branches;
    // sparkles
    Sparkle sparkles[MAX_SPARKLES];
    int sparkle_counter;
    // sparkle generators
    SparkleGenerator generators[MAX_GENERATORS];
    int generator_counter;
};

#endif
