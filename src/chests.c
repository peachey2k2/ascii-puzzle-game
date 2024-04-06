#include "chests.h"

extern ChestLoot chestLoot[];
extern int chestLootCount;

ChestLoot getChestLoot(Vector2i pos){
    for (int i = 0; i < chestLootCount; i++){
        if (vectorCompare(chestLoot[i].pos, pos)){
            return chestLoot[i];
        }
    }
    ChestLoot c = (ChestLoot){.loot = '\0'};
    return c;
}

void readChestLoot(){
    FILE *file = fopen("data/chests.txt", "r");
    char line[100];

    if (file == NULL){
        printf("Error: Could not open chests.txt\n");
        exit(1);
    }
    while (fgets(line, 100, file) != NULL){
        int x, y;
        char c;
        char *text;
        sscanf(line, "%d %d %c :", &x, &y, &c);
        chestLoot[chestLootCount++] = (ChestLoot){
            .pos = (Vector2i){x, y},
            .loot = c,
        };
        text = strchr(line, ':')+1;
        strcpy(chestLoot[chestLootCount-1].text, text);
    }
    
    fclose(file);
    return;
}

bool chestLootExists(Vector2i pos){
    for (int i = 0; i < chestLootCount; i++){
        if (vectorCompare(chestLoot[i].pos, pos)){
            return true;
        }
    }
    return false;
}