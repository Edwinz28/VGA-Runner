#include <stdlib.h>

volatile int pixel_buffer_start; // global variable

//VGA boundaries
const int VGA_X_MIN = 0;
const int VGA_X_MAX = 319;
const int VGA_Y_MIN = 0;
const int VGA_Y_MAX = 239;


//Function prototypes:
void clear_screen();
void wait_for_vsync();
void plot_pixel(int x, int y, short int line_color);

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

    while(1)
    {
        clear_screen();

        //code



        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }

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

//clears screen to black
void clear_screen()
{
    for(int x = VGA_X_MIN; x <= VGA_X_MAX; x++){
        for(int y = VGA_Y_MIN; y <= VGA_Y_MAX; y++){
            plot_pixel(x,y,0x0000);
        }
    }
}

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}
