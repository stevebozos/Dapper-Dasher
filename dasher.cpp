#include <iostream>
#include "raylib.h"

using namespace std;

// // END OF HEADERS AND NAMESPACES // //
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
    SpriteAnimationData2D is a custom data type to store
    all the data related to the animation of the sprite.

    It contains:

    - the rectangle dimensions to be used to exctract
    the sprite from the spritesheet.
    - The position of the sprite on the screen
    - The frame that we want to choose from the spritesheet.
    - The time that the animation is running.
    - The time that the animation is being updated.

    We create a custom data type using the keyword "struct". 
*/ 

// {Rectangle spriteRectangle, Vector2 position, int frame, float runningTime, float updateTime}
//
// This struct is a blueprint to initialize all the animation
// variables for a 2D sprite.
struct SpriteAnimationData2D
{
    // (Rectangle)
    //
    //The rectangle that contains the sprite.
    Rectangle spriteRectangle; 

    // (Vector2) 
    //
    //The position of the sprite on the canvas.
    Vector2 position;

    // (int) 
    //
    //The current sprite animation of the spritesheet.
    int frame;

    // (float)
    //
    // Timer that keeps time of the animation display on the canvas.
    float runningTime;

    // (float)
    //
    // The amount of time before we update the animation frame
    // We want to update the animation 12 times per second in order to achieve a smooth running animation.
    float updateTime;
};


//////////////////////////////// FUNCTION DECLARATION ////////////////////////////////////////////////////

// Returns true if the sprite's bottom edge is equal or greater than the game window's Y value,
// of false if the sprite's bottom edge is less than the Y value of the game window.
bool IsOnGround(SpriteAnimationData2D spriteData, int window_height)
{
    return spriteData.position.y >= (window_height - spriteData.spriteRectangle.height);
}


