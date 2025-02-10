#include "Font.h"

using namespace std;

Font::Font(const string& fontPath) 
{
    LoadAll(fontPath);
}

Font::~Font() {
    cout<<"DESTRUCTOR"<<endl;
    if (m_arrayTexture)
        volpe::TextureManager::DestroyTexture(m_arrayTexture);
    m_arrayTexture = nullptr;

    for (MiniFont* font : m_fonts) {
        delete font;
    }
    m_fonts.clear();
}


void Font::LoadAll(const string& fileName)
{
    cout << "[Font::LoadAll] Base .fnt: " << fileName << endl;

    string baseName = "data/Fonts/" + fileName;

    vector<string> variationBaseNames;
    int pageOffset = 0;
    int count = 0;

    int maxFonts = 30;

    while(count < maxFonts) {
        stringstream ss;
        ss << baseName << count << ".fnt";
        string dataFile = ss.str();

        ifstream file(dataFile);
        if(!file.is_open()) {
            break;
        }
        file.close();

        MiniFont* mini = new MiniFont(dataFile, pageOffset);
        if(!mini) break;

        pageOffset += mini->GetPages();

        m_fonts.push_back(mini);

        stringstream ss2;
        ss2 << baseName << count;
        variationBaseNames.push_back(ss2.str());

        count++;
    }

    // now we create one big array texture for ALLLLL pages
    // ArrayTextureOfAllFiles(variationBaseNames);

    cout << "[Font::LoadAll] # of mini-fonts loaded: " << m_fonts.size() << endl;
}

void Font::ArrayTextureOfAllFiles(const vector<string>& listOfFontSizes)
{
    cout << "[Font::ArrayTextureOfAllFiles] building array texture..." << endl;
    vector<string> files;

    // for each mini-font variation i
    for (int i=0; i < (int)listOfFontSizes.size(); i++){
        int pages = m_fonts[i]->GetPages();
        for(int j=0; j < pages; j++){
            // e.g. "data/test0_0.tga"
            string textureFile = listOfFontSizes[i] + "_" + to_string(j) + ".tga";
            cout << "  Looking for: " << textureFile << endl;
            files.push_back(textureFile);
        }
    }

    cout << "Loading " << files.size() << " textures." << endl;

    m_arrayTexture = volpe::TextureManager::CreateAutoArrayTexture(files);
    
}

const CharInfo& Font::GetCharacter(char c, int index) const{
    
    return m_fonts[index]->GetCharacter(c);
}
const int Font::GetLineHeight(int index){
    return m_fonts[index]->GetLineHeight();
}
const unordered_map<pair<char, char>, int, PairHash>& Font::GetKerning(int index) const{
    return m_fonts[index]->GetKerning();
}

const vector<MiniFont*>& Font::GetFonts() const {
    return m_fonts;
}

volpe::Texture* Font::GetTexture() const {
    return m_arrayTexture;
}

