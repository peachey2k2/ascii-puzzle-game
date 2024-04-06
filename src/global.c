#include "global.h"

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

#define CYAN (Color){0, 255, 255, 255}

const Vector2i UP = {0, -1};
const Vector2i DOWN = {0, 1};
const Vector2i LEFT = {-1, 0};
const Vector2i RIGHT = {1, 0};
const Vector2i ZERO = {0, 0};

Vector2 toVector2(Vector2i v){
    return (Vector2){(float)v.x, (float)v.y};
}

Vector2i vectorAdd(Vector2i a, Vector2i b){
    return (Vector2i){a.x + b.x, a.y + b.y};
}

Vector2i vectorInvert(Vector2i a){
    return (Vector2i){-a.x, -a.y};
}

bool vectorCompare(Vector2i a, Vector2i b){
    return a.x == b.x && a.y == b.y;
}