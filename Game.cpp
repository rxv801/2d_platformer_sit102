#include "splashkit.h"
using std::to_string;

// Global variables
string text; // Text for debugging or display purposes
int current_level = 1; // Tracks the current level of the game
bool level_needs_loading = true; // Flag to indicate if a new level needs to be loaded

/**
 * @brief Represents the player in the game.
 */
struct Player
{
    int x, y, vx, vy, width, height; ///< Position, velocity, and dimensions of the player.
    bool on_ground; ///< Indicates whether the player is on the ground.
};

/**
 * @brief Represents an enemy in the game.
 */
struct Enemy
{
    int x, y, vx, vy, width, height; ///< Position, velocity, and dimensions of the enemy.
    bool on_ground; ///< Indicates whether the enemy is on the ground.
    bool is_alive; ///< Indicates whether the enemy is alive.

    int shadow_left_x, shadow_left_y; ///< Shadow properties for collision detection.
    int shadow_left_width, shadow_left_height;

    int shadow_right_x, shadow_right_y;
    int shadow_right_width, shadow_right_height;

    /**
     * @brief Default constructor to initialize enemy properties.
     */
    Enemy()
    : x(0), y(0), vx(1), vy(0), width(30), height(30), on_ground(false), is_alive(true),
      shadow_left_x(0), shadow_left_y(0), shadow_left_width(2), shadow_left_height(2),
      shadow_right_x(0), shadow_right_y(0), shadow_right_width(2), shadow_right_height(2)
    {}
};

/**
 * @brief Represents a platform in the game.
 */
struct Platform
{
    int x, y, width, height; ///< Position and dimensions of the platform.
};

/**
 * @brief Represents the goal in the game.
 */
struct Goal
{
    int x, y, width, height; ///< Position and dimensions of the goal.
};

/**
 * @brief Represents the state of the game.
 */
enum GameState
{
    Playing, ///< The game is currently being played.
    Win, ///< The player has won the game.
    GameOver ///< The player has lost the game.
};

GameState current_state = Playing; // Tracks the current state of the game

// Constants for screen dimensions and game elements
const int SCREEN_WIDTH = 800; ///< Width of the game window.
const int SCREEN_HEIGHT = 600; ///< Height of the game window.
const int NUM_PLATFORMS = 4; ///< Number of platforms in the game.
const int NUM_ENEMIES = 1; ///< Number of enemies in the game.

Platform platforms[NUM_PLATFORMS]; // Array to store platforms

/**
 * @brief Checks for axis-aligned bounding box (AABB) collision between two rectangles.
 *
 * @param x1 X-coordinate of the first rectangle.
 * @param y1 Y-coordinate of the first rectangle.
 * @param w1 Width of the first rectangle.
 * @param h1 Height of the first rectangle.
 * @param x2 X-coordinate of the second rectangle.
 * @param y2 Y-coordinate of the second rectangle.
 * @param w2 Width of the second rectangle.
 * @param h2 Height of the second rectangle.
 * @return true if the rectangles collide, false otherwise.
 */
bool aabb_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
}

/**
 * @brief Handles the behavior and movement of an enemy.
 *
 * @param e The enemy to handle.
 * @param platforms Array of platforms in the game.
 * @param platform_count Number of platforms in the game.
 * @param player The player in the game.
 */
