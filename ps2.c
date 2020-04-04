#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


//create the functionality of ps2 input

int main()
{
    //saves ps2 bytes
    unsigned char data_in = 0;
    unsigned char break_code = 0;

    volatile int* PS2_ptr = (int *) 0xFF200100;

    int PS2_data;

    while(1)
    {

        PS2_data = *PS2_ptr;
        
        data_in = PS2_data & 0xFF;

        //Use WASD for controls


        if(data_in == 0x1C)
        {
            printf ("left \n");
        }
        else if(data_in == 0x23)
        {
            printf ("right \n");
        }
        else if(data_in == 0x1B)
        {
            printf ("down \n");
        }
        else if(data_in == 0x1D)
        {
            printf ("up \n");
        }
        else if(data_in == 0xF0)
        {
            //break code do nothing
        }
    }
}