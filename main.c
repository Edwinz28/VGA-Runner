#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

volatile int pixel_buffer_start; // global variable
volatile char *character_buffer = (char *) 0xC9000000;// VGA character buffer
volatile int *LEDR_ptr = (int *) 0xFF200000;

//VGA boundaries
const int VGA_X_MIN = 1;
const int VGA_X_MAX = 318;
const int VGA_Y_MIN = 1;
const int VGA_Y_MAX = 238;
const int GROUND_Y_START = 200;

//Color Constants
const short int GROUND_COLOR = 0x07E0;
const short int BACKGROUND_COLOR = 0x001F;
const short int PLAYER_BODY_COLOR = 0xF800;
const short int OBSTACLE_COLOR = 0xF81F;

//Player struct definiton
struct Player{
    int x;
    int y;
    int size;
    int y_dir;
    int x_dir;
};

//obstacle struct definition
struct Obstacle{
    int x;
    int y;
    int x_speed;
    int width;
    int height;
};

//Linked list of obstacles
struct node{
    struct Obstacle data;
    struct node *next;
};


//Function prototypes:
void clear_screen();
void wait_for_vsync();
void plot_pixel(int x, int y, short int line_color);
void draw_player(int x, int y, int size, short int color, int offset);
struct node* spawn_obstacle(struct node* head);
struct node* draw_obstacle(struct node* head); 
struct Obstacle create_obstacle();
bool collision(struct Player player);
void printTextOnScreen(int x, int y, char *scorePtr);
void draw_obstacle_helper(struct node* curr, short int color, int offset);

int main(void){
    //Initialize the score to 0
    int totalScore = 0;
    int scoreOnes = 0;
    int scoreTens = 0;
    int scoreHundreds = 0;
    *LEDR_ptr = 0;
    int timeCount = 0;

    //Initialize FPGA 
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020; //Vga buffer
    volatile int* PS2_ptr = (int *) 0xFF200100; // ps2 keyboard
   
    *(pixel_ctrl_ptr + 1) = 0xC8000000;                                      
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); 
    
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    
    //Player attributes
    const int PLAYER_SIZE = 15;
    const int PLAYER_START_X = 75;
    const int PLAYER_START_Y = 185;

    //initialize player
    struct Player player;
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    player.y_dir = 0;
    player.x_dir = 0;
    player.size = PLAYER_SIZE;


    //Initialize obstacles 
    //Make linked list of obstacles
    //new obstacles always are at the end of the screen (end of list)
    //when obstacles get off screen: -> delete head of list
    struct node *head = NULL;

    //Initialize Ps2 Keyboard data
    int PS2_data;
    unsigned char data_in = 0;

    bool gameOver = false;
    clear_screen();

    //Main game loop
    while(!gameOver){
        timeCount++;

        // Plot score
        if (timeCount == 2) {
            timeCount = 0;
            totalScore++;
            scoreHundreds = totalScore / 100;
            scoreTens = (totalScore - scoreHundreds * 100) / 10;
            scoreOnes = totalScore - scoreHundreds * 100 - scoreTens * 10;
            *LEDR_ptr = totalScore;
        }

        char myScoreString[15];
        myScoreString[0] = 'M';
        myScoreString[1] = 'Y';
        myScoreString[2] = ' ';
        myScoreString[3] = 'S';
        myScoreString[4] = 'C';
        myScoreString[5] = 'O';
        myScoreString[6] = 'R';
        myScoreString[7] = 'E';
        myScoreString[8] = ':';
        myScoreString[9] = ' ';

        if (scoreHundreds != 0) {
            myScoreString[10] = scoreHundreds + '0';
        } 
        
        else {
            myScoreString[10] = ' ';
        }
        
        if (scoreHundreds == 0 && scoreTens == 0) {
            myScoreString[11] = ' ';
        } 
        
        else {
            myScoreString[11] = scoreTens + '0';
        }
        
        myScoreString[12] = scoreOnes + '0';
        myScoreString[13] = '\0';
        
        printTextOnScreen(285, 0, myScoreString);

        PS2_data = *PS2_ptr;
        data_in = PS2_data & 0xFF;

        //Gets direction code
        if(data_in == 0x1B && player.y + player.size < VGA_Y_MAX)
        {
            player.x_dir = 0;
            player.y_dir = 2;
        }
        else if(data_in == 0x1D && player.y > VGA_Y_MIN)
        {
            player.x_dir = 0;
            player.y_dir = -2;
        }
        else if(data_in == 0x1C && player.x > VGA_X_MIN)
        {
            player.y_dir = 0;
            player.x_dir = -2;
        }
        else if(data_in == 0x23 && player.x + player.size < VGA_Y_MAX)
        {
            player.y_dir = 0;
            player.x_dir = 2;
        }
        
        // Erases player
        draw_player(player.x, player.y, player.size , BACKGROUND_COLOR, 3);
        
        //updates new player direction
        player.y += player.y_dir;
        player.x += player.x_dir;
        // Draw player
        draw_player(player.x, player.y, player.size , PLAYER_BODY_COLOR, 0);
 
        head = spawn_obstacle(head);
        //clear_bg(head);
        head = draw_obstacle(head);

        gameOver = collision(player);

        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
    }

}

