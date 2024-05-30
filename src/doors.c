#include "doors.h"

extern DoorInfo doorInfo[];
extern int doorInfoCount;
extern PlateInfo plateInfo[];
extern int plateInfoCount;

DoorInfo getDoorInfo(Vector2i pos){
    for (int i = 0; i < doorInfoCount; i++){
        if (vectorCompare(doorInfo[i].pos, pos)){
            return doorInfo[i];
        }
    }
    DoorInfo d = (DoorInfo){.id = -1};
    return d;
}

PlateInfo getPlateInfo(Vector2i pos){
    for (int i = 0; i < plateInfoCount; i++){
        if (vectorCompare(plateInfo[i].pos, pos)){
            return plateInfo[i];
        }
    }
    PlateInfo p = (PlateInfo){.id = -1};
    return p;
}

void updateDoors(int id, bool undo){
    for (int i = 0; i < doorInfoCount; i++){
        if (doorInfo[i].id == id){
            undo ? doorInfo[i].cur-- : doorInfo[i].cur++;
            // if (doorInfo[i].cur >= doorInfo[i].min){
            //     doorInfo[i].cur = 0;
            // }
        }
    }
}

void readDoors(){
    FILE *file = fopen("data/doors.txt", "r");
    char line[100];

    if (file == NULL){
        printf("Error: Could not open doors.txt\n");
        exit(1);
    }
    while (fgets(line, 100, file) != NULL){
        int x, y, id, min;
        sscanf(line, "%d %d %d %d", &x, &y, &id, &min);
        doorInfo[doorInfoCount++] = (DoorInfo){
            .pos = (Vector2i){x, y},
            .id = id,
            .min = min,
            .cur = 0,
            .onTop = 0
        };
    }
    
    fclose(file);
    return;
}

void readPlates(){
    FILE *file = fopen("data/plates.txt", "r");
    char line[100];

    if (file == NULL){
        printf("Error: Could not open plates.txt\n");
        exit(1);
    }
    while (fgets(line, 100, file) != NULL){
        int x, y, id;
        sscanf(line, "%d %d %d", &x, &y, &id);
        plateInfo[plateInfoCount++] = (PlateInfo){
            .pos = (Vector2i){x, y},
            .id = id
        };
    }
    
    fclose(file);
    return;
}

bool doorInfoExists(Vector2i pos){
    for (int i = 0; i < doorInfoCount; i++){
        if (vectorCompare(doorInfo[i].pos, pos)){
            return true;
        }
    }
    return false;
}

bool plateInfoExists(Vector2i pos){
    for (int i = 0; i < plateInfoCount; i++){
        if (vectorCompare(plateInfo[i].pos, pos)){
            return true;
        }
    }
    return false;
}
