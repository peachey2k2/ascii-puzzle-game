#ifndef TELEPORTERS_H
#define TELEPORTERS_H

#include "global.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct TeleporterInfo{
    Vector2i pos;
    Vector2i dest;
} TeleporterInfo;

extern TeleporterInfo teleporterInfo[];
extern int teleporterInfoCount;

TeleporterInfo getTeleporterInfo(Vector2i);
void readTeleporters();
bool teleporterInfoExists(Vector2i);

#endif