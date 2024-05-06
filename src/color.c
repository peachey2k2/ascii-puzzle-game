#include "color.h"

extern float *visibleMapColors;

void addColorToVisibleMap(int pos, float r, float g, float b, float a){
    visibleMapColors[pos*4] = r;
    visibleMapColors[pos*4+1] = g;
    visibleMapColors[pos*4+2] = b;
    visibleMapColors[pos*4+3] = a;
}

void colorItem(int pos, char item){
    switch (item){
        case '#':
            addColorToVisibleMap(pos, 0.8, 0.8, 0.8, 1);
            break;
        case '.':
            addColorToVisibleMap(pos, 0.4, 0.4, 0.4, 1);
            break;
        case '@':
            addColorToVisibleMap(pos, 0.2, 0.4, 1, 1);
            break;
        case 'A':
        case 'B':
        case 'a':
        case 'b':
            addColorToVisibleMap(pos, -1, -1, -1, 1);
            break;
        case 'C':
            addColorToVisibleMap(pos, 0.8, 0.7, 0.2, 1);
            break;
        case 'c':
            addColorToVisibleMap(pos, 0.4, 0.3, 0.1, 1);
            break;
        case 'k':
            addColorToVisibleMap(pos, 0.2, 0.9, 0.2, 1);
            break;
        case '_':
            addColorToVisibleMap(pos, 0.2, 0.7, 0.9, 1);
            break;
        case 'R':
        case 'r':
            addColorToVisibleMap(pos, 0.9, 0.3, 0.7, 1);
            break;
        case 'e':
            addColorToVisibleMap(pos, 0.9, 0.3, 0.4, 1);
            break;
        case '<':
        case '>':
        case '^':
        case 'V':
            addColorToVisibleMap(pos, 0.7, 0.9, 0.7, 1);
            break;
        default:
            addColorToVisibleMap(pos, 0.5, 0.5, 0.5, 1);
            break;
    }
}