#ifndef MAIN_H
#define MAIN_H

#include "global.h"
#include "color.h"
#include "dialogue.h"
#include "chests.h"
#include "doors.h"
#include "teleporters.h"

#define BOUNDARY_CHECK(vec, min, max) (vec.x >= min.x && vec.x < max.x && vec.y >= min.y && vec.y < max.y)

enum FLAGS{
    FLAGS_DEATH,
    FLAGS_GHOST_MODE,
    FLAGS_WARP,
    FLAGS_APPLE,
    FLAGS_SANDWICH,
    FLAGS_GLASSES,
};

struct ShaderParams{
    int offset;
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
    MovedObject movedObjects[50];
    int movedObjectCount;
    int flags;
} MovePacket;

const Vector2i windowSize = {640, 480};
const Vector2i mapSize = {1000, 1000};
const Vector2i visibleMapSize = {35, 25}; // 2 extra tiles for each side
const Vector2i center = {17, 12};

float *visibleMapColors;

ChestLoot chestLoot[100];
int chestLootCount;

DoorInfo doorInfo[500];
int doorInfoCount;
PlateInfo plateInfo[500];
int plateInfoCount;

TeleporterInfo teleporterInfo[500];
int teleporterInfoCount;


#endif
