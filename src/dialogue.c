#include "dialogue.h"

extern bool dialogue;
extern char **dialogueText;
extern FILE *dialogueFile;

extern bool info;
extern char *infoBoxText;

bool findDialogue(Vector2i pos){
    char line[30], id[30];
    sprintf(id, "%d %d:", pos.x, pos.y);
    rewind(dialogueFile);
    while (fgets(line, 30, dialogueFile) != NULL){
        // strcpy(line, strtok(line, "\n")); // unsafe and broken
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, id) == 0){
            return true;
        }
    }
    return false;
}

void startDialogue(Vector2i pos){
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
        fgets(dialogueText[i], 27, dialogueFile);
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