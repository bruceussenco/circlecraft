#ifndef PLAYER_H
#define PLAYER_H

typedef struct {
    float x;
    float y;
    int hotbar[9];
} Player;

Player newPlayer(float x, float y) {
    return (Player){x, y, {0}};
}

#endif