//forwards player collision detection using color detection
bool collision(struct Player player)
{
    for(int x = player.x; x < player.x + player.size; x++)
    {
        for(int y = player.y; y < player.y + player.size; y++)
        {
            if( *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) == OBSTACLE_COLOR)
            {
            printf("Oh no we hit something \n");
            return true;
            }
        }
    }

    return false;
}


/* ///////////////////////////////////////////////////// Obstacle spawning ////////////////////////////////////////////// */
struct Obstacle create_obstacle()
{
    struct Obstacle data;
    data.x = VGA_X_MAX;
    data.y = rand () % VGA_Y_MAX;
    data.x_speed = -2;
    data.height = 25;
    data.width = 25;

    return data;
}
//randomly spawns obstacles and inserts into a linked list
struct node* spawn_obstacle(struct node* head){
    //Liklihood to spawn obstacle
    int num = rand() % 50;

    if(num == 3)
    {
        if(head == NULL)
        {
            printf("Spawning obstacle! New head \n");  
            struct node* newNode = (struct node*)malloc(sizeof(struct node));

            struct Obstacle data = create_obstacle();
            newNode->data = data;
            newNode->next = NULL;

            //Assigns to head
            head = newNode;
        }
        else
        {
            printf("Spawning obstacle! \n");
            struct node* curr = head;
        
            while(curr->next != NULL)
            {
                curr = curr->next;
            }
            struct node* newNode = (struct node*)malloc(sizeof(struct node));

            struct Obstacle data = create_obstacle();
            newNode->data = data;
            newNode->next = NULL;

            curr->next = newNode;
        }
    }

    return head;
}

//Draws obstacles and deletes from linked list if off screen
struct node* draw_obstacle(struct node* head) {
    
    //Empty list
    if(head == NULL)
    {
        printf("No obstacles to draw\n");
        return NULL;
    }
    
    struct node* curr = head;
    
    //Draws obstacles
    while(curr->next != NULL){
        //off screen, delete from list
        if(curr->data.x + curr->data.width <= 0){
            printf("Deleting obstacle\n");
            struct node* temp = curr;
            curr = curr->next;
            free(temp);
            head = curr;
        }
        else{   
            //clears old obstacles
            draw_obstacle_helper(curr, BACKGROUND_COLOR, 2);
            
            curr->data.x +=  curr->data.x_speed;
            //Draws new obstacle positions 
            draw_obstacle_helper(curr, OBSTACLE_COLOR, 0);
            curr = curr->next;
        }
    }

    return head;
}

/*//////////////////////////////////////////////////////////////    VGA DRAWING   /////////////////////////////////////////////////////////*/

//Iterates through the obstacle sizes and draws them, uses an offset to full clear old drawings
void draw_obstacle_helper(struct node* curr, short int color, int offset)
{
    for(int i = curr->data.x - offset; i < curr->data.x + curr->data.width + offset; i++)
    {
        for(int j = curr->data.y - offset; j < curr->data.y +curr->data.height + offset; j++)
        {
            if(i <= VGA_X_MAX && i >= VGA_X_MIN && j >= VGA_Y_MIN  && j <= VGA_Y_MAX)
            {
                plot_pixel(i, j, color);
            }
        }
    }
}

//Print text on the Screen
void printTextOnScreen(int x, int y, char *scorePtr){
    /* assume that the text string fits on one line */
    int offset = (y << 7) + x;

    while (*(scorePtr)){ // while it hasn't reach the null-terminating char in the string
        // write to the character buffer
        *(character_buffer + offset) = *(scorePtr);
        ++scorePtr;
        ++offset;
    }
}

//Draws player
void draw_player(int x, int y, int size, short int color, int offset){
    //draws a square for now needs to be updated to draw an actual player
    for (int i = x - offset; i < x + size + offset; i++)
    {
        for(int j = y - offset; j < y + size + offset; j++)
        {
            if(i <= VGA_X_MAX && i >= VGA_X_MIN && j >= VGA_Y_MIN  && j <= VGA_Y_MAX)
            {
                plot_pixel(i, j, color);
            }
        }
    }
}

//clears screen to background
void clear_screen(){
    //Sky & background
    for(int x = VGA_X_MIN; x <= VGA_X_MAX; x++){
        for(int y = VGA_Y_MIN; y <= VGA_Y_MAX; y++){
            plot_pixel(x,y,BACKGROUND_COLOR);
        }
    }
}

//plots pixels on VGA
void plot_pixel(int x, int y, short int line_color){
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

//Waits for buffers to sync before switching
void wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

    volatile int * status =(int *)0xFF20302C;

    *pixel_ctrl_ptr = 1;

    while((*status &0x1) != 0){
            //do nothing
    }
    return;
}
