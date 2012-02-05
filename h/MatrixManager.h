#ifndef MATRIXMANAGER_H
#define MATRIXMANAGER_H


#include <s3eSurface.h>
#include "Iw2D.h"
#include "s3e.h"
#include "IwGeom.h"


#define SURFACE_DIM      1024
#define CHAR_W           16
#define CHAR_H           20
#define MATRIX_W         (SURFACE_DIM / CHAR_W)
#define MATRIX_H         (SURFACE_DIM / CHAR_H)

#define MAX_FRAME_DELAY  60
#define MATRIX_COLOR     0xff00ff00
#define MAX_INTENSITY    0xff

#define MAX_STARS        0x00800
#define MX_VERTICES      (2048)

#define FIELD_RES        65536
#define FIELD_SIZE       1024
#define FIELD_DIV        (FIELD_RES / FIELD_SIZE)


void myIwGxInitStars();
void myIwGxPrepareStars();
void myIwGxDrawStar(int x, int y, CIwSVec2 texpos, int iscale, iwangle rotval, int alphaf);
void myIwGxDoneStars();


class MatrixManager
{
public:
    class Star
    {
    public:
        friend class MatrixManager;
        int enabled;

        Star() { enabled = 0; }
        ~Star() {}

        void SetStar();
        void UpdateStar();
        void DrawStar(int alphaf);
        void MoveStar(int dx);

    private:
        int fx, fy;
        int multiplier;
        int star_hue;
        int size_frame;
        iwangle rot_inc;
        iwangle rot;
        int time_appeared;
        int timeout;
    };

    MatrixManager(void);
    ~MatrixManager(void);

    void SetMatrixColor(unsigned int matrixc);
    //
    void MarkingBigText(int x, int y, char *strw);
    void MarkingSmallText(int x, int y, char *strw);
    void ClearMarkings();
    //
    void AddStar();
    void MoveStars(int dx);
    void UpdateMatrix();
    void DrawMatrix();

private:
    //
    Star starfield[MAX_STARS];
    int star_counter;
    int internal_mover;
    //
    unsigned int matrix_color;
    //
    char matrix_chars[MATRIX_W][MATRIX_H];
    int matrix_intensity[MATRIX_W][MATRIX_H];
    //
    short anim_start[MATRIX_W];
    short anim_point[MATRIX_W];
    //
    char matrix_keep_intensity[MATRIX_W][MATRIX_H];
    char matrix_keep_text[MATRIX_W][MATRIX_H];
};

#endif
