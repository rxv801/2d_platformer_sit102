#include "splashkit.h"
using std::to_string;

string text;
int current_level = 1;
bool level_needs_loading = true;
struct Player
{
    int x, y, vx, vy, width, height;
    bool on_ground;
};

struct Platform
{
    int x, y, width, height;
};

struct Goal
{
    int x, y, width, height;
};

enum GameState
{
    Playing,
    Win,
    GameOver
};

GameState current_state = Playing;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int NUM_PLATFORMS = 3;

Platform platforms[NUM_PLATFORMS];

/**
 * @brief Detects collision between two axis-aligned rectangles using AABB logic.
 * 
 * @param x1 X position of the first rectangle
 * @param y1 Y position of the first rectangle
 * @param w1 Width of the first rectangle
 * @param h1 Height of the first rectangle
 * @param x2 X position of the second rectangle
 * @param y2 Y position of the second rectangle
 * @param w2 Width of the second rectangle
 * @param h2 Height of the second rectangle
 * @return true if the rectangles overlap, false otherwise
 */

bool aabb_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
}

/**
 * @brief Checks and handles collisions between the player, platforms, and the goal.
 * 
 * This function updates the player's grounded state by checking for collisions 
 * with all platforms. If the player is falling and collides with a platform, 
 * their vertical velocity is reset and they are marked as on the ground.
 * 
 * It also checks if the player reaches the goal, and updates the game state to Win.
 * 
 * @param player Reference to the Player object being checked for collisions.
 * @param platforms Pointer to the array of Platform objects.
 * @param platform_count Number of platforms in the array.
 * @param goal Reference to the Goal object to check for level completion.
 */

void check_collisions(Player &player, Platform *platforms, int platform_count, Goal &goal)
{
    player.on_ground = false;
    for (int i = 0; i < platform_count; ++i)
    {
        Platform &plat = platforms[i];
        if (aabb_collision(player.x, player.y, player.width, player.height, plat.x, plat.y, plat.width, plat.height))
        {
            if (player.vy >= 0)
            {
                player.y = plat.y - player.height;
                player.vy = 0;
                player.on_ground = true;
            }
        }
    }
  
    if (aabb_collision(player.x, player.y, player.width, player.height, goal.x, goal.y, goal.width, goal.height))
    {
        level_needs_loading = true;
        current_level++;

        if (current_level > 3)
        {
            current_state = Win;
        }
        else if (current_level == 1)
        {

            player = {100, 400, 0, 0, 40, 40, false};


        }
        else if (current_level == 2)
        {
            player = {100, 300, 0, 0, 40, 40, false};
        }
        else if (current_level == 3)
        {
            player = {100, 200, 0, 0, 40, 40, false};
        }
    }    

}


/**
 * @brief Draws all game elements including background, player, platforms, and goal.
 *
 * This function handles rendering of the entire game screen. It draws the background first,
 * then selects the appropriate player sprite based on movement direction (left, right, or idle),
 * followed by rendering each platform and the goal object.
 *
 * @param player The player object containing position, velocity, and size.
 * @param platforms Array of platforms to be drawn in the scene.
 * @param platform_count The total number of platforms in the array.
 * @param goal The goal object to be drawn at the end of the level.
 */

void draw_game(const Player &player, const Platform *platforms, int platform_count, const Goal &goal)
{

    draw_bitmap(bitmap_named("background"), 0, 0);

    if(player.vx > 0)
        draw_bitmap(bitmap_named("player_right"), player.x, player.y);

    else if(player.vx < 0)
        draw_bitmap(bitmap_named("player_left"), player.x, player.y);
    
    else
        draw_bitmap(bitmap_named("player_still"), player.x, player.y);
    for (int i = 0; i < platform_count; ++i)
    {
        draw_bitmap(bitmap_named("platform"), platforms[i].x, platforms[i].y-8);
    }
    
    if(current_level==3)
    {
        draw_bitmap(bitmap_named("goal"), goal.x, goal.y+5);
    }


}

/**
 * @brief Handles keyboard input to control the player's movement.
 * 
 * Sets the player's horizontal velocity based on left/right key presses.
 * If the spacebar is pressed and the player is on the ground, a jump is triggered
 * by applying an upward vertical velocity.
 * 
 * @param p Reference to the Player object whose movement will be controlled.
 */


void handle_input(Player &p)
{
    if (key_down(LEFT_KEY))      p.vx = -5;
    else if (key_down(RIGHT_KEY)) p.vx = 5;
    else                          p.vx = 0;

    if (key_typed(SPACE_KEY) && p.on_ground)
        p.vy = -15;
}

/**
 * @brief Applies basic physics to the player, including gravity and boundary constraints.
 * 
 * Updates the player's position based on current velocity. Applies gravity by incrementing
 * vertical velocity. Ensures the player stays within horizontal screen bounds and checks 
 * if the player has fallen below the screen, which triggers a game over.
 * 
 * @param p Reference to the Player object whose physics will be applied.
 */

