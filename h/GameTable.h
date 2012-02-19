#ifndef GAMETABLE_H
#define GAMETABLE_H


#include "s3e.h"
#include "IwGeom.h"


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
#define FRAMES_DESTROY 4
#define FRAMES_FALL    26
#define FRAMES_GLOW    10

#define MAX_WORM       10
#define MAX_BONUS      32

#define BONUS_NONE     (-1)
#define BONUS_CLOCK    0
#define BONUS_BOMB     1
#define BONUS_CHARGE1  2
#define BONUS_CHARGE2  3
#define BONUS_CHARGE3  4
#define BONUS_TYPES    5

#define BIT_FRAMES     10
#define MAX_CAPACITY   256


void myIwGxInitBonus();
void myIwGxPrepareBonus();
void myIwGxDrawBonus(int x, int y, CIwSVec2 texpos, iwangle rotval, int alphaf);
void myIwGxDoneBonus();


int get_grid_anim_frame(int i);


typedef struct send_rval_s 
{
    int bits_sent;
    int connections_left;
    int connections_right;
    int bonus_collected;
    int bomb;
    int freeze;
} SendRval;


class GameTable
{
public:
    class LeftConnector
    {
    public:
        friend class GameTable;
        int enabled;

        LeftConnector()
        {
            enabled = 1;
            frames_incoming = -1;
            frames_sending = -1;
            capacity = 8;
            speed = 1;
            accumulator = 0;
            display_bit[1] = 0;
        }
        ~LeftConnector() {}

        void Reset_acc()
        {
            accumulator = 0;
            frames_incoming = -1;
        }
        void Set_SC(int spd, int cap)
        {
            accumulator = 0;
            frames_incoming = -1;
            frames_sending = -1;
            speed = spd;
            if (speed > (4000 / BIT_FRAMES))
                speed = (4000 / BIT_FRAMES);
            capacity = cap;
            if (capacity > MAX_CAPACITY)
                capacity = MAX_CAPACITY;
        }
        void IncrementSpeed()
        {
            speed++;
            if (speed > (4000 / BIT_FRAMES))
                speed = (4000 / BIT_FRAMES);
        }
        void IncrementCapacity()
        {
            capacity++;
            if (capacity > MAX_CAPACITY)
                capacity = MAX_CAPACITY;
        }
        void SetupSending()
        {
            frames_sending = accumulator;
            Reset_acc();
        }
        void UpdateConnectorL()
        {
            int recv_interval;
            //
            if (!enabled)
                return;
            //
            if (frames_incoming >= 0)
                frames_incoming--;
            if (frames_sending >= 0)
                frames_sending--;
            //
            if (accumulator >= capacity)
            {
                accumulator = capacity;
                return;
            }
            if ((frames_sending < 0) && (frames_incoming < 0) && (accumulator < capacity))
            {
                recv_interval = 4000 / (speed * BIT_FRAMES);
                if ((rand() % recv_interval) == 0)
                {
                    frames_incoming = BIT_FRAMES;
                    accumulator++;
                    display_bit[0] = (char)(rand() % 2) + (char)48;
                }
            }
        }
        void RenderConnectorL(int x, int y);

    private:
        char display_bit[2];
        int frames_incoming;
        int frames_sending;
        int accumulator;
        int speed;
        int capacity;
    };


    class RightConnector
    {
    public:
        friend class GameTable;
        int enabled;

        RightConnector()
        {
            enabled = 1;
            frames_sending = -1;
            still_to_send = 0;
            capacity = 4;
            display_bit[1] = 0;
        }
        ~RightConnector() {}

        void Reset()
        {
            frames_sending = -1;
            still_to_send = -1;
        }

        int GetCapacity()
        {
            return capacity;
        }
        void IncrementCapacity()
        {
            capacity++;
            if (capacity > MAX_CAPACITY)
                capacity = MAX_CAPACITY;
        }
        int SetupSending(int to_send)
        {
            frames_sending = BIT_FRAMES;
            still_to_send = to_send;
            if (still_to_send > capacity)
                still_to_send = capacity;
            display_bit[0] = (char)(rand() % 2) + (char)48;
            //
            // return unsent bits
            return (to_send - still_to_send);
        }
        void UpdateConnectorR()
        {
            if (!enabled)
                return;
            //
            if (frames_sending >= 0)
                frames_sending--;
            if ((frames_sending < 0) && (still_to_send > 0))
            {
                still_to_send--;
                frames_sending = BIT_FRAMES;
                display_bit[0] = (char)(rand() % 2) + (char)48;
            }
        }
        void RenderConnectorR(int x, int y);

    private:
        char display_bit[2];
        int frames_sending;
        int still_to_send;
        int capacity;
    };


    class BonusItem
    {
    public:
        friend class GameTable;
        int enabled;

        BonusItem(void) { enabled = 0; rot = 0; }
        ~BonusItem(void) {}

        void SetBonusItem(int ti, int tj, int btype, int timeout_ms);
        void UpdateBonusItem();
        void DrawBonusItem(int x, int y);

