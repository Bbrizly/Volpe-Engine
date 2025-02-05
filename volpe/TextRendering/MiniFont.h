#pragma once
// #include "../volpe/Volpe.h"
#include <string>
#include <unordered_map>
#include <utility>
#include "CharInfo.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace std;

// Custom hash function for pair<char, char>
struct PairHash {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& pair) const {
        return hash<T1>()(pair.first) ^ (hash<T2>()(pair.second) << 1);
    }
};


class MiniFont {
private:
    void LoadFont(const string& fontPath);
    void ArrayTextureOfAllFiles(string filename);

    volpe::Texture* m_arrayTexture;
    unordered_map<char, CharInfo> m_characters;
    unordered_map<pair<char, char>, int, PairHash> m_kerning;
    
    int m_lineHeight = 0; // Distance between lines
    int m_scaleW;     // Texture width
    int m_scaleH;     // Texture height
    int m_pages;
    int m_pageOffset;

public:
    MiniFont(const string& fontPath, int pageOffset);
    MiniFont();

    ~MiniFont();

    const CharInfo& GetCharacter(char c) const;
    const int GetLineHeight();
    const int GetPages();
    volpe::Texture* GetTexture() const;

    const unordered_map<pair<char, char>, int, PairHash>& GetKerning() const;

};