// Updates the sprite's values in order to update the coordinates on the spritesheet
// for every frame.
SpriteAnimationData2D UpdateAnimation(SpriteAnimationData2D spriteData, float deltaTime, int maxFrame)
{
    // Update running time
    spriteData.runningTime += deltaTime;

    //Update the player's animation
    // Check if running time has surpassed update time
    if(spriteData.runningTime >= spriteData.updateTime)
    {
        spriteData.runningTime = 0.0f;

        spriteData.spriteRectangle.x = spriteData.frame * spriteData.spriteRectangle.width;
        spriteData.frame++; // Go to the next frame
            
        // Reset the frame to avoid exceeding the spritesheet's width reset frame to 0
        if(spriteData.frame > maxFrame)
        {
            spriteData.frame = 0;
        }
    }

    // Return the updated values of the sprite's animation
    return spriteData;

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////


int main(){

    // // MAIN CANVAS INITIALIZATION // //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // int windowDimensions[] = {512, 380};                                  // Array that holds the window dimensions in pixels {width, heigt}

    const int window_width{512};                                        // Main Canvas width dimensions in pixels
    const int window_height{380};                                       // Main Canvas height dimensions in pixels
    
    InitWindow(window_width, window_height, "Dapper Dahser v1.0");      // Creates a canvas on the screen given the width and heigh in pixels, and the title of the window.
    
    SetTargetFPS(60);                                                   // Set the targer frames per second of the main canvas.
     
    

    // // GAME PHYSICS & VELOCITIES // //
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    const int gravity{1'000};                               // Gravity modifier in pixels per second (The apostrophe is ignored by the compiler. Is used only to make the number more readable.)

    bool isInAir{false};                                    // Check if the player is in the air to avoid double jumping

    int player_jump_velocity{-600};                         // The amount of velocity on the Y axis simulating a jump in (pixels/second)

    int player_velocity{0};                                 // Player's vertical velocity (pixels per second)   

    int enemyNebula_Velocity{-250};                         // Enemy's Nebula horizontal velocity in (pixels per second)



    // // // ANIMATION INITIALIZATION  // // //
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    // // // PLAYER ANIMATION // // //
    Texture2D playerSpritesheet = LoadTexture("textures/scarfy.png");

    // The player's sprite animation frame
    SpriteAnimationData2D playerAnimation{
        {0.0, 0.0, playerSpritesheet.width/6, playerSpritesheet.height},                                                        // Sprite rectangle dimensions
        {window_width/2 - playerAnimation.spriteRectangle.width/2, window_height - playerAnimation.spriteRectangle.height},     // PosX and PosY on the spritesheet
        {0},                                                                                                                    // Current frame
        {0.0f},                                                                                                                 // Timer that keeps time of the animation display
        {1.0f/12.0f}                                                                                                            // Time that the current frame will be changed to the next on the spritesheet
    }; 
    


    // // // ENEMY ANIMATION // // //

    // NEBULA
    Texture2D nebulaSpritesheet = LoadTexture("textures/12_nebula_spritesheet.png"); // Loads the Nebula enemy spritesheet (8x8)
    
    int nebula_number = GetRandomValue(10,20);
    int nebula_frequency = GetRandomValue(350,600);
    //int nebula_counter{nebula_number};

    // The array enemies_nebula holds all the nebula animation data.
    SpriteAnimationData2D enemies_nebula[nebula_number]{};

    for(int i=0; i<nebula_number; i++){

        // Rectangle Dimension (x, y, width, height)
        enemies_nebula[i].spriteRectangle.x = 0.0f;
        enemies_nebula[i].spriteRectangle.y = 0.0f;
        enemies_nebula[i].spriteRectangle.width = nebulaSpritesheet.width/8;
        enemies_nebula[i].spriteRectangle.height = nebulaSpritesheet.height/8;

        // Rectangle position on the X and Y axis. Every nebula has 300px difference between them in the X axis.
        enemies_nebula[i].position.x = window_width + (i * 500);
        enemies_nebula[i].position.y = window_height - enemies_nebula[i].spriteRectangle.height;

        enemies_nebula[i].frame = 0;            //Current frame of the spritesheet
        enemies_nebula[i].runningTime = 0.0f;   //Timer that keeps time of the animation display
        enemies_nebula[i].updateTime = 0.0f;    //The amount of time before we update the animation
    }

    // FINISH LINE ?

    float finishLine{ enemies_nebula[nebula_number - 1].position.x};

    // COLLISION DETECTION
    bool isColliding{};


    // // // ENVIRONMENT ANIMATION // // //

    // BACKGROUND
    Texture2D background = LoadTexture("textures/far-buildings.png");            // Load the far background texture
    // Initiate the X coordinate of the Far Background to 0
    // This variable is going to be used in order to move the background backwards on the X axis.
    float background_1_posX{};
    float background_velocity{20};

    // MIDGROUND
    Texture2D midground = LoadTexture("textures/back-buildings.png");
    float midground_1_posX{};
    float midground_velocity{40};

    // FOREGROUND
    Texture2D foreground = LoadTexture("textures/foreground.png");
    float foreground_1_posX{};
    float foreground_velocity{80};


    // GAME LOOP START //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // While the user does not click on the "X" button loop through the below code //
    while(!WindowShouldClose())
    {
        
        BeginDrawing();                                                         // Start the frame buffer

        ClearBackground(WHITE);                                                 // Refresh the frame buffer with a white backgroud

        float delta_time{GetFrameTime()};                                       // Delta Time (time since last frame)
        

        // // // ENVIRONMENT SIDE SCROLLING // // //

        // BACKGROUND

        background_1_posX -= background_velocity * delta_time;                               // Move the background to the left on the X axis 

        if(background_1_posX <= -background.width * 2){
            background_1_posX = 0.0f;
        }

        
        Vector2 background_1_Position{background_1_posX, 0.0};
        DrawTextureEx(background,background_1_Position, 0.0f, 2.0, WHITE);

        Vector2 background_2_Position{background_1_posX + background.width * 2, 0.0};
        DrawTextureEx(background, background_2_Position, 0.0f, 2.0, WHITE);


        // MIDGROUND
        midground_1_posX -= midground_velocity * delta_time;

        if(midground_1_posX <= -midground.width * 2){
            midground_1_posX = 0.0f;
        }

        Vector2 midground_1_Position{midground_1_posX, 0.0};
        DrawTextureEx(midground, midground_1_Position, 0.0f, 2.0, WHITE);
        Vector2 midground_2_Position{midground_1_posX + midground.width * 2, 0.0};
        DrawTextureEx(midground, midground_2_Position, 0.0f, 2.0, WHITE);

        // FOREGROUND
        foreground_1_posX -= foreground_velocity * delta_time;

        if(foreground_1_posX <= -foreground.width * 2){
            foreground_1_posX = 0.0f;
        }

        Vector2 foreground_1_Position{foreground_1_posX, 0.0};
        DrawTextureEx(foreground, foreground_1_Position, 0.0f, 2.0, WHITE);
        Vector2 foreground_2_Position{foreground_1_posX + foreground.width * 2, 0.0};
        DrawTextureEx(foreground, foreground_2_Position, 0.0f, 2.0, WHITE);



        // // // PLAYER GRAVITY // // //

        // Check every frame if the player is on the ground
        if(IsOnGround(playerAnimation, window_height))
        {
            player_velocity = 0;
            isInAir = false;
        }
        else
        {
            // Apply Gravity to the player if he is not touching the ground
            player_velocity += gravity * delta_time;
            isInAir = true;
        }

        // // // PLAYER INPUT // // //
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // Player Jumping
        if(IsKeyDown(KEY_SPACE) && !isInAir)
        {   
            player_velocity += player_jump_velocity;
        }

        
        // // POSITION UPDATE // //
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        for(int i=0; i<nebula_number; i++)
        {
            enemies_nebula[i].position.x += enemyNebula_Velocity * delta_time;      // Update the nebula enemies position by 250 every frame on the X axis
        }
        
        finishLine += enemyNebula_Velocity * delta_time;                            // Update the finish line  

        playerAnimation.position.y += player_velocity * delta_time;                 // Update the player's velocity by 10 every frame on the Y axis

        



        // // ANIMATION // //
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // ***General formula for updating animation from a spritesheet***
        // x = frame * width/n
        // x -> the animation we want to draw
        // frame -> the frame on the spritesheet
        // width -> the spritesheet's width
        // n -> the number of animations on the spritesheet

        // We want to freeze the animation while the player is in air
        // so the player does not "run" while jumping.
        if(!isInAir)
        {
           playerAnimation = UpdateAnimation(playerAnimation, delta_time, 5);
        }

        // Update the Enemy Nebula Animation
        for(int i=0; i<nebula_number; i++)
        {
            enemies_nebula[i] = UpdateAnimation(enemies_nebula[i], delta_time, 7);
        }

        // COLLISION DETECTION
        // We are going to iterate through all the elements of the nebula array
        // in order to check each time a nebule object touches the player the bool lean isColliding
        // will become true and the textures of the player and the obstacles is going to stop being drawed
        for(SpriteAnimationData2D nebula:enemies_nebula)
        {
            
            // Applying padding to the nebula sprite
            // The reason that we are applying padding is because the sprite has
            // some extra space before the enemy is colliding with the player.
            // So the padding on the sprite should be 50+ one the X axis, 50+ on the Y axis, 

            float enemy_padding{50};

            //Get the coordinates of the enemy rectangle on the screen
            Rectangle nebula_rectangle{
                nebula.position.x + enemy_padding,
                nebula.position.y + enemy_padding,
                nebula.spriteRectangle.width - 2 * enemy_padding,
                nebula.spriteRectangle.height - 2 * enemy_padding
            };

            // Get the coordinates of the player on the screen
            Rectangle player_rectangle{
                playerAnimation.position.x,
                playerAnimation.position.y,
                playerAnimation.spriteRectangle.width,
                playerAnimation.spriteRectangle.height
            };

            if(CheckCollisionRecs(nebula_rectangle, player_rectangle))
            {
                isColliding = true;
            }

            // if(playerAnimation.position.x > nebula.position.x){
            //     nebula_counter--;
            //     DrawText(TextFormat("%i", nebula_counter), 50, 50, 50.0, WHITE);
            // }

        }

        if(isColliding)
        {
            // GAME OVER
            DrawText("Game Over!", (window_width/2)-120, window_height/2, 50.0, RED);

            // Stop the background from scrolling
            background_velocity = 0.0f;
            midground_velocity = 0.0f;
            foreground_velocity = 0.0f;
        }
        else if(playerAnimation.position.x > finishLine)                                        // Winning Condition
        {
            DrawText("You Win!", (window_width/2)-120, window_height/2, 50.0, GOLD);
        }
        else
        {
            // Draw the enemy Nebula
            for(int i=0 ; i<nebula_number; i++)
            {          
                DrawTextureRec(nebulaSpritesheet, enemies_nebula[i].spriteRectangle, enemies_nebula[i].position, WHITE);
            }

            // Draw the player on the screen
            DrawTextureRec(playerSpritesheet, playerAnimation.spriteRectangle, playerAnimation.position, WHITE);
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        

        // Close the frame buffer
        EndDrawing();

    }

    // // GAME LOOP END // //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Unload the texture that is loaded in the Texture buffer.
    // Everytime in Raylib we load a texture at the start of the program,
    // we need to unload the texture at the end of the program.
    UnloadTexture(playerSpritesheet);
    UnloadTexture(nebulaSpritesheet);
    UnloadTexture(background);
    UnloadTexture(midground);
    UnloadTexture(foreground);
    // Close the window that we have initialized before the program ends //
    CloseWindow();

}