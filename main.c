#include "./src/HeaderHandler.h"

void initApp();
void processInput();
void update();
void render(Camera3D camera);
void destroyApp();

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    const char* buildName = "ASPHA DEVELOPMENT BUILD";

    InitWindow(screenWidth, screenHeight, buildName);
    SetWindowOpacity(1);

    Camera3D camera = { 0 };

    SetTargetFPS(60);   
   
    while (!WindowShouldClose())       
    {   
        processInput();
        update();
        render(camera);
    }

    CloseWindow();       

    return 0;
}

void initApp()
{

}

void processInput()
{

}

void update()
{

}

void render(Camera3D camera)
{
    BeginDrawing();
            
        ClearBackground(RED);
            
        BeginMode3D(camera);

            // draw in here

        EndMode3D();

    EndDrawing();
}

void destroyApp()
{

}