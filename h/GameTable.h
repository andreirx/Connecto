#ifndef GAMETABLE_H
#define GAMETABLE_H

#define GRID_W 10
#define GRID_H 10

#define CONNECT_NONE  3
#define CONNECT_OK    2
#define CONNECT_RIGHT 1
#define CONNECT_LEFT  0

// bit 0: right, bit 1: up, bit 2: left, bit 3: down
#define CB_RIGHT 1
#define CB_UP    2
#define CB_LEFT  4
#define CB_DOWN  8

#define ANIM_NONE      0
#define ANIM_FALL      101
#define ANIM_DESTROY   102
#define FRAMES_DESTROY 8
#define FRAMES_FALL    30

class GameTable
{
public:
    GameTable(void);
    ~GameTable(void);

    inline unsigned char get_grid_connector(int x, int y)
    {
        // WARNING: no limit checking
        return grid_connectors[x][y];
    }

    inline unsigned char is_grid_clickable(int x, int y)
    {
        // WARNING: no limit checking
        return grid_clickable[x][y];
    }

    inline unsigned char get_grid_state(int x, int y)
    {
        // WARNING: no limit checking
        return grid_state[x][y];
    }

    inline unsigned char get_grid_color_shift(int x, int y)
    {
        // WARNING: no limit checking
        return grid_color_shift[x][y];
    }

    // public function for initializing the table
    void reset_table(int percent_missing_links);
    // public function for updating the table connections state ("markers")
    int check_connections();
    // public function for returning new elements on a column
    void new_column(int x);
    // public function which "clicks" a grid element, rotating it counterclockwise
    void click_element(int x, int y);
    // public function which deletes the connected paths, and inserts new elements
    int send_connections();
    inline int can_send()
    {
        return can_send_connections;
    }
    inline int is_animating()
    {
        // returns ANIM_NONE, ANIM_DESTROY, or ANIM_FALL
        return still_animating;
    }
    int fall_anim_finished()
    {
        int rVal = fall_finished;
        fall_finished = 0;
        return rVal;
    }
    // public function which deletes tiles around x, y and inserts new elements
    void bomb_table(int x, int y);
    // public function which updates animations
    void update_anims();
    void update_color_shifts();

    // bit 0: right, bit 1: up, bit 2: left, bit 3: down
    //unsigned char grid_codes[16];// = {0, 5, 10, 12, 6, 3, 9, 7, 11, 13, 14, 15, 1, 8, 4, 2};
    //unsigned char grid_codep[16];// = {0, 12, 15, 5, 14, 1, 4, 7, 13, 6, 2, 8, 3, 9, 10, 11};
    unsigned char grid_anim_frame[GRID_W][GRID_H];
    unsigned char grid_anim_type[GRID_W][GRID_H];

private:
    unsigned char grid_connectors[GRID_W][GRID_H];
    unsigned char grid_clickable[GRID_W][GRID_H];
    unsigned char grid_state[GRID_W][GRID_H];
    unsigned char grid_color_shift[GRID_W][GRID_H];

    int can_send_connections;
    int missing_links;
    int still_animating;
    int fall_finished;

    int trigger_anim;
    int trigger_circle;
    int tc_x, tc_y;

    int new_elements;
    int missing_link_elements;
    unsigned char get_new_element();

    unsigned char grid_element_rotate(unsigned char grid_code);
    void expand_connections(int cx, int cy, unsigned char ctype, unsigned char marker);
};

#endif
