#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "global.h"

// extern char *dialogueText;

void startDialogue(Vector2i);
bool proceedDialogue();
void infoBox(char*);

#endif