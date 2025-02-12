#pragma once
#include <GL/glew.h>

struct CharInfo {
    float uStart, uEnd, vStart, vEnd;
    float xOffset, yOffset;          
    float xAdvance;                   
    int page;
    
    CharInfo() : uStart(0), uEnd(0), vStart(0), vEnd(0), xOffset(0), yOffset(0), xAdvance(0), page(0) {}
    
};