        int getOffsetX()
        {
            int ox, tx;
            //
            ox = oi * 64 + 32;
            tx = target_i * 64 + 32;
            //
            if ((falling_frame >= 0) && (falling_frame < (FRAMES_FALL * 2)))
                return ox + ((tx - ox) * falling_frame) / (FRAMES_FALL * 2);
            return tx;
        }

        int getOffsetY()
        {
            int oy, ty;
            //
            oy = oj * 64 + 32;
            ty = target_j * 64 + 32;
            //
            if ((falling_frame >= 0) && (falling_frame < (FRAMES_FALL * 2)))
                return oy + ((ty - oy) * falling_frame) / (FRAMES_FALL * 2);
            return ty;
        }

    private:
        int falling_frame;
        int glowing_frame;
        int oi, oj;
        int target_i, target_j;
        int bonus_type;
        iwangle rot;
        int time_appeared;
        int timeout;
    };


    class Worm
    {
    public:
        Worm(void)
        {
            int i;
            SetWorm(0, 0);
            for (i = 0; i < MAX_WORM; i++)
            {
                worm_x[i] = i;
                worm_y[i] = 5;
            }
        }
        ~Worm(void) {}

        void SetWorm(int wlen, int tfreq)
        {
            int i;
            if ((wlen >= 0) && (wlen <= MAX_WORM))
                worm_length = wlen;
            tile_interaction = tfreq;
            moving = 0;
        }

        int can_click(int gx, int gy)
        {
            int i;
            if (worm_length <= 0)
                return 1;
            for (i = 0; i < worm_length; i++)
                if (((gx == worm_x[i]) && (gy == worm_y[i])) ||
                    ((gx == worm_ox[i]) && (gy == worm_oy[i]) && moving))
                    return 0;
            return 1;
        }

        int touches_xy(int x, int y)
        {
            int i;
            for (i = 0; i < worm_length; i++)
                if ((worm_x[i] == x) && (worm_y[i] == y))
                    return 1;
            return 0;
        }

        inline int get_current_length()
        {
            return worm_length;
        }

        void update_worm();
        void draw_worm(int x, int y);

    private:
        int worm_length;
        int moving;
        int tile_interaction;
        //
        int worm_x[MAX_WORM];
        int worm_y[MAX_WORM];
        int worm_ox[MAX_WORM];
        int worm_oy[MAX_WORM];
    };


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
    SendRval send_connections();
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

    void SetWormParams(int wlen, int tfreq, int timeout)
    {
        worm_wlen = wlen;
        worm_tfreq = tfreq;
        worm_timeout = timeout;
        the_worm.SetWorm(worm_wlen, worm_tfreq);
    }

    void update_the_worm()
    {
        the_worm.update_worm();
        if (the_worm.get_current_length() <= 0)
        {
            if (((int)s3eTimerGetMs() - worm_wait_time) > worm_timeout)
            {
                the_worm.SetWorm(worm_wlen, worm_tfreq);
            }
        }
    }

    void AddBonusItem(int btype, int timeout)
    {
        // add a certain bonus, with a certain timeout value, at a random position
        //
        int internal_counter = 0;
        int ti, tj;
        int already_taken = 1;
        //
        while (bonuses[bonus_counter].enabled == 1)
        {
            bonus_counter = (bonus_counter + 1) % MAX_BONUS;
            internal_counter++;
            // if all bonuses are already enabled, do nothing
            if (internal_counter > MAX_BONUS)
                return;
        }
        //
        while (already_taken)
        {
            already_taken = 0;
            ti = rand() % GRID_W;
            tj = rand() % GRID_H;
            for (internal_counter = 0; internal_counter < MAX_BONUS; internal_counter++)
                if (bonuses[internal_counter].enabled &&
                    (bonuses[internal_counter].target_i == ti) &&
                    (bonuses[internal_counter].target_j == tj))
                    already_taken = 1;
        }
        //
        bonuses[bonus_counter].SetBonusItem(ti, tj, btype, timeout);
    }

    void UpdateBonusItems()
    {
        int i;
        for (i = 0; i < MAX_BONUS; i++)
            bonuses[i].UpdateBonusItem();
    }

    BonusItem *GetBonusItem(int i)
    {
        int k = i;
        if (k < 0)
            k = 0;
        if (k >= MAX_BONUS)
            k = MAX_BONUS - 1;
        return bonuses + k;
    }

    BonusItem *GetBonusItemAt(int ti, int tj)
    {
        BonusItem *rVal = NULL;
        int i;
        for (i = 0; i < MAX_BONUS; i++)
            if (bonuses[i].enabled && (bonuses[i].target_i == ti) && (bonuses[i].target_j == tj))
            {
                rVal = bonuses + i;
                break;
            }
        return rVal;
    }

    Worm the_worm;
    LeftConnector LC[GRID_H];
    RightConnector RC[GRID_H];

private:
    unsigned char grid_connectors[GRID_W][GRID_H];
    unsigned char grid_clickable[GRID_W][GRID_H];
    unsigned char grid_state[GRID_W][GRID_H];
    unsigned char grid_color_shift[GRID_W][GRID_H];

    BonusItem bonuses[MAX_BONUS];
    int bonus_counter;

    int worm_wait_time;
    int worm_timeout;
    int worm_wlen;
    int worm_tfreq;

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
