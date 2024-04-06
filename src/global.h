#ifndef GLOBAL_H
#define GLOBAL_H

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define MALLOC2(_pointer, _type, _size1, _size2)                    \
    (_pointer) = (_type**)malloc(_size1 * sizeof(_type*));          \
    for (int _i = 0; _i < _size1; _i++){                            \
        (_pointer)[_i] = (_type*)malloc(_size2 * sizeof(_type));    \
    }

#define FREE2(_pointer, _size)              \
    for (int _i = 0; _i < _size; _i++){     \
        free(_pointer[_i]);                 \
    }                                       \
    free(_pointer);

#define COLOR(r, g, b, a) (Color)(r/255, g/255, b/255, a/255)
#define MIN(a, b) a < b ? a : b
#define MAX(a, b) a > b ? a : b

#define CYAN (Color){0, 255, 255, 255}
#define TPGRAY (Color){100, 100, 100, 200}

typedef struct Vector2i{
    int x, y;
} Vector2i;

extern const Vector2i UP;
extern const Vector2i DOWN;
extern const Vector2i LEFT;
extern const Vector2i RIGHT;
extern const Vector2i ZERO;

Vector2 toVector2(Vector2i);
Vector2i vectorAdd(Vector2i, Vector2i);
Vector2i vectorInvert(Vector2i);
bool vectorCompare(Vector2i, Vector2i);

#endif