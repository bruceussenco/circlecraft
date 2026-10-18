#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "planet.h"

#define TEXTURE_SIZE 8

typedef struct {
    float x;
    float y;
} Player;

float getRadius(float tileAngle, int worldHeight, float y) {
    return (1/tileAngle)*pow(1.0+tileAngle, worldHeight-y);
}

float getAngle(float tileAngle, float x) {
    return tileAngle * x - PI/2;
}

float radToDeg(float angle) {
    return angle/PI*180;
}

// ---------- update ------------ //
void update(Player* player, Camera2D* camera, Planet* planet,
            bool* planetMode, bool* debugMenu) {
    /*camera->zoom = expf(logf(camera->zoom) + ((float)GetMouseWheelMove()*0.1f));
    if (camera->zoom > 10.0f) camera->zoom = 10.0f;
    else if (camera->zoom < 0.5f) camera->zoom = 0.5f;*/
    if (IsKeyPressed(KEY_Q)) *planetMode = !*planetMode;
    if (IsKeyPressed(KEY_E)) *debugMenu = !*debugMenu;

    float speed = 0.1;
    if (IsKeyDown(KEY_LEFT_SHIFT)) speed = 0.3;

    if (IsKeyDown(KEY_A)) player->x -= speed;
    if (IsKeyDown(KEY_D)) player->x += speed;
    if (IsKeyDown(KEY_W)) player->y -= speed;
    if (IsKeyDown(KEY_S)) player->y += speed;

    if (player->y < 0) player->y = 0;
    if (player->y > planet->height-2) player->y = planet->height-2;

    if (*planetMode) { // planet mode
        float tileAngle = PI*2/planet->width;
        float py = player->y + 1;
        float r = getRadius(tileAngle, planet->height, py)*TEXTURE_SIZE;
        float a = getAngle(PI*2/planet->width, player->x+0.5);
        float x = cos(a)*r;
        float y = sin(a)*r;
        camera->target.x = x;
        camera->target.y = y;
        camera->rotation = -radToDeg(a) + 90;
        camera->zoom = 4/(getRadius(tileAngle, planet->height, py+1)
                        - getRadius(tileAngle, planet->height, py));
    } else { // flat mode
        camera->target = (Vector2){
            (player->x)*TEXTURE_SIZE,
            (player->y)*TEXTURE_SIZE
        };
        camera->rotation = 0;
        camera->zoom = 4;
    }
}

int main(void) {
    const int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 450;
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "circlecraft");
    
    Planet planet = {};
    if (!planetInit(&planet, 64, 32)) {
        CloseWindow();
        return 1;
    }
    planetGenerate(&planet);

    const int BLOCK_COUNT = 3;
    Texture2D blockTextures[BLOCK_COUNT+1];
    blockTextures[1] = LoadTexture("textures/grass.png");
    blockTextures[2] = LoadTexture("textures/dirt.png");
    blockTextures[3] = LoadTexture("textures/stone.png");
    Texture2D playerTexture = LoadTexture("textures/player.png");

    Player player = {1.0, planet.height/2-1.0};

    Camera2D camera = {0};
    camera.target = (Vector2){player.x*TEXTURE_SIZE, player.y*TEXTURE_SIZE};
    camera.offset = (Vector2){SCREEN_WIDTH/2, SCREEN_HEIGHT/2};
    camera.rotation = 0;
    camera.zoom = 1;
    camera.zoom = 4;

    Shader planetShader = LoadShader("planet.vs", 0);

    // ------------------------------
    
    float tileAngle = 2*PI/planet.width;
    float innerPlanetRadius = TEXTURE_SIZE/tileAngle;
    float tileSize = TEXTURE_SIZE;
    float planetWidthPx = planet.width*TEXTURE_SIZE;
    float planetHeight = planet.height;

    int planetRadiusLoc = GetShaderLocation(planetShader, "planetRadius");
    int worldWidthPxLoc = GetShaderLocation(planetShader, "worldWidthPx");
    int worldHeightLoc = GetShaderLocation(planetShader, "worldHeight");
    int tileAngleLoc = GetShaderLocation(planetShader, "angleBetweenTiles");
    int tileSizeLoc = GetShaderLocation(planetShader, "tileSize");

    SetShaderValue(planetShader, planetRadiusLoc,
                   &innerPlanetRadius, SHADER_UNIFORM_FLOAT);
    SetShaderValue(planetShader, worldWidthPxLoc,
                   &planetWidthPx, SHADER_UNIFORM_FLOAT);
    SetShaderValue(planetShader, worldHeightLoc,
                   &planetHeight, SHADER_UNIFORM_FLOAT);
    SetShaderValue(planetShader, tileAngleLoc,
                   &tileAngle, SHADER_UNIFORM_FLOAT);
    SetShaderValue(planetShader, tileSizeLoc,
                   &tileSize, SHADER_UNIFORM_FLOAT);

    bool planetMode = true;
    bool debugMenu  = true;

    // -------------- game loop --------------- //
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        // update
        Vector2 mScreen = GetMousePosition();
        Vector2 mGlobal = {
            camera.target.x + (mScreen.x-camera.offset.x)/camera.zoom,
            camera.target.y + (mScreen.y-camera.offset.y)/camera.zoom
        };
        int mPlanetX = floor(mGlobal.x/TEXTURE_SIZE);
        int mPlanetY = floor(mGlobal.y/TEXTURE_SIZE);

        update(&player, &camera, &planet, &planetMode, &debugMenu);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (
                mPlanetX >= 0 && mPlanetY >= 0 &&
                mPlanetX < planet.width && mPlanetY < planet.height
            ) {
                int index = mPlanetX + mPlanetY*planet.width;
                planet.blocks[index] = 0;
            }
        }

        // render
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (planetMode) BeginShaderMode(planetShader);
        BeginMode2D(camera);

        planetRender(&planet, blockTextures, TEXTURE_SIZE);

        DrawTexture(
            playerTexture,
            (int)(player.x*TEXTURE_SIZE),
            (int)(player.y*TEXTURE_SIZE),
            WHITE
        );
                
        // cursor marker
        DrawRectangle(
            mPlanetX*TEXTURE_SIZE, mPlanetY*TEXTURE_SIZE,
            TEXTURE_SIZE, TEXTURE_SIZE, (Color){255, 255, 255, 32}
        );

        EndMode2D();
        if (planetMode) EndShaderMode();
        // player on screen
        /*int x = ((player.x-0.5)*TEXTURE_SIZE - camera.target.x)*camera.zoom + camera.offset.x;
        int y = ((player.y-1)*TEXTURE_SIZE - camera.target.y)*camera.zoom + camera.offset.y;
        DrawTexturePro(
            playerTexture,
            (Rectangle){0, 0, TEXTURE_SIZE, 2*TEXTURE_SIZE},
            (Rectangle){x, y, TEXTURE_SIZE*camera.zoom, 2*TEXTURE_SIZE*camera.zoom},
            (Vector2){0, 0}, 0, WHITE
        );*/

        // ui
        if (debugMenu) {
            DrawRectangle(0, 0, 200, 70, (Color){0, 0, 0, 128});
            DrawText(TextFormat("pos: (%.4f, %.4f)", player.x, player.y), 5, 0, 0, WHITE);
            DrawFPS(10, 20);
        }

        EndDrawing();
    }

    UnloadTexture(playerTexture);
    for (int i = 1; i <= BLOCK_COUNT; i++) UnloadTexture(blockTextures[i]);

    planetUnload(&planet);

    CloseWindow();

    return 0;
}
