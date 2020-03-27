#include <stdlib.h>
#include <stdbool.h>

volatile int pixel_buffer_start; // global variable

//VGA boundaries ===============================================================================
const int VGA_X_MIN = 0;
const int VGA_X_MAX = 319;
const int VGA_Y_MIN = 0;
const int VGA_Y_MAX = 239;

//Background boundaries  ===============================================================================
const int GROUND_Y_START = 200;

//Player attributes  ===============================================================================
const int PLAYER_SIZE = 15;
const int PLAYER_START_X = 125;
const int PLAYER_START_Y = 185;

//colors ========= Very temporary for now  ===============================================================================
const short int GROUND_COLOR = 0x07E0;
const short int BACKGROUND_COLOR = 0x001F;
const short int PLAYER_BODY_COLOR = 0xF800;

//Function prototypes:  ===============================================================================
void clear_screen();
void wait_for_vsync();
void plot_pixel(int x, int y, short int line_color);
void draw_player(int x, int y);
bool on_ground(int y);

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    
    /* set back pixel buffer to start of SDRAM memory */

    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    

    //Stores player position
    int player_x = PLAYER_START_X;
    int player_y = PLAYER_START_Y;

    //Stores player directions
    int move_x = 0;
    int move_y = 0;


    while(1)
    {
        clear_screen();

        draw_player(player_x, player_y);



        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }

}

//Draws player
void draw_player(int x, int y)
{
    //draws a square for now needs to be updated to draw an actual player
    for (int i = x; i < x + PLAYER_SIZE; i++)
    {
        for(int j = y; j < y + PLAYER_SIZE; j++)
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
bool on_ground(int y)
{
    if(y + PLAYER_SIZE >= GROUND_Y_START)
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
