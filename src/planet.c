#include <stdlib.h>
#include "planet.h"
#include "raylib.h"

bool planetInit(Planet *planet, int width, int height) {
    if (width <= 0 || height <= 0) return false;

    planet->width  = width;
    planet->height = height;
    planet->blocks = malloc(
        (size_t)width * (size_t)height * sizeof *planet->blocks
    );

    if (planet->blocks == NULL) {
        planet->width = 0;
        planet->height = 0;
        return false;
    }

    return true;
}

void planetGenerate(Planet *planet) {
    for (int y = 0; y < planet->height; y++) {
        for (int x = 0; x < planet->width; x++) {
            int index = y*planet->width + x;
            if (y == 0)
                planet-> blocks[index] = 1;
            else if (y > planet->height/2 + 2)
                planet->blocks[index] = 3;
            else if (y > planet->height/2)
                planet->blocks[index] = 2;
            else if (y == planet->height/2)
                planet->blocks[index] = 1;
            else
                planet->blocks[index] = 0;
        }
    }
}

int mymod(int a, int b) {
    return (a % b + b) % b;
}
void planetRender(Planet *planet, Texture2D *textures, int texture_size,
        int i0, int j0, int i1, int j1) {
    //for (int j = 0; j < planet->height; j++) {
    //    for (int i = 0; i < planet->width; i++) {
    if (j0 < 0) j0 = 0;
    if (j1 > planet->height) j1 = planet->height;
    for (int j = j0; j < j1; j++) {
        for (int i = i0; i < i1; i++) {
            int index = mymod(i, planet->width) + j*planet->width;
            int block_id = planet->blocks[index];
            if (block_id == 0) continue;

            int x = i*texture_size;
            int y = j*texture_size;
            DrawTexture(textures[block_id], x, y, WHITE);
        }
    }
}

void planetUnload(Planet *planet) {
    free(planet->blocks);
    planet->blocks = NULL;
    planet->width = 0;
    planet->height = 0;
}

