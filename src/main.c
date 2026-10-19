#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "planet.h"
#include "player.h"

#define TEXTURE_SIZE 8
#define DEFAULT_ZOOM 4

typedef struct {
    int x;
    int y;
} IntVec2;

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
IntVec2 getMousePosPlanet(Camera2D* camera, bool planetMode/*TODO*/) {
    Vector2 mScreen = GetMousePosition();
    Vector2 mGlobal = {
        camera->target.x + (mScreen.x-camera->offset.x)/camera->zoom,
        camera->target.y + (mScreen.y-camera->offset.y)/camera->zoom
    };
    return (IntVec2){
        floor(mGlobal.x/TEXTURE_SIZE),
        floor(mGlobal.y/TEXTURE_SIZE)
    };
}
void updateCamera(Camera2D* camera, Planet* planet, Player* player, bool planetMode) {
    if (planetMode) {
        float tileAngle = PI*2/planet->width;
        float py = player->y + 1;
        float r = getRadius(tileAngle, planet->height, py)*TEXTURE_SIZE;
        float a = getAngle(tileAngle, player->x+0.5);
        float x = cos(a)*r;
        float y = sin(a)*r;
        camera->target.x = x;
        camera->target.y = y;
        camera->rotation = -radToDeg(a) + 90;
        camera->zoom = DEFAULT_ZOOM/(getRadius(tileAngle, planet->height, py+1)
                                   - getRadius(tileAngle, planet->height, py));
    } else {
        camera->target = (Vector2){
            (player->x)*TEXTURE_SIZE,
            (player->y)*TEXTURE_SIZE
        };
        camera->rotation = 0;
        camera->zoom = DEFAULT_ZOOM;
    }
}
void updatePlayer(Player* player, Planet* planet) {
    float speed = 0.1;
    if (IsKeyDown(KEY_LEFT_SHIFT)) speed = 0.3;

    if (IsKeyDown(KEY_A)) player->x -= speed;
    if (IsKeyDown(KEY_D)) player->x += speed;
    if (IsKeyDown(KEY_W)) player->y -= speed;
    if (IsKeyDown(KEY_S)) player->y += speed;

    if (player->y < 0) player->y = 0;
    if (player->y > planet->height-2) player->y = planet->height-2;
}
void updateConfig(bool* planetMode, bool* debugMenu) {
    if (IsKeyPressed(KEY_Q)) *planetMode = !*planetMode;
    if (IsKeyPressed(KEY_E)) *debugMenu = !*debugMenu;
}

    
void updatePlanet(Planet* planet, IntVec2 mousePlanet) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (
            mousePlanet.x >= 0 && mousePlanet.y >= 0 &&
            mousePlanet.x < planet->width && mousePlanet.y < planet->height
        ) {
            int index = mousePlanet.x + mousePlanet.y*planet->width;
            planet->blocks[index] = 0;
        }
    }
}
// ---------------------------------------- //
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

    Player player = newPlayer(1, planet.height/2-1);

    Camera2D camera = {0};
    camera.offset = (Vector2){SCREEN_WIDTH/2, SCREEN_HEIGHT/2};
    camera.zoom = DEFAULT_ZOOM;

    // ------------------------------
    Shader planetShader = LoadShader("planet.vs", 0);
    
    float tileAngle = 2*PI/planet.width;
    float textureSize = TEXTURE_SIZE;
    float planetWidthPx = planet.width*TEXTURE_SIZE;
    float planetHeight = planet.height;

    int worldWidthPxLoc = GetShaderLocation(planetShader, "worldWidthPx");
    int worldHeightLoc = GetShaderLocation(planetShader, "worldHeight");
    int tileAngleLoc = GetShaderLocation(planetShader, "angleBetweenTiles");
    int textureSizeLoc = GetShaderLocation(planetShader, "textureSize");

    SetShaderValue(planetShader, worldWidthPxLoc, &planetWidthPx, SHADER_UNIFORM_FLOAT);
    SetShaderValue(planetShader, worldHeightLoc, &planetHeight, SHADER_UNIFORM_FLOAT);
    SetShaderValue(planetShader, tileAngleLoc, &tileAngle, SHADER_UNIFORM_FLOAT);
    SetShaderValue(planetShader, textureSizeLoc, &textureSize, SHADER_UNIFORM_FLOAT);

    bool planetMode = true;
    bool debugMenu  = true;

    // -------------- game loop --------------- //
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        // update
        IntVec2 mousePlanet = getMousePosPlanet(&camera, planetMode);

        updateConfig(&planetMode, &debugMenu);
        updatePlayer(&player, &planet);
        updateCamera(&camera, &planet, &player, planetMode);
        updatePlanet(&planet, mousePlanet);

        // render
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);
        if (planetMode) BeginShaderMode(planetShader);

        planetRender(&planet, blockTextures, TEXTURE_SIZE);

        DrawTexture(
            playerTexture,
            (int)(player.x*TEXTURE_SIZE),
            (int)(player.y*TEXTURE_SIZE),
            WHITE
        );
                
        // cursor marker
        DrawRectangle(
            mousePlanet.x*TEXTURE_SIZE, mousePlanet.y*TEXTURE_SIZE,
            TEXTURE_SIZE, TEXTURE_SIZE, (Color){255, 255, 255, 32}
        );

        if (planetMode) EndShaderMode();
        EndMode2D();

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
    UnloadShader(planetShader);
    planetUnload(&planet);

    CloseWindow();

    return 0;
}
