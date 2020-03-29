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
    int y;
    int x_speed;
    int width;
    int height;
};

//Linked list of obstacles
struct node
{
    struct Obstacle data;
    struct node *next;
};


//Function prototypes:  ===============================================================================
void clear_screen();
void wait_for_vsync();
void plot_pixel(int x, int y, short int line_color);
void draw_player(int x, int y, int size);
bool on_ground(int y, int size);
void jump(int *y);
void spawn_obstacle(struct node* head);
void draw_obstacle(struct node* head); 
bool collision(struct Player player);



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
    //Make linked list of obstacles
    //new obstacles always are at the end of the screen (end of list)
    //when obstacles get off screen: -> delete head of list
    struct node *head = (struct node*)malloc(sizeof(struct node));
    head->next = NULL;
    //struct node *tail = (struct node*)malloc(sizeof(struct node));

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

        //collision(player);

        spawn_obstacle(head);
        draw_obstacle(head);

        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
    }

}

//forwards player collision detection using color detection
bool collision(struct Player player)
{
    //checks up to 5 pixels ahead incase of pixel skipping (odd /even incrementing)
    for(int i = 0; i < 2; i ++)
    {   
        //checks if color matches
        if( *(short int *)(pixel_buffer_start + (player.y << 10) + ((player.x + i) << 1)) == OBSTACLE_COLOR)
        {
            printf("Oh no we hit something \n");
            return true;
        }
    }

    return false;
}

void spawn_obstacle(struct node* head)
{
    int num = rand () % 10;

    if(num == 3)
    {
        printf("Spawning obstacle! \n");
        struct node* curr = head;
        while(curr->next != NULL)
        {
            curr = curr->next;
        }
        struct node* newNode = (struct node*)malloc(sizeof(struct node));

        //random for now just to test spawning
        struct Obstacle data;
        data.x = VGA_X_MAX;
        data.y = 100;
        data.x_speed = -10;
        data.height = 25;
        data.width = 25;

        newNode->data = data;
        newNode->next = NULL;

        curr->next = newNode;
    }
}

void draw_obstacle(struct node* head) 
{
    struct node* curr = head;
    struct node* prev = curr;
    while(curr->next != NULL)
    {
        //off screen, delete from list
        if(curr->data.x <= 0 && curr != head)
        {
            printf("Deleting obstacle\n");
            struct node* temp = curr;
            prev->next = curr->next;
            free (temp);
            curr = prev->next;
        }
        else
        {
            for(int x = curr->data.x; x <  curr->data.x +  curr->data.width; x++)
            {
                for(int y =  curr->data.y; y <  curr->data.y +  curr->data.height; y++)
                {
                    if(x <= VGA_X_MAX && x >= VGA_X_MIN && y >= VGA_Y_MIN  && y <= VGA_Y_MAX)
                    {
                        plot_pixel(x,y,OBSTACLE_COLOR);
                    }
                }
            }
            curr->data.x +=  curr->data.x_speed;
            prev = curr;
            curr = curr->next;
        }
    }
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
