#include "teleporters.h"

void readTeleporters(){
    FILE *file = fopen("data/teleporters.txt", "r");
    char line[100];

    if (file == NULL){
        printf("Error: Could not open teleporters.txt\n");
        exit(1);
    }
    while (fgets(line, 100, file) != NULL){
        int x, y, dx, dy;
        sscanf(line, "%d %d %d %d", &x, &y, &dx, &dy);
        teleporterInfo[teleporterInfoCount++] = (TeleporterInfo){
            .pos = (Vector2i){x, y},
            .dest = (Vector2i){dx, dy}
        };
    }
    
    fclose(file);
    return;
}

TeleporterInfo getTeleporterInfo(Vector2i pos){
    for (int i = 0; i < teleporterInfoCount; i++){
        if (vectorCompare(teleporterInfo[i].pos, pos)){
            return teleporterInfo[i];
        }
    }
    TeleporterInfo t = (TeleporterInfo){.pos.x = -1};
    return t;
}

bool teleporterInfoExists(Vector2i pos){
    for (int i = 0; i < teleporterInfoCount; i++){
        if (vectorCompare(teleporterInfo[i].pos, pos)){
            return true;
        }
    }
    return false;
}