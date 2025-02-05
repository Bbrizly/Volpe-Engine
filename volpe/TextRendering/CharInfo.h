#pragma once
#include <GL/glew.h>

struct CharInfo {
    float uStart, uEnd, vStart, vEnd;
    float xOffset, yOffset;          
    float xAdvance;                   
    int page;
};