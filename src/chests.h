#ifndef CHESTS_H
#define CHESTS_H

#include "global.h"

typedef struct ChestLoot{
    Vector2i pos;
    char loot;
    char text[20];
} ChestLoot;

extern ChestLoot chestLoot[];
extern int chestLootCount;

ChestLoot getChestLoot(Vector2i);
void readChestLoot();
bool chestLootExists(Vector2i);

#endif