#include "raylib.h"
#include <math.h>

#include <stdlib.h>

#include "planet.h"

#define WORLD_WIDTH  16
#define WORLD_HEIGHT 16
#define WORLD_TILE_COUNT WORLD_WIDTH * WORLD_HEIGHT

#define TEXTURE_SIZE 8

typedef struct {
    float x;
    float y;
} Player;

// ---------- update ------------ //
void update(Player* player, Camera2D* camera) {
    camera->zoom = expf(logf(camera->zoom) + ((float)GetMouseWheelMove()*0.1f));
    if (camera->zoom > 10.0f) camera->zoom = 10.0f;
    else if (camera->zoom < 0.5f) camera->zoom = 0.5f;

    float speed = 0.1;
    if (IsKeyDown(KEY_LEFT_SHIFT)) speed = 0.3;

    if (IsKeyDown(KEY_A)) player->x -= speed;
    if (IsKeyDown(KEY_D)) player->x += speed;
    if (IsKeyDown(KEY_W)) player->y -= speed;
    if (IsKeyDown(KEY_S)) player->y += speed;

    camera->target = (Vector2){
        (player->x)*TEXTURE_SIZE,
        (player->y)*TEXTURE_SIZE
    };
}

int main(void) {
    const int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 450;
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "circlecraft");
    
    Planet planet = {};
    if (!planetInit(&planet, WORLD_WIDTH, WORLD_HEIGHT)) {
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

    Player player = {1.0, WORLD_HEIGHT/2-1.0};

    Camera2D camera = {0};
    camera.target = (Vector2){player.x*TEXTURE_SIZE, player.y*TEXTURE_SIZE};
    camera.offset = (Vector2){SCREEN_WIDTH/2, SCREEN_HEIGHT/2};
    camera.rotation = 0;
    camera.zoom = 1;
    camera.zoom = 4;

    // -------------- game loop --------------- //
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        // update
        update(&player, &camera);

        Vector2 mScreen = GetMousePosition();
        Vector2 mGlobal = {
            camera.target.x + (mScreen.x-camera.offset.x)/camera.zoom,
            camera.target.y + (mScreen.y-camera.offset.y)/camera.zoom
        };
        int mPlanetX = mGlobal.x/TEXTURE_SIZE;
        int mPlanetY = mGlobal.y/TEXTURE_SIZE;

        // render
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);

        planetRender(&planet, blockTextures, TEXTURE_SIZE);
                
        // cursor marker
        DrawRectangle(
            mPlanetX*TEXTURE_SIZE, mPlanetY*TEXTURE_SIZE,
            TEXTURE_SIZE, TEXTURE_SIZE, (Color){255, 255, 255, 32}
        );

        EndMode2D();
        // player on screen
        int x = ((player.x-0.5)*TEXTURE_SIZE - camera.target.x)*camera.zoom + camera.offset.x;
        int y = ((player.y-1)*TEXTURE_SIZE - camera.target.y)*camera.zoom + camera.offset.y;
        DrawTexturePro(
            playerTexture,
            (Rectangle){0, 0, TEXTURE_SIZE, 2*TEXTURE_SIZE},
            (Rectangle){x, y, TEXTURE_SIZE*camera.zoom, 2*TEXTURE_SIZE*camera.zoom},
            (Vector2){0, 0}, 0, WHITE
        );


        // ui
        DrawFPS(10, 10);

        EndDrawing();
    }

    UnloadTexture(playerTexture);
    for (int i = 1; i <= BLOCK_COUNT; i++) UnloadTexture(blockTextures[i]);

    planetUnload(&planet);

    CloseWindow();

    return 0;
}