void apply_physics(Player &p)
{

    p.vy += 1;
    p.y  += p.vy;
    p.x  += p.vx;

    
    if (p.x < 0) {p.x = 0;}
    if (p.x + p.width > SCREEN_WIDTH)   p.x = SCREEN_WIDTH - p.width;

    if (p.y + p.height >= SCREEN_HEIGHT)
    {
        current_state = GameOver;
        return; 
    }
}

/**
 * @brief Loads all bitmap resources used in the game.
 * 
 * This function should be called once at the start of the game to ensure all
 * necessary images are available for rendering the player, platforms, goal, and background.
 */

void load_bitmaps()
{
    load_bitmap("platform", "/Users/rex/Documents/Study_Material/SIT102_Intro_to_programming/D4/Resources/new_super_mario_bros_wii_conceptart_tKE4P Background Removed.png");
    load_bitmap("goal", "/Users/rex/Documents/Study_Material/SIT102_Intro_to_programming/D4/Resources/136-1366547_simple-sonic-clipart-image-checkpoint-flag-mariowiki-nintendo-switch-game-card-case Background Removed.png");
    load_bitmap("player_right", "/Users/rex/Documents/Study_Material/SIT102_Intro_to_programming/D4/Resources/pngimg.com - mario_PNG88.png");
    load_bitmap("player_left", "/Users/rex/Documents/Study_Material/SIT102_Intro_to_programming/D4/Resources/pngimg.com - mario_PNG88 copy.png");
    load_bitmap("player_still", "/Users/rex/Documents/Study_Material/SIT102_Intro_to_programming/D4/Resources/980-9809169_super-mario.png");
    load_bitmap("background", "/Users/rex/Documents/Study_Material/SIT102_Intro_to_programming/D4/Resources/the-skyboxes-of-super-mario-sunshine-are-beautiful-v0-mldawocl4wqe1.png");

}




void load_level(int level,Goal &goal, Platform *platforms, int previous_goal_y)
{


    if (level == 1)
    {

        goal = {730, 250, 28, 40};

        platforms[0] = {0, 500, 200, 67};
        platforms[1] = {350, 400, 200, 67};
        platforms[2] = {650, 300, 200, 67};
    }
    else if (level == 2)
    {

        goal = {680, 200, 28, 40};

        platforms[0] = {0, 300, 200, 67};
        platforms[1] = {350, 350, 200, 67};
        platforms[2] = {650, 250, 200, 67};
    }
    else if (level == 3)
    {

        goal = {720, 220, 28, 40};

        platforms[0] = {0, 250, 200, 67};
        platforms[1] = {300, 400, 200, 67};
        platforms[2] = {600, 300, 200, 67};
    }
    else
    {
        current_state = Win;
    }
}

/**
 * @brief Main game loop for the 2D platformer.
 * 
 * Sets up the game window, initializes game objects, processes input,
 * updates game logic, and renders the current state in a loop until the window is closed.
 * 
 * @return Exit status code.
 */

int main()
{

    load_bitmaps();
    open_window("2D Platformer", SCREEN_WIDTH, SCREEN_HEIGHT);

    music game_music = load_music("background_music", "/Users/rex/Documents/Study_Material/SIT102_Intro_to_programming/D4/Resources/super-mario-bros-music.mp3");
    if(current_state == Playing)
     {      
        play_music(game_music, true);  
     }
    Player player;
    player = {100, 400, 0, 0, 40, 40, false}; 
    Goal   goal;
    Platform platforms[NUM_PLATFORMS];


    while (!window_close_requested("2D Platformer"))
    {
        process_events();

        if (current_state == Playing)
        {
            handle_input(player);
            apply_physics(player);
            check_collisions(player, platforms, NUM_PLATFORMS, goal);
        }
        
        if (current_state == GameOver && key_typed(SPACE_KEY))
        {
            current_state = Playing;
            current_level = 1;            
            level_needs_loading = true; 
            player = {100, 400, 0, 0, 40, 40, false};        
        }


        if (level_needs_loading)
        {
            load_level(current_level, goal, platforms, goal.y);
            level_needs_loading = false;
        }

        clear_screen(COLOR_SKY_BLUE);
        draw_game(player, platforms, NUM_PLATFORMS, goal);
         

        if (current_state == Win)
            draw_text("You Win!", COLOR_BLACK, "Arial", 256, 330, 250);

        if (current_state == GameOver)
            draw_text("Game Over!", COLOR_RED, "Arial", 50, 330, 250);

        if (current_state == GameOver)
        {
            draw_text("Game Over!", COLOR_RED, "Arial", 32, 330, 250);
            draw_text("Press Space to Restart", COLOR_BLACK, "Arial", 100, 300, 350);
        }
        refresh_screen(60);
    }
    return 0;
}