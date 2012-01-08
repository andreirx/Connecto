#include <time.h>
#include <stdlib.h>
#include "GameTable.h"
#include "s3e.h"
#include "Iw2D.h"
#include "IwGx.h"

int grid_anim_frame[10] =
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


GameTable::GameTable(void)
{
    new_elements = 0;
    missing_link_elements = 0;
    can_send_connections = 0;
    fall_finished = 0;
    trigger_anim = -1;
    trigger_circle = -1;
}

GameTable::~GameTable(void)
{
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
}

// public function for returning new elements on a column
void GameTable::new_column(int x)
{
    int k, i, j;
    for (k = 0; k < GRID_H; k++)
    {
        grid_connectors[x][k] = get_new_element();
        grid_state[x][k] = CONNECT_NONE;
        grid_anim_frame[x][k] = FRAMES_DESTROY;
        grid_anim_type[x][k] = ANIM_DESTROY;
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
    if (!can_send_connections)
        return rVal;
    for (i = 0; i < GRID_W; i++)
    {
        for (j = 0; j < GRID_H; j++)
        {
            if (grid_state[i][j] == CONNECT_OK)
            {
                rVal += 0x000001;
                if (i == (GRID_W - 1))
                    if (grid_connectors[i][j] & CB_RIGHT != 0)
                        rVal += 0x010000;
                // move one position down
                for (k = j; k > 0; k--)
                {
                    grid_connectors[i][k] = grid_connectors[i][k - 1];
                    grid_state[i][k] = grid_state[i][k - 1];
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
    for (i = (x - 2); i <= (x + 2); i++)
        for (j = (y - 2); j <= (y + 2); j++)
        {
            // check boundaries
            if ((i >= 0) && (i < GRID_W) && (j >= 0) && (j < GRID_H))
            {
                // move one position down
                for (k = j; k > 0; k--)
                {
                    grid_connectors[i][k] = grid_connectors[i][k - 1];
                    grid_state[i][k] = grid_state[i][k - 1];
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
        expand_connections(GRID_W - 1, j, CB_RIGHT, CONNECT_RIGHT);
    // check connections to the left
    for (j = 0; j < GRID_H; j++)
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
