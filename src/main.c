#include "main.h"
#include <math.h>

float *visibleMapColors;

extern ChestLoot chestLoot[];
extern int chestLootCount;

extern DoorInfo doorInfo[];
extern int doorInfoCount;

extern PlateInfo plateInfo[];
extern int plateInfoCount;

Font font;
Shader shader;
struct ShaderParams ShaderParams;
struct NodePositions NodePositions;

Color *shiftingColors;
Color shiftingColor;
int shiftingIndex = 0;

char **map;
char **layer2;
char *visibleMap;
Vector2i playerPos = {0, 0};

MovePacket *moveHistory; // for undo
int moveHistoryIndex = 0;
int undoCount = 0;
int stamina = 999;
float holdTimer = 0;
int consecutiveUndos = 0;
unsigned int inGameClockTimer;

bool dialogue = false;
char **dialogueText;
FILE *dialogueFile;

bool info = false;
char *infoBoxText;

char addedItem = '\0';

char temp[12];
MovePacket movePacket;

bool cycle = false;
// int flags;

int deferredFlags = 0;

Vector2 mousePos;
Vector2i guideBuf[100];

Vector2 glassesFactor = {0, 0};

void killTile(Vector2i pos, char tile, bool prev);
bool isEnemy(char c);

bool isLayer2(char c){
    switch (c){
        case 'R':
        case 'A':
        case 'B':
        case 'e':
        case 'E':
            return true;
        default:
            return false;
    }
}

//flags for managing various game states
//we love flags
bool getFlag(int flag){
    return movePacket.flags & (1 << flag);
}
void setFlag(int flag, bool value){
    movePacket.flags = value ? movePacket.flags | (1 << flag) : movePacket.flags & ~(1 << flag);
}

void setFlagDeferred(int flag, bool value){
    // moveHistory[moveHistoryIndex-1].flags = value ? moveHistory[moveHistoryIndex-1].flags | (1 << flag) : moveHistory[moveHistoryIndex-1].flags & ~(1 << flag);
    if (getFlag(flag) != value){
        deferredFlags = deferredFlags | (1 << flag);
    }
}

Vector2 getScreenPos(Vector2i gridPos){
    return (Vector2){
        (gridPos.x - playerPos.x) * 20 + 320 + glassesFactor.x,
        (gridPos.y - playerPos.y) * 20 + 220 + glassesFactor.y
    };
}

Vector2 getScreenPosNoShift(Vector2i gridPos){
    return (Vector2){(gridPos.x - playerPos.x) * 20 + 320, (gridPos.y - playerPos.y) * 20 + 220};
}

Vector2i keyToVector(int key){
    switch (key){
        case KEY_UP:
            return UP;
        case KEY_DOWN:
            return DOWN;
        case KEY_LEFT:
            return LEFT;
        case KEY_RIGHT:
            return RIGHT;
    }
    return (Vector2i){0, 0};
}

int pressPlate(Vector2i pos){
    PlateInfo plate = getPlateInfo(pos);
    if (plate.id == -1) return 0;
    updateDoors(plate.id, false);
    return 1;
}

int releasePlate(Vector2i pos){
    PlateInfo plate = getPlateInfo(pos);
    if (plate.id == -1) return 0;
    updateDoors(plate.id, true);
    return 1;
}

char getItemInLayer(Vector2i pos, bool layer){
    if (!BOUNDARY_CHECK(pos, ZERO, mapSize)){
        #ifdef DEBUG_BUILD
        printf("getItemInLayer: (%d, %d) is out of bounds\n", pos.x, pos.y);
        #endif
        return 0;
    }
    return layer ? layer2[pos.y][pos.x] : map[pos.y][pos.x];
}

void setItemInLayer(char c, Vector2i pos, bool layer){
    if (!BOUNDARY_CHECK(pos, ZERO, mapSize)){
        #ifdef DEBUG_BUILD
        printf("setItemInLayer: (%d, %d) is out of bounds\n", pos.x, pos.y);
        #endif
        return;
    }
    if (layer){
        layer2[pos.y][pos.x] = c;
    } else {
        map[pos.y][pos.x] = c;
    }
}

char getItem(Vector2i pos){
    return getItemInLayer(pos, layer2[pos.y][pos.x] != '.');
}

void setItem(char c, Vector2i pos){
    setItemInLayer(c, pos, isLayer2(c));
}

char getOnTop(){
    return getItem(playerPos);
}

void setOnTop(char c){
    setItem(c, playerPos);
}

int toVisibleMapPos(Vector2i pos){
    return (visibleMapSize.x + 1) * (pos.y - playerPos.y + center.y) + (pos.x - playerPos.x + center.x);
}

Vector2i toMapPosI(int pos){
    return (Vector2i){(pos % (visibleMapSize.x + 1)) + playerPos.x - center.x, (pos / (visibleMapSize.x + 1)) + playerPos.y - center.y};
}

Vector2i toMapPosV(Vector2i pos){
    return (Vector2i){pos.x + playerPos.x - center.x, pos.y + playerPos.y - center.y};
}