void handle_enemy(Enemy &e, const Platform *platforms, int platform_count, Player &player)
{
    // Apply gravity to the enemy
    e.vy += 1;
    e.y += e.vy;

    e.on_ground = false;

    // Check collision with platforms
    for (int i = 0; i < platform_count; ++i)
    {
        const Platform &plat = platforms[i];
        if (aabb_collision(e.x, e.y, e.width, e.height, plat.x, plat.y, plat.width, plat.height))
        {
            if (e.vy >= 0)
            {
                e.y = plat.y - e.height;
                e.vy = 0;
                e.on_ground = true;
            }
        }
    }

    e.x += e.vx;

    // Update shadow properties for collision detection
    e.shadow_left_x = e.x - 2;
    e.shadow_left_y = e.y + e.height;
    e.shadow_left_width = 2;
    e.shadow_left_height = 2;

    e.shadow_right_x = e.x + e.width;
    e.shadow_right_y = e.y + e.height;
    e.shadow_right_width = 2;
    e.shadow_right_height = 2;

    bool left_on_platform = false;
    bool right_on_platform = false;

    // Check if shadows are on platforms
    for (int i = 0; i < platform_count; ++i)
    {
        const Platform &plat = platforms[i];

        if (aabb_collision(e.shadow_left_x, e.shadow_left_y, e.shadow_left_width, e.shadow_left_height, plat.x, plat.y, plat.width, plat.height))
        {
            left_on_platform = true;
        }

        if (aabb_collision(e.shadow_right_x, e.shadow_right_y, e.shadow_right_width, e.shadow_right_height, plat.x, plat.y, plat.width, plat.height))
        {
            right_on_platform = true;
        }
    }

    // Reverse direction if one shadow is not on a platform
    if (!left_on_platform || !right_on_platform)
    {
        e.vx = -e.vx;
    }

    // Game Over if enemy collides with the player
    if (!e.is_alive) return;

    // Stomp logic
    if (aabb_collision(e.x, e.y, e.width, e.height, player.x, player.y, player.width, player.height))
    {
        if (player.y + player.height <= e.y + 10) // check if player is above
        {
            e.is_alive = false;
            player.vy = -10; // give player bounce after stomp
        }
        else
        {
            current_state = GameOver;
        }
    }
}


/**
 * @brief Draws the game elements, including the player, platforms, goal, and enemy.
 *
 * @param player The player in the game.
 * @param platforms Array of platforms in the game.
 * @param platform_count Number of platforms in the game.
 * @param goal The goal in the game.
 * @param enemy The enemy in the game.
 */
void draw_game(const Player &player, const Platform *platforms, int platform_count, const Goal &goal, const Enemy &enemy)
{
    draw_bitmap(bitmap_named("background"), 0, 0);

    // Draw the enemy bitmap
    if (enemy.is_alive)
        draw_bitmap(bitmap_named("enemy"), enemy.x, enemy.y); // Draw the enemy bitmap

    // Remove the debugging rectangle for the enemy
    // fill_rectangle(COLOR_RED, enemy.x, enemy.y, enemy.width, enemy.height); // Debugging rectangle for enemy

    // Draw the player based on its state
    if (player.vx > 0 && player.on_ground)
        draw_bitmap(bitmap_named("player_right"), player.x, player.y);
    else if (player.vx < 0 && player.on_ground)
        draw_bitmap(bitmap_named("player_left"), player.x, player.y);
    else if (player.on_ground == false && player.vx > 0)
        draw_bitmap(bitmap_named("player_in_air_right"), player.x, player.y);
    else if (player.on_ground == false && player.vx < 0)
        draw_bitmap(bitmap_named("player_in_air_left"), player.x, player.y);
    else if (player.on_ground == false && player.vx == 0)
        draw_bitmap(bitmap_named("player_in_air_right"), player.x, player.y);
    else
        draw_bitmap(bitmap_named("player_still"), player.x, player.y);

    // Draw platforms
    for (int i = 0; i < platform_count; ++i)
    {
        draw_bitmap(bitmap_named("platform"), platforms[i].x - 10, platforms[i].y - 8);
    }

    // Draw the goal
    if (current_level == 3)
        draw_bitmap(bitmap_named("goal"), goal.x, goal.y + 5);

    // Debugging: Draw player and platform boundaries
    draw_rectangle(COLOR_RED, player.x, player.y, player.width, player.height); // Debugging player position
    for (int i = 0; i < platform_count; ++i)
    {
        draw_rectangle(COLOR_RED, platforms[i].x, platforms[i].y, platforms[i].width, platforms[i].height);
    }
}

