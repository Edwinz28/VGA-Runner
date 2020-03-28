#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

volatile int pixel_buffer_start; // global variable

//VGA boundaries ===============================================================================
const int VGA_X_MIN = 0;
const int VGA_X_MAX = 319;
const int VGA_Y_MIN = 0;
const int VGA_Y_MAX = 239;

//Background boundaries  ===============================================================================
const int GROUND_Y_START = 200;

//colors ========= Very temporary for now  ===============================================================================
const short int GROUND_COLOR = 0x07E0;
const short int BACKGROUND_COLOR = 0x001F;
const short int PLAYER_BODY_COLOR = 0xF800;
const short int OBSTACLE_COLOR = 0xF81F;


//Player struct
struct Player{
    int x;
    int y;
    int size;
    int y_dir;
    bool is_grounded;
};

//obstacle struct
struct Obstacle{
    int x;
    int x_speed;
    int size;
};


//Function prototypes:  ===============================================================================
void clear_screen();
void wait_for_vsync();
void plot_pixel(int x, int y, short int line_color);
void draw_player(int x, int y, int size);
bool on_ground(int y, int size);
void jump(int *y);
bool spawn_obstacle();
void draw_obstacle(struct Obstacle *obs); 



int main(void)
{

    //Initialize FPGA  ===============================================================================
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    volatile int * key_ptr = (int *)0xFF200050;                                                 //might need to use interrupts instead of key registers
   
    *(pixel_ctrl_ptr + 1) = 0xC8000000;                                      
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); 
    
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    

    //Player attributes  ===============================================================================
    const int PLAYER_SIZE = 15;
    const int PLAYER_START_X = 125;
    const int PLAYER_START_Y = 185;


    //initialize player ===============================================================================
    struct Player player;
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    player.y_dir = 0;
    player.size = PLAYER_SIZE;
    player.is_grounded = true;

    //Initialize obstacles  ===============================================================================
    //struct Obstacle obstacle[10];

    struct Obstacle test;
    test.size = 10;
    test.x = VGA_X_MAX;
    test.x_speed = -10;

    //Main game loop
    while(1)
    {
        clear_screen();

        draw_player(player.x, player.y, player.size);
        player.is_grounded = on_ground(player.y, player.size);

        if((*key_ptr & 0x01) && player.is_grounded)
        {
            printf("Jumping! \n");
            jump(&player.y);

            //updates player status
            player.y_dir = 10;
            player.is_grounded = false;
        }

        //reset gravity
        if(player.is_grounded && player.y_dir != 0)
        {
            player.y_dir = 0;
        }
        
        player.y += player.y_dir;

            draw_obstacle(&test);

        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
    }

}


bool spawn_obstacle()
{
    int num = rand () % 10;

    if(num == 3)
    {
        printf("Spawning obstacle! \n");
        return true;
    }

    return false;
}

void draw_obstacle(struct Obstacle *obs) 
{
    for(int x = obs->x; x < obs->x + obs->size; x++)
    {
        plot_pixel(obs->x, 100, OBSTACLE_COLOR);
                plot_pixel(obs->x, 101, OBSTACLE_COLOR);
                        plot_pixel(obs->x, 102, OBSTACLE_COLOR);
    }

        obs->x += obs->x_speed;
}

//Allows the player to jump
void jump(int * y)
{
    *y -= 50;
}
//Draws player
void draw_player(int x, int y, int size)
{
    //draws a square for now needs to be updated to draw an actual player
    for (int i = x; i < x + size; i++)
    {
        for(int j = y; j < y + size; j++)
        {
            plot_pixel(i, j, PLAYER_BODY_COLOR);
        }
    }
}



//clears screen to background
void clear_screen()
{
    //Sky & background
    for(int x = VGA_X_MIN; x <= VGA_X_MAX; x++){
        for(int y = VGA_Y_MIN; y <= VGA_Y_MAX; y++){
            plot_pixel(x,y,BACKGROUND_COLOR);
        }
    }

    //Ground
    for(int x = 0; x < VGA_X_MAX; x++)
    {
        for(int y = GROUND_Y_START; y < VGA_Y_MAX; y++)
        {
            plot_pixel(x,y, GROUND_COLOR);
        }
    }
}

//==================================================================== Finalized code ========================================
//Checks if player is on ground
bool on_ground(int y, int size)
{
    if(y + size >= GROUND_Y_START)
    {
        return true;
    }

    return false;
}



//plots pixels on VGA
void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void wait_for_vsync()
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

    volatile int * status =(int *)0xFF20302C;

    *pixel_ctrl_ptr = 1;


    while((*status &0x1) != 0)
    {
            //do nothing
    }

    return;
}