bool isOutOfBounds(Vector2i pos){
    return pos.x < 0 || pos.x >= mapSize.x || pos.y < 0 || pos.y >= mapSize.y;
}

bool isEdge(Vector2i pos){
    return pos.x == -1 || pos.x == mapSize.x || pos.y == - 1 || pos.y == mapSize.y;
}

float getAngleTo(Vector2i a, Vector2i b){
    return atan2(b.y - a.y, -(b.x - a.x)); // -x because the y axis is inverted
}

bool isEnemy(char c){
    switch (c){
        case 'e':
        case 'E':
            return true;
        default:
            return false;
    }
}

bool isMoveable(Vector2i pos, Vector2i move){
    Vector2i newPos = vectorAdd(pos, move);
    if (isOutOfBounds(newPos)) return false;
    char item = getItem(newPos);
    switch (item){
        case '#':
        case 'R':
        case 'A':
        case 'B':
            return false;
        case 'D':
            if (getDoorInfo(newPos).cur < getDoorInfo(newPos).min){
                return false;
            }
            break;
    }
    return true;
}

bool warpFlag = false;
Vector2i conveyorPush(Vector2i pos, Vector2i dir, bool player){
    if (player){
        warpFlag = true;
        movePacket.pos = pos;
    }
    // moveHistory[moveHistoryIndex-1].pos = pos;
    Vector2i newPos = vectorAdd(pos, dir);
    return isMoveable(pos, dir) ? newPos : pos;
}

bool moveTo(int key, bool undo){
    Vector2i move = keyToVector(key);
    Vector2i newPos = vectorAdd(playerPos, (undo ? vectorInvert(move) : move));

    if (move.x == 0 && move.y == 0) return false;
    if (newPos.x < 0 || newPos.x >= mapSize.x || newPos.y < 0 || newPos.y >= mapSize.y) return false;
    // printf("%d,%d -> %d,%d\n", playerPos.x, playerPos.y, newPos.x, newPos.y);

    if (!undo){
        if (!getFlag(FLAGS_GHOST_MODE)){
            if (newPos.x == 164 && newPos.y == 11){
                return false;
            }
            switch (getItem(newPos)){
                case '#':
                    return false;
                case 'D':
                    if (getDoorInfo(newPos).cur < getDoorInfo(newPos).min){
                        return false;
                    }
                    break;
                case 'T':
                    warpFlag = true;
                    movePacket.pos = newPos;
                    newPos = getTeleporterInfo(newPos).dest;
                    break;
                case 'R':
                    Vector2i rockNewPos = vectorAdd(newPos, move);
                    // bool conveyored = false;
                    switch (getItem(rockNewPos)){
                        case '_':
                            pressPlate(rockNewPos);
                            break;
                        case '.':
                            break;
                        case 'D':
                            if (getDoorInfo(rockNewPos).cur < getDoorInfo(rockNewPos).min){
                                return false;
                            }
                            break;
                        case '^':
                            rockNewPos = conveyorPush(rockNewPos, UP, false);
                            // conveyored = true;
                            break;
                        case 'V':
                            rockNewPos = conveyorPush(rockNewPos, DOWN, false);
                            // conveyored = true;
                            break;
                        case '<':
                            rockNewPos = conveyorPush(rockNewPos, LEFT, false);
                            // conveyored = true;
                            break;
                        case '>':
                            rockNewPos = conveyorPush(rockNewPos, RIGHT, false);
                            // conveyored = true;
                            break;
                        default:
                            if (isEnemy(getItem(rockNewPos))){
                                killTile(rockNewPos, getItem(rockNewPos), false);
                            } else {
                                return false;
                            }
                    }
                    // setFlag(FLAGS_ROCK_MOVED, true);
                    setItem('R', rockNewPos); // this is a mess but a beautiful one
                    if (getItemInLayer(newPos, false) == '_'){
                        releasePlate(newPos);
                    }
                    setItemInLayer('.', newPos, true);
                    // if (!getFlag(FLAGS_ROCK_MOVED)){
                    movePacket.movedObjects[movePacket.movedObjectCount++] = (MovedObject){newPos, rockNewPos, 'R'};
                    // if (conveyored){
                    //     movePacket.movedObjects[movePacket.movedObjectCount++] = (MovedObject){newPos, rockNewPos, 'R'};
                    // }
                    break;
            }
            switch (getItemInLayer(newPos, false)){
                case '^':
                    newPos = conveyorPush(newPos, UP, true);
                    break;
                case 'V':
                    newPos = conveyorPush(newPos, DOWN, true);
                    break;
                case '<':
                    newPos = conveyorPush(newPos, LEFT, true);
                    break;
                case '>':
                    newPos = conveyorPush(newPos, RIGHT, true);
                    break;
            }
        }
    }
    Vector2i finalMovement = vectorAdd(newPos, vectorInvert(playerPos));
    playerPos = newPos;
    if (getFlag(FLAGS_GLASSES)){
        glassesFactor = (Vector2){
            finalMovement.x*20 + glassesFactor.x,
            finalMovement.y*20 + glassesFactor.y
        };
    }
    setOnTop(map[playerPos.y][playerPos.x]);
    if (!undo){
        moveHistory[moveHistoryIndex].button = key;
        // setFlag(FLAGS_GHOST_MODE, false);
    }
    return true;
}

