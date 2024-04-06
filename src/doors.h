#ifndef DOORS_H
#define DOORS_H

#include "global.h"

typedef struct DoorInfo{
    Vector2i pos;
    int id;
    int min;
    int cur;
    char onTop;
} DoorInfo;

typedef struct PlateInfo{
    Vector2i pos;
    int id;
} PlateInfo;

extern DoorInfo doorInfo[];
extern int doorInfoCount;
extern PlateInfo plateInfo[];
extern int plateInfoCount;

DoorInfo getDoorInfo(Vector2i);
PlateInfo getPlateInfo(Vector2i);

void updateDoors(int, bool);

void readDoors();
void readPlates();

bool doorInfoExists(Vector2i);
bool plateInfoExists(Vector2i);

#endif