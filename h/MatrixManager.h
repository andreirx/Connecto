#ifndef MATRIXMANAGER_H
#define MATRIXMANAGER_H


#include <s3eSurface.h>
#include "Iw2D.h"


#define SURFACE_DIM      1024
#define CHAR_W           16
#define CHAR_H           16
#define MATRIX_W         (SURFACE_DIM / CHAR_W)
#define MATRIX_H         (SURFACE_DIM / CHAR_H)

#define MAX_FRAME_DELAY  60
#define MATRIX_COLOR     0xff00ff00
#define MAX_INTENSITY    0xff


class MatrixManager
{
public:
    MatrixManager(void);
    ~MatrixManager(void);

    void SetMatrixColor(unsigned int matrixc);
    //
    void MarkingBigText(int x, int y, char *strw);
    void MarkingSmallText(int x, int y, char *strw);
    void ClearMarkings();
    //
    void UpdateMatrix();
    void DrawMatrix();

private:
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