void findGuidePoints(Vector2i sel){
    int guideBufSize = 0;
    switch (getItemInLayer(sel, false)){
        case 'D':
            DoorInfo door = getDoorInfo(sel);
            for (int i=0; i<plateInfoCount; i++){
                if (plateInfo[i].id == door.id){
                    guideBuf[guideBufSize++] = plateInfo[i].pos;
                }
            }
            break;
        case '_':
            PlateInfo plate = getPlateInfo(sel);
            for (int i=0; i<doorInfoCount; i++){
                if (doorInfo[i].id == plate.id){
                    guideBuf[guideBufSize++] = doorInfo[i].pos;
                }
            }
            break;
        case 'T':
            TeleporterInfo teleporter = getTeleporterInfo(sel);
            guideBuf[guideBufSize++] = teleporter.dest;
            break;
        case 'x':
            for (int i=0; i<teleporterInfoCount; i++){
                if (vectorCompare(teleporterInfo[i].dest, sel)){
                    guideBuf[guideBufSize++] = teleporterInfo[i].pos;
                }
            }
            break;
    }
    guideBuf[guideBufSize].x = -1;
}

bool checkKey(int key){
    bool out = IsKeyPressed(key) || (IsKeyDown(key) && holdTimer <= 0);
    if (out){
        if (key == KEY_Z){
            if (consecutiveUndos < 8){
                holdTimer = 0.2;
            } else if (consecutiveUndos < 40){
                holdTimer = 0.05;
            } else {
                holdTimer = -1;
            }
            consecutiveUndos++;
        } else {
            holdTimer = 0.2;
            consecutiveUndos = 0;
        }
    }
    return out;
}

bool placeNode(){
    if (playerPos.x == NodePositions.a.x && playerPos.y == NodePositions.a.y){
        NodePositions.a = (Vector2i){0, 0};
        setItemInLayer('.', playerPos, true);
        if (getOnTop() == '_') releasePlate(playerPos);
    } else if (playerPos.x == NodePositions.b.x && playerPos.y == NodePositions.b.y){
        NodePositions.b = (Vector2i){0, 0};
        setItemInLayer('.', playerPos, true);
        if (getOnTop() == '_') releasePlate(playerPos);
    } else if (NodePositions.a.x == 0){
        NodePositions.a = playerPos;
        if (getOnTop() == '_') pressPlate(playerPos);
        setOnTop('A');
    } else if (NodePositions.b.x == 0){
        NodePositions.b = playerPos;
        if (getOnTop() == '_') pressPlate(playerPos);
        setOnTop('B');
    } else {
        return false;
    }
    return true;
}

void openChest(){
    undoCount++;
    ChestLoot loot = getChestLoot(playerPos);
    if (loot.loot != '\0'){
        int i;
        if (loot.loot == 'G'){
            
            // setFlag(FLAGS_GLASSES, true);
            setFlagDeferred(FLAGS_GLASSES, true);
        } else {
            for (i=0; i<5; i++){
                if (movePacket.items[i] == ' '){
                    // movePacket.items[i] = loot.loot; // trolled
                    break;
                }
            }
            if (i == 5){
                infoBox("Your inventory is full!");
                return;
            } else {
                // movePacket.items[0] = loot.loot;
                addedItem = loot.loot;
            }
        }
        infoBox(loot.text);
        setOnTop('c');
    }
}

void undoChest(){
    undoCount--;
    setOnTop('C');
}

bool isNPC(char c){
    switch (c){
        case 'k':
        case 's':
            return true;
        default:
            return false;
    }
}

bool interact(){ // returns whether the player moved
    // printf("%d\n", getOnTop());
    if (isNPC(getOnTop())){
        startDialogue(playerPos);
        return false;
    }
    switch (getOnTop()){
        case 'C':
            openChest();
            return true;
        case 'A':
        case 'B':
        case '.':
        case '_':
            return placeNode();
        default:
            return false;
    }
}

void undoInteract(){
    // printf("%c\n", getOnTop());
    switch (getOnTop()){
        case 'c':
            undoChest();
            break;
        case 'A':
        case 'B':
        case '_':
            placeNode();
            break;
        case '.':
            placeNode();
            break;
    }
}

bool isItemUsable(int index){
    //todo
    return movePacket.items[index-1] != ' ';
}

void useItem(char item){
    switch (item){
        case 'g':
            setFlag(FLAGS_GHOST_MODE, true);
            break;
        case 'm':
            if (NodePositions.a.x > 0 && NodePositions.b.x > 0){
                // movePacket.pos = playerPos;
                moveHistory[moveHistoryIndex-1].pos = playerPos;
                if (cycle){
                    playerPos = NodePositions.a;
                } else {
                    playerPos = NodePositions.b;
                }
                cycle = !cycle;
                // setFlag(FLAGS_WARP, true);
                moveHistory[moveHistoryIndex-1].flags |= 1 << FLAGS_WARP;
            } else if (NodePositions.a.x > 0){
                moveHistory[moveHistoryIndex-1].pos = playerPos;
                playerPos = NodePositions.a;
                moveHistory[moveHistoryIndex-1].flags |= 1 << FLAGS_WARP;
            } else if (NodePositions.b.x > 0){
                moveHistory[moveHistoryIndex-1].pos = playerPos;
                playerPos = NodePositions.b;
                moveHistory[moveHistoryIndex-1].flags |= 1 << FLAGS_WARP;
            }
            break;
        case 'a':
            moveHistory[moveHistoryIndex-1].flags |= 1 << FLAGS_APPLE;
            stamina += 200;
            break;
        case 's':
            moveHistory[moveHistoryIndex-1].flags |= 1 << FLAGS_SANDWICH;
            stamina += 400;
            break;
    }
}

void moveEnemy(char enemy, Vector2i pos){
    Vector2i newPos = {0, 0};
    switch (enemy){
        case 'e':
            if (vectorCompare(playerPos, pos)){
                // setFlag(FLAGS_DEATH, true); // nah i'd win.
            } else if (playerPos.x == pos.x){
                if (playerPos.y > pos.y){
                    if (isMoveable(pos, DOWN)) newPos = vectorAdd(pos, DOWN);
                } else {
                    if (isMoveable(pos, UP)) newPos = vectorAdd(pos, UP);
                }
            } else if (playerPos.y == pos.y){
                if (playerPos.x > pos.x){
                    if (isMoveable(pos, RIGHT)) newPos = vectorAdd(pos, RIGHT);
                } else {
                    if (isMoveable(pos, LEFT)) newPos = vectorAdd(pos, LEFT);
                }
            } else {
                return;
            }
            if (vectorCompare(newPos, ZERO)) return;
            movePacket.movedObjects[movePacket.movedObjectCount++] = (MovedObject){pos, newPos, 'e'};
            setItemInLayer('.', pos, true);
            if (getItemInLayer(pos, false) == '_') releasePlate(pos);
            if (getItemInLayer(newPos, false) == '_') pressPlate(newPos);
            break;
        case 'R':
            for (int i=0; i<movePacket.movedObjectCount; i++){
                if (vectorCompare(movePacket.movedObjects[i].newPos, pos)){
                    return;
                }
            }
            switch (getItemInLayer(pos, false)){
                case '^':
                    newPos = conveyorPush(pos, UP, false);
                    break;
                case 'V':
                    newPos = conveyorPush(pos, DOWN, false);
                    break;
                case '<':
                    newPos = conveyorPush(pos, LEFT, false);
                    break;
                case '>':
                    newPos = conveyorPush(pos, RIGHT, false);
                    break;
            }
            if (vectorCompare(newPos, ZERO)) return;
            movePacket.movedObjects[movePacket.movedObjectCount++] = (MovedObject){pos, newPos, 'R'};
            setItemInLayer('.', pos, true);
            break;
    }
}

void moveEnemies(){
    for (int i=0; i<visibleMapSize.y; i++){
        for (int j=0; j<visibleMapSize.x; j++){
            Vector2i pos = toMapPosV((Vector2i){j, i});
            char item = isOutOfBounds(pos) ? '.' : getItemInLayer(pos, true);
            if (isEnemy(item) || item == 'R'){
                moveEnemy(item, pos);
            }
        }
    }
    for (int i = 0; i < movePacket.movedObjectCount; i++){
        MovedObject obj = movePacket.movedObjects[i];
        setItemInLayer(obj.tile, obj.newPos, true);
    }
}

bool handleInput(){
    bool moved, out = true;
    int usedItem = 0;

    movePacket.button = 0;
    movePacket.movedObjectCount = 0;
    movePacket.pos = ZERO;
    // movePacket.usedItem.item = 0;
    // printf("%c\n", getOnTop());

    if (getFlag(FLAGS_DEATH)){
        if (checkKey(KEY_Z)){
            goto undo; // yes i'm using goto shut up
        }
        return false;
    }
    if (dialogue){
        if (checkKey(KEY_X)){
            dialogue = proceedDialogue();
        }
        return false;
    }
    if (info){
        if (checkKey(KEY_X)){
            info = false;
        }
        return false;
    }
    if (checkKey(KEY_Z)){
undo:   if (moveHistoryIndex > 0){
            moveHistoryIndex--;
            movePacket = moveHistory[moveHistoryIndex];

            for (int i = 0; i < movePacket.movedObjectCount; i++){
                MovedObject obj = movePacket.movedObjects[i];
                setItemInLayer('.', obj.newPos, isLayer2(obj.tile));
                setItemInLayer(obj.tile, obj.oldPos, isLayer2(obj.tile));
                if (getItemInLayer(obj.oldPos, false) == '_') pressPlate(obj.oldPos);
                if (getItemInLayer(obj.newPos, false) == '_') releasePlate(obj.newPos);
            }

            if (getFlag(FLAGS_WARP)){
                playerPos = movePacket.pos;
            }
            if (getFlag(FLAGS_APPLE)){
                stamina -= 200;
            }
            if (getFlag(FLAGS_SANDWICH)){
                stamina -= 400;
            }
            // why did i do this
            switch (movePacket.button){
                // case KEY_UP:
                //     movePacket.button = KEY_UP;
                //     break;
                // case KEY_DOWN:
                //     movePacket.button = KEY_DOWN;
                //     break;
                // case KEY_LEFT:
                //     movePacket.button = KEY_LEFT;
                //     break;
                // case KEY_RIGHT:
                //     movePacket.button = KEY_RIGHT;
                //     break;
                case KEY_X:
                    undoInteract();
                    break;
            }
            if (movePacket.button != 0) moveTo(movePacket.button, true);
            movePacket.button = 0;

            stamina++;
            undoCount++;
        }
        moved = false;
    } else if (checkKey(KEY_UP)){
        movePacket.button = KEY_UP;
    } else if (checkKey(KEY_DOWN)){
        movePacket.button = KEY_DOWN;
    } else if (checkKey(KEY_LEFT)){
        movePacket.button = KEY_LEFT;
    } else if (checkKey(KEY_RIGHT)){
        movePacket.button = KEY_RIGHT;
    } else if (checkKey(KEY_X)){
        moved = interact();
        if (moved) movePacket.button = KEY_X;
    } else {
        moved = false;
        out = false;
        for (int i = 1; i <= 5; i++){
            if (isItemUsable(i) && checkKey(KEY_ZERO + i)){
                usedItem = i;
                moved = true;
                out = true;
                break;
            }
        }
    }
    if (movePacket.button >= 262 && movePacket.button <= 265){
        moved = moveTo(movePacket.button, false);
    }
    if (moved){
        bool isDead = false;
        if (!getFlag(FLAGS_DEATH)){
            if (isEnemy(getOnTop())){
                isDead = true;
            }
            moveEnemies();
            if (isEnemy(getOnTop())){
                isDead = true;
            }
        }
        if (getFlag(FLAGS_WARP)){
            setFlag(FLAGS_WARP, false);
        }
        if (warpFlag){
            warpFlag = false;
            setFlag(FLAGS_WARP, true);
        }
        moveHistory[moveHistoryIndex] = movePacket;
        moveHistoryIndex++;
        stamina--;
        if (getFlag(FLAGS_APPLE)){
            setFlag(FLAGS_APPLE, false);
        }
        if (getFlag(FLAGS_SANDWICH)){
            setFlag(FLAGS_SANDWICH, false);
        }
        if (deferredFlags > 0){
            movePacket.flags ^= deferredFlags;
            deferredFlags = 0;
        }
        if (addedItem != '\0'){
            for (int i=0; i<5; i++){
                if (movePacket.items[i] == ' '){
                    movePacket.items[i] = addedItem;
                    break;
                }
            }
            addedItem = '\0';
        }
        if (usedItem > 0){
            useItem(movePacket.items[usedItem-1]);
            if  (movePacket.items[usedItem-1] != 'm')
                movePacket.items[usedItem-1] = ' ';
        }
        if (isDead){
            setFlag(FLAGS_DEATH, true);
        }
        if (usedItem == 0){
            setFlag(FLAGS_GHOST_MODE, false);
        }
    }

    return out;
}

void killTile(Vector2i pos, char tile, bool prev){
    if (vectorCompare(pos, playerPos)){
        setFlag(FLAGS_DEATH, true);
        return;
    }
    MovedObject obj = {pos, ZERO, tile};
    switch (tile){
        case '#':
        case '.':
        case '_':
        case 'R':
        case 'D':
        case 'd':
        case '^':
        case 'V':
        case '<':
        case '>':
        case 'T':
        case 'x':
        case 'C':
            return;
        case 'k':
            setOnTop('.');
            moveHistory[moveHistoryIndex-1].movedObjects[moveHistory[moveHistoryIndex-1].movedObjectCount] = obj;
            (moveHistory[moveHistoryIndex-1].movedObjectCount)++;
            break;
        case 'e':
        case 'E':
            setItemInLayer('.', pos, true);
            // map[pos.y][pos.x] = '.';
            
            if (getItemInLayer(pos, false) == '_') releasePlate(pos);

            if (prev){
                bool flag = false;
                for (int i = 0; i < moveHistory[moveHistoryIndex-1].movedObjectCount; i++){
                    if (vectorCompare(moveHistory[moveHistoryIndex-1].movedObjects[i].newPos, pos)){
                        obj.oldPos = moveHistory[moveHistoryIndex-1].movedObjects[i].oldPos;
                        moveHistory[moveHistoryIndex-1].movedObjects[i] = obj;
                        // printf("replaced\n");
                        flag = true;
                        break;
                    }
                }
                if (!flag){
                    moveHistory[moveHistoryIndex-1].movedObjects[moveHistory[moveHistoryIndex-1].movedObjectCount] = obj;
                    (moveHistory[moveHistoryIndex-1].movedObjectCount)++;
                }
            } else {
                movePacket.movedObjects[movePacket.movedObjectCount] = obj;
                (movePacket.movedObjectCount)++;
            }
            break;
    }
    if (isLayer2(tile)){
        layer2[pos.y][pos.x] = '.';
    } else {
        map[pos.y][pos.x] = '.';
    }
    visibleMap[(visibleMapSize.x + 1) * (pos.y - playerPos.y + center.y) + (pos.x - playerPos.x + center.x)] = '.';
}

void updateGame(){
    layer2[0][0] = '.';
    for (int i=0; i<visibleMapSize.y; i++){
        for (int j=0; j<visibleMapSize.x; j++){
            int pos = (visibleMapSize.x + 1) * i + j;
            char item, item2;
            Vector2i truePos = toMapPosV((Vector2i){j, i});

            item = isOutOfBounds(truePos) ? isEdge(truePos) ? '#' : '.' : map[truePos.y][truePos.x];
            item2 = isOutOfBounds(truePos) ? '.' : layer2[truePos.y][truePos.x];

            if (item2 == '.'){
                if (item == 'D'){
                    if (getDoorInfo(truePos).cur >= getDoorInfo(truePos).min){
                        item = 'd';
                    }
                }
            } else {
                item = item2;
            }

            visibleMap[pos] = item;
            colorItem(pos, item);

            if (NodePositions.a.x > 0 && NodePositions.b.x > 0){
                if (CheckCollisionPointLine(getScreenPosNoShift(truePos), getScreenPosNoShift(NodePositions.a), getScreenPosNoShift(NodePositions.b), 11)){
                    if (vectorCompare(truePos, playerPos) && (getOnTop() == 'A' || getOnTop() == 'B')) continue;
                    if (item == 'A' || item == 'B') continue;
                    addColorToVisibleMap(pos, 1, 0, 0, 1);
                    killTile(truePos, item, true);
                }
            }
        }
    }
    visibleMap[(visibleMapSize.x + 1) * center.y + center.x] = '@';
    if (!getFlag(FLAGS_DEATH)){
        if (getFlag(FLAGS_GHOST_MODE)){
            colorItem((visibleMapSize.x + 1) * center.y + center.x, 'g');
        } else if (getItem(playerPos) == 'R'){
            colorItem((visibleMapSize.x + 1) * center.y + center.x, 'R');
        } else {
            colorItem((visibleMapSize.x + 1) * center.y + center.x, '@');
        }
    }
    SetShaderValueV(shader, ShaderParams.modulate, visibleMapColors, SHADER_UNIFORM_VEC4, (visibleMapSize.x+1) * visibleMapSize.y);

    // printf("%d\n", moveHistory[moveHistoryIndex-1].movedObjectCount);
    // for (int i=0; i<moveHistoryIndex; i++){
    //     printf("%d, ", moveHistory[i].button);
    // }
    // for (int i=0; i<moveHistoryIndex; i++){
    //     printf("%d, ", moveHistory[i].movedObjectCount);
    // }
    // printf("\n");
    // for (int i=MAX(moveHistoryIndex-5, 0); i<moveHistoryIndex; i++){
    //     for (int j=0; j<moveHistory[i].movedObjectCount; j++){
    //         printf("%d,%d -> %d,%d\n", moveHistory[i].movedObjects[j].oldPos.x, moveHistory[i].movedObjects[j].oldPos.y, moveHistory[i].movedObjects[j].newPos.x, moveHistory[i].movedObjects[j].newPos.y);
    //     }
    // }
    // printf("\b \n");
}

static void UpdateDrawFrame(){

    inGameClockTimer = GetTime() > 99*60+59 ? 99*60+59 : GetTime();

    holdTimer -= GetFrameTime();
    
    shiftingColor = shiftingColors[shiftingIndex++];
    if (shiftingIndex > 35) shiftingIndex = 0; 
    SetShaderValue(shader, ShaderParams.shifting, (float[]){shiftingColor.r, shiftingColor.g, shiftingColor.b, 255}, SHADER_UNIFORM_VEC4);

    if (getFlag(FLAGS_GLASSES)){
        glassesFactor = (Vector2){glassesFactor.x * 0.85, glassesFactor.y * 0.85};
    } else {
        glassesFactor = (Vector2){0, 0};
    }

    if (IsKeyUp(KEY_Z)) consecutiveUndos = 0;
    if (handleInput()) updateGame();

    guideBuf[0].x = -1;
    Vector2i sel;
    if (getFlag(FLAGS_GLASSES)){
        mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, (Rectangle){10, 10, 620, 420})){
            sel = toMapPosV((Vector2i){
                (int)(mousePos.x + 30 - glassesFactor.x) / 20,
                (int)(mousePos.y + 30 - glassesFactor.y) / 20
            });
            findGuidePoints(sel);
        }
    }

    SetShaderValue(shader, ShaderParams.offset, (int[]){
        10 - 40 + glassesFactor.x, // goofy ahh magic numbers
        11.5 - 40 + glassesFactor.y // tbh idk why 11.5 works better than 12
    }, SHADER_UNIFORM_IVEC2);

    BeginDrawing();
    
        ClearBackground(BLACK);

        #ifdef ENABLE_ON_SCREEN_COORDS
        DrawRectangle(0, 0, 100, 20, BLACK);
        sprintf(temp, "%d", playerPos.x);
        DrawTextEx(font, temp, (Vector2){0, 435}, 10, 10, WHITE);
        sprintf(temp, "%d", playerPos.y);
        DrawTextEx(font, temp, (Vector2){0, 445}, 10, 10, WHITE);
        #endif

        DrawRectangleLinesEx((Rectangle){20, 20, 600, 400}, 2.0, WHITE);

        DrawTextEx(font, "items", (Vector2){35, 433}, 16, 9.5, CYAN);
        DrawTextEx(font, "undos", (Vector2){195, 433}, 16, 9.5, CYAN);
        DrawTextEx(font, "stmna", (Vector2){355, 433}, 16, 9.5, CYAN);
        DrawTextEx(font, "timer", (Vector2){515, 433}, 16, 9.5, CYAN);

        sprintf(temp, "%s", movePacket.items);
        DrawTextEx(font, temp, (Vector2){35, 453}, 16, 9.5, WHITE);

        sprintf(temp, "%05d", undoCount);
        DrawTextEx(font, undoCount < 99999 ? temp : "99999", (Vector2){195, 453}, 16, 9.5, WHITE);

        sprintf(temp, "%05d", stamina);
        DrawTextEx(font, temp, (Vector2){355, 453}, 16, 9.5, WHITE);

        sprintf(temp, "%02d:%02d", inGameClockTimer/60, inGameClockTimer%60);
        DrawTextEx(font, temp, (Vector2){515, 453}, 16, 9.5, WHITE);
        
        // anything under here will only be rendered inside the white rectangle
        BeginScissorMode(30, 30, 580, 380);

            if (guideBuf[0].x != -1){
                int guidePtr = 0;
                Vector2 selScr = getScreenPos(sel);
                DrawRectangleLinesEx((Rectangle){selScr.x-10, selScr.y-10, 20, 20}, 1.0, ORANGE);
                while (guideBuf[guidePtr].x != -1){
                    Vector2 endPos = getScreenPos(guideBuf[guidePtr]);
                    DrawLineEx(selScr, endPos, 1, ORANGE);
                    guidePtr++;
                }
            }

            BeginShaderMode(shader);
                DrawTextEx(font, visibleMap, (Vector2){
                    15 + glassesFactor.x - 40,
                    13 + glassesFactor.y - 40
                }, 16, 9.5, WHITE);
            EndShaderMode();

            if (NodePositions.a.x > 0 && NodePositions.b.x > 0){
                DrawLineEx(getScreenPos(NodePositions.a), getScreenPos(NodePositions.b), 2, shiftingColor);
            } else if (NodePositions.a.x > 0){
                DrawLineEx(getScreenPos(NodePositions.a), getScreenPos(playerPos), 2, TPGRAY);
            } else if (NodePositions.b.x > 0){
                DrawLineEx(getScreenPos(NodePositions.b), getScreenPos(playerPos), 2, TPGRAY);
            }
        EndScissorMode();

        if (dialogue){
            DrawRectangle(50, 30, 540, 100, BLACK);
            DrawRectangleLinesEx((Rectangle){60, 40, 520, 80}, 2.0, CYAN);

            BeginScissorMode(65, 45, 510, 70);
                DrawTextEx(font, dialogueText[0], (Vector2){75, 53}, 16, 9.5, WHITE);
                DrawTextEx(font, dialogueText[1], (Vector2){75, 73}, 16, 9.5, WHITE);
                DrawTextEx(font, dialogueText[2], (Vector2){75, 93}, 16, 9.5, WHITE);
            EndScissorMode();
        }

        if (info){
            DrawRectangle(90, 30, 500, 60, BLACK);
            DrawRectangleLinesEx((Rectangle){100, 40, 480, 40}, 2.0, CYAN);

            BeginScissorMode(105, 45, 470, 30);
                DrawTextEx(font, infoBoxText, (Vector2){115, 53}, 16, 9.5, WHITE);
            EndScissorMode();
        }

        if (getFlag(FLAGS_DEATH)){
            DrawRectangle(270, 230, 100, 20, BLACK);
            DrawTextEx(font, "DEATH", (Vector2){275, 233}, 16, 9.5, WHITE);
        }

    EndDrawing();
    // exit(0);
}

Vector2i multiSpawns[100];
int multiSpawnCount = 0;
void generateMap(){
    FILE *file = fopen("data/map.txt", "r");
    char line[mapSize.x + 5];
    for (int i=0; i<mapSize.y; i++){
        fgets(line, mapSize.x + 5, file); //???????????????????? voodoo shit
        for (int j=0; j<mapSize.x; j++){
            if (line[j] == '|'){
                layer2[i][j] = 'e';
                map[i][j] = '_';
                multiSpawns[multiSpawnCount++] = (Vector2i){j, i};
            }
            else if (isLayer2(line[j])){
                layer2[i][j] = line[j];
                map[i][j] = '.';
            } else {
                layer2[i][j] = '.';
                map[i][j] = line[j];
            }
        }
    }
    fclose(file);
}

#ifdef ENABLE_MAP_VALIDATION
int count;
void validateData(){
    int playerCount = 0;
    int errorCount = 0;
    printf("\n----- checking for errors -----\n");
    if (mapSize.x < 1 || mapSize.y < 1){
        printf("[fatal] map size too small\n");
        exit(1);
    }
    for (int i=0; i<mapSize.y; i++){
        for (int j=0; j<mapSize.x; j++){
            switch (map[i][j]){
                case '@':
                    playerCount++;
                    if (playerCount > 1){
                        printf("[fatal] too many players\n");
                        exit(1);
                    }
                    playerPos = (Vector2i){j, i};
                    map[i][j] = '.';
                    break;
                case 'C':
                    if (!chestLootExists((Vector2i){j, i})){
                        printf("[error] missing chest loot for %d,%d\n", j, i);
                        errorCount++;
                    }
                    break;
                case 'D':
                    if (!doorInfoExists((Vector2i){j, i})){
                        printf("[error] missing door info for %d,%d\n", j, i);
                        errorCount++;
                    }
                    break;
                case '_':
                    if (!plateInfoExists((Vector2i){j, i})){
                        printf("[error] missing plate info for %d,%d\n", j, i);
                        errorCount++;
                    }
                    break;
                case 'T':
                    if (!teleporterInfoExists((Vector2i){j, i})){
                        printf("[error] missing teleporter for %d,%d\n", j, i);
                        errorCount++;
                    }
                    break;
            }
            switch (layer2[i][j]){
                case 'A':
                    NodePositions.a = (Vector2i){j, i};
                    break;
                case 'B':
                    NodePositions.b = (Vector2i){j, i};
                    break;
            }
        }
    }

    if (playerCount < 1){
        printf("[fatal] no player found\n");
        exit(1);
    }
    printf("%d error%s found\n\n", errorCount, errorCount == 1 ? "" : "s");
    printf("start anyway? (y/n) ");
    char c = getchar();
    if (c != 'y' && c != 10){
        printf("exiting...\n");
        exit(0);
    }
}
#endif

int main(){
    MALLOC2(map, char, mapSize.y, mapSize.x);
    MALLOC2(layer2, char, mapSize.y, mapSize.x);
    visibleMap = malloc(visibleMapSize.y * (visibleMapSize.x + 1) * sizeof(char));
    visibleMapColors = malloc(4 * visibleMapSize.y * (visibleMapSize.x + 1) * sizeof(float));
    moveHistory = malloc(1000 * sizeof(MovePacket));

    dialogueFile = fopen("data/dialogue.txt", "r");
    MALLOC2(dialogueText, char, 3, 25);

    infoBoxText = malloc(20 * sizeof(char));

    // printf("\nworking directory: %s\n\n", GetWorkingDirectory());

    generateMap();

    readChestLoot();
    readDoors();
    readPlates();
    readTeleporters();

    for (int i=0; i<multiSpawnCount; i++){
        pressPlate(multiSpawns[i]);
    }

    NodePositions.a = (Vector2i){0, 0};
    NodePositions.b = (Vector2i){0, 0};

    #ifdef ENABLE_MAP_VALIDATION
        validateData();
    #else
        for (int i=0; i<mapSize.y; i++){
            for (int j=0; j<mapSize.x; j++){
                switch (map[i][j]){
                    case '@':
                        playerPos = (Vector2i){j, i};
                        map[i][j] = '.';
                        break;
                }
                switch (layer2[i][j]){
                    case 'A':
                        NodePositions.a = (Vector2i){j, i};
                        break;
                    case 'B':
                        NodePositions.b = (Vector2i){j, i};
                        break;
                }
            }
        }
    #endif

    for (int i=visibleMapSize.x; i<visibleMapSize.y * (visibleMapSize.x + 1); i+=visibleMapSize.x + 1){
        visibleMap[i] = '\n';
        addColorToVisibleMap(i, 1,1,1,1);
    }
    strcpy(movePacket.items, "     ");
    moveHistory[0] = movePacket;

    printf("----- initializing window -----\n");

    InitWindow(windowSize.x, windowSize.y, "game");

    font = LoadFont("resources/Monocraft.otf");
    shader = LoadShader("shaders/color.vs", "shaders/color.fs");
    shiftingColors = LoadImageColors(LoadImage("resources/shifting.png"));
    ShaderParams.offset = GetShaderLocation(shader, "offset");
    ShaderParams.modulate = GetShaderLocation(shader, "modulate");
    ShaderParams.shifting = GetShaderLocation(shader, "shifting");
    SetTextLineSpacing(20);

    SetTargetFPS(60);

    updateGame();
    while (!WindowShouldClose()){
        UpdateDrawFrame();
    }

    UnloadFont(font);

    FREE2(map, mapSize.y);
    FREE2(layer2, mapSize.y);
    free(visibleMap);
    free(visibleMapColors);
    free(shiftingColors);
    free(moveHistory);

    CloseWindow();

    return 0;
}