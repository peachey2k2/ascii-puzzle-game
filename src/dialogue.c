#include "dialogue.h"

extern bool dialogue;
extern char **dialogueText;
extern FILE *dialogueFile;

extern bool info;
extern char *infoBoxText;

bool findDialogue(Vector2i pos){
    char line[100], id[30];
    sprintf(id, "%d %d", pos.x, pos.y);
    strcat(id, ":");
    // return;
    rewind(dialogueFile);
    while (fgets(line, 100, dialogueFile) != NULL){
        strcpy(line, strtok(line, "\n"));
        if (strcmp(line, id) == 0){
            // dialogueText = line;
            return true;
        }
    }
    return false;
}

void startDialogue(Vector2i pos){
    // strcpy(id, ID);
    bool result = findDialogue(pos);
    if (!result){
        printf("Dialogue not found\n");
        return;
    }
    dialogue = true;
    proceedDialogue();
}

bool proceedDialogue(){
    for (int i = 0; i < 3; i++){
        fgets(dialogueText[i], 25, dialogueFile);
    }
    if (dialogueText[0][0] == '/'){
        // printf("End of dialogue\n");
        dialogue = false;
        return false;
    }
    return true;
}

void infoBox(char *text){
    info = true;
    strcpy(infoBoxText, text);
}