#ifndef MAIN_H
#define MAIN_H

#include "global.h"
#include "color.h"
#include "dialogue.h"
#include "chests.h"
#include "doors.h"

enum FLAGS{
    FLAGS_DEATH,
    FLAGS_ROCK_MOVED,
    FLAGS_GHOST_MODE,
    FLAGS_WARP,
};

struct ShaderParams{
    int modulate;
    int shifting;
};

struct NodePositions{
    Vector2i a;
    Vector2i b;
};

struct UsedItem{
    char item;
    int index;
};

typedef struct MovedObject{
    Vector2i oldPos;
    Vector2i newPos;
    char tile;
} MovedObject;

typedef struct MovePacket{
    int button;
    char items[5];
    Vector2i pos;
    // struct UsedItem usedItem;
    MovedObject movedObjects[10];
    int movedObjectCount;
    int flags;
} MovePacket;

const Vector2i windowSize = {640, 480};
const Vector2i mapSize = {20, 20};
const Vector2i visibleMapSize = {31, 21};
const Vector2i center = {15, 10};

float *visibleMapColors;

ChestLoot chestLoot[100];
int chestLootCount;

DoorInfo doorInfo[500];
int doorInfoCount;
PlateInfo plateInfo[500];
int plateInfoCount;


#endif