void handle_player(Player &p, Platform *platforms, int platform_count, Goal &goal)
{

    if (key_down(LEFT_KEY))
        p.vx = -5;
    else if (key_down(RIGHT_KEY))
        p.vx = 5;
    else
        p.vx = 0;

    if (key_typed(SPACE_KEY) && p.on_ground)
        p.vy = -15;

    // Apply gravity and physics
    p.vy += 1;
    p.y += p.vy;
    p.x += p.vx;

    // Screen boundaries
    if (p.x < 0)
        p.x = 0;
    if (p.x + p.width > SCREEN_WIDTH)
        p.x = SCREEN_WIDTH - p.width;

    if (p.y + p.height >= SCREEN_HEIGHT)
    {
        current_state = GameOver;
        return;
    }

    p.on_ground = false;

    // Platform collision
    for (int i = 0; i < platform_count; ++i)
    {
        Platform &plat = platforms[i];
        if (aabb_collision(p.x, p.y, p.width, p.height, plat.x, plat.y, plat.width, plat.height))
        {
            if (p.vy >= 0)
            {
                p.y = plat.y - p.height;
                p.vy = 0;
                p.on_ground = true;
            }
        }
    }

    // Goal collision
    if (aabb_collision(p.x, p.y, p.width, p.height, goal.x, goal.y, goal.width, goal.height))
    {
        level_needs_loading = true;
        current_level++;

        if (current_level > 3)
        {
            current_state = Win;
        }
        else
        {
            if (current_level == 1)
                p = {-15, 400, 0, 0, 30, 40, false};
            else if (current_level == 2)
                p = {-15, 300, 0, 0, 30, 40, false};
            else if (current_level == 3)
                p = {100, 200, 0, 0, 30, 40, false};
        }
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
    load_bitmap("enemy", "Resources/Goomba.png");
}

void load_level(int level, Goal &goal, Platform *platforms, Enemy *enemies)
{
    if (level == 1)
    {
        goal = {772, 0, 28, 300};

        enemies[0].x = 350; // Set enemy position
        enemies[0].y = 350;
        enemies[0].is_alive = true;

        platforms[0] = {-15, 500, 180, 67};
        platforms[1] = {300, 400, 180, 67};
        platforms[2] = {615, 300, 180, 67};
    }
    else if (level == 2)
    {
        goal = {772, 0, 28, 300};

        enemies[0].x = 450; // Set enemy position
        enemies[0].y = 350;
        enemies[0].is_alive = true;

        platforms[0] = {-15, 300, 180, 67};
        platforms[1] = {350, 350, 180, 67};
        platforms[2] = {650, 250, 180, 67};
    }
    else if (level == 3)
    {
        goal = {772, 225, 28, 40};

        enemies[0].x = 470; // Set enemy position
        enemies[0].y = 200;
        enemies[0].is_alive = true;

        platforms[0] = {-15, 250, 180, 67};
        platforms[2] = {650, 250, 180, 67};
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
    // Declare and initialize enemies
    Enemy enemies[NUM_ENEMIES];
    enemies[0].x = 350;
    enemies[0].y = 350;

    load_bitmaps();
    open_window("2D Platformer", SCREEN_WIDTH, SCREEN_HEIGHT);

    music game_music = load_music("background_music", "Resources/super-mario-bros-music.mp3");
    Player player;
    player = {-15, 400, 0, 0, 30, 40, false}; // Width reverted to 40
    Goal goal;
    Platform platforms[NUM_PLATFORMS];

    while (!window_close_requested("2D Platformer"))
    {
        process_events();

        handle_music(current_state, game_music);

        if (current_state == Playing)
        {
            handle_player(player, platforms, NUM_PLATFORMS, goal);

            // Handle each enemy
            for (int i = 0; i < NUM_ENEMIES; ++i)
            {
                handle_enemy(enemies[i], platforms, NUM_PLATFORMS, player);
            }
        }

        if (current_state == GameOver && key_typed(SPACE_KEY))
        {
            current_state = Playing;
            current_level = 1;
            level_needs_loading = true;
            player = {-15, 400, 0, 0, 30, 40, false}; // Width reverted to 40
        }

        if (level_needs_loading)
        {
            load_level(current_level, goal, platforms, enemies);
            level_needs_loading = false;
        }

        clear_screen(COLOR_SKY_BLUE);

        draw_game(player, platforms, NUM_PLATFORMS, goal, enemies[0]);

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