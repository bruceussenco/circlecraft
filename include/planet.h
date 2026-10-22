#include <stdbool.h>
#include "raylib.h"

#ifndef PLANET_H
#define PLANET_H

typedef struct {
    int width;
    int height;
    int *blocks;
} Planet;

bool planetInit(Planet *planet, int width, int height);
void planetGenerate(Planet *planet);
void planetRender(Planet *planet, Texture2D *textures, int texture_size, int i0, int j0, int i1, int j1);
void planetUnload(Planet *planet);

#endif
