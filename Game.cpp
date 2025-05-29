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
const int NUM_PLATFORMS = 4;

Platform platforms[NUM_PLATFORMS];

bool aabb_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
}

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
            player = {-15, 400, 0, 0, 40, 40, false};
        }
        else if (current_level == 2)
        {
            player = {-15, 300, 0, 0, 40, 40, false};
        }
        else if (current_level == 3)
        {
            player = {100, 200, 0, 0, 40, 40, false};
        }
    }    
}



void draw_game(const Player &player, const Platform *platforms, int platform_count, const Goal &goal)
{
    draw_bitmap(bitmap_named("background"), 0, 0);

    if(player.vx > 0 && player.on_ground)
        draw_bitmap(bitmap_named("player_right"), player.x, player.y);

    else if(player.vx < 0 && player.on_ground)
        draw_bitmap(bitmap_named("player_left"), player.x, player.y);
    
    else if(player.on_ground == false && player.vx > 0)
        draw_bitmap(bitmap_named("player_in_air_right"), player.x, player.y);
    else if(player.on_ground == false && player.vx < 0)
        draw_bitmap(bitmap_named("player_in_air_left"), player.x, player.y);
    
    else if(player.on_ground == false && player.vx == 0)
        draw_bitmap(bitmap_named("player_in_air_right"), player.x, player.y);
    
    else
        draw_bitmap(bitmap_named("player_still"), player.x, player.y);

    for (int i = 0; i < platform_count; ++i)
    {
        draw_bitmap(bitmap_named("platform"), platforms[i].x, platforms[i].y-8);
    }
    
    if(current_level == 3)
    {
        draw_bitmap(bitmap_named("goal"), goal.x, goal.y+5);
    }
}

void handle_input(Player &p)
{
    if (key_down(LEFT_KEY))      p.vx = -5;
    else if (key_down(RIGHT_KEY)) p.vx = 5;
    else                          p.vx = 0;

    if (key_typed(SPACE_KEY) && p.on_ground)
        p.vy = -15;
}

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

void load_bitmaps()
{
    load_bitmap("platform", "Resources/new_super_mario_bros_wii_conceptart_tKE4P Background Removed.png");
    load_bitmap("goal", "Resources/136-1366547_simple-sonic-clipart-image-checkpoint-flag-mariowiki-nintendo-switch-game-card-case Background Removed.png");
    load_bitmap("player_right", "Resources/pngimg.com - mario_PNG88.png");
    load_bitmap("player_left", "Resources/pngimg.com - mario_PNG88 copy.png");
    load_bitmap("player_still", "Resources/980-9809169_super-mario.png");
    load_bitmap("background", "Resources/the-skyboxes-of-super-mario-sunshine-are-beautiful-v0-mldawocl4wqe1.png");
    load_bitmap("player_in_air_right", "Resources/Screenshot 2025-05-25 at 14.51 Background Removed.38.png");
    load_bitmap("player_in_air_left", "Resources/Screenshot 2025-05-25 at 14.51 Background Removed.38 copy.png");
}

void load_level(int level, Goal &goal, Platform *platforms, int previous_goal_y)
{
    if (level == 1)
    {
        goal = {772, 0, 28, 300};

        platforms[0] = {-15, 500, 180, 67};
        platforms[1] = {300, 400, 180, 67};
        platforms[2] = {615, 300, 180, 67};
    }
    else if (level == 2)
    {
        goal = {772, 0, 28, 300};

        platforms[0] = {-15, 300, 180, 67};
        platforms[1] = {350, 350, 180, 67};
        platforms[2] = {650, 250, 180, 67};
    }
    else if (level == 3)
    {
        goal = {772, 220, 28, 40};

        platforms[0] = {-15, 250, 180, 67};
        platforms[1] = {300, 400, 180, 67};
        platforms[2] = {615, 300, 180, 67};
    }
    else
    {
        current_state = Win;
    }
}

void handle_music(GameState state, music game_music)
{
    if (state == Playing && !music_playing())
    {
        play_music(game_music, true);
    }
    else if (state != Playing && music_playing())
    {
        stop_music();
    }
}

int main()
{
    load_bitmaps();
    open_window("2D Platformer", SCREEN_WIDTH, SCREEN_HEIGHT);

    music game_music = load_music("background_music", "Resources/super-mario-bros-music.mp3");  
    Player player;
    player = {-15, 400, 0, 0, 40, 40, false};
    Goal   goal;
    Platform platforms[NUM_PLATFORMS];

    while (!window_close_requested("2D Platformer"))
    {
        process_events();

        handle_music(current_state, game_music);

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
            player = {-15, 400, 0, 0, 40, 40, false};
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
        {
            draw_text("Game Over!", COLOR_RED, "Arial", 32, 330, 250);
            draw_text("Press Space to Restart", COLOR_BLACK, "Arial", 100, 300, 350);
        }
        refresh_screen(60);
    }
    return 0;
}