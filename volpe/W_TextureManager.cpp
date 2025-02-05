//-----------------------------------------------------------------------------
// File:			W_TextureManager.cpp
// Original Author:	Gordon Wood
//
// See header for notes
//-----------------------------------------------------------------------------
#include "W_TextureManager.h"
#include "iostream"
using namespace std;

namespace volpe
{

std::map<std::string, TextureManager::Entry*>	TextureManager::m_textures;

//----------------------------------------------------------
// Creates a new texture or returns an existing copy if already
// loaded previously
//----------------------------------------------------------
Texture* TextureManager::CreateTexture(const std::string& path)
{
	std::map<std::string, Entry*>::iterator iter = m_textures.find(path);

	if( iter != m_textures.end() )
	{
		iter->second->m_refCount++;
		return iter->second->m_pTex;
	}

	Texture* pTex = new Texture(path);
	Entry* pEntry = new Entry(pTex);
	m_textures[path] = pEntry;
	return pTex;
}

//----------------------------------------------------------
// When creating directly from data, we don't check for duplicates
// and just delegate directly to the texture class
//----------------------------------------------------------
Texture* TextureManager::CreateTexture(void* pData, unsigned int width, unsigned int height, Texture::Format fmt)
{
	return new Texture(pData, width, height, fmt);
}
Texture* TextureManager::CreateArrayTexture(void* pData, unsigned int width, unsigned int height,unsigned int layers, Texture::Format fmt){
	return new Texture(pData, width, height, layers, fmt);
}
Texture* TextureManager::CreateAutoArrayTexture(const std::vector<std::string>& filePaths){
	
	if(filePaths.empty()) {
        printf("Error: No file paths provided for array texture creation.\n");
        return nullptr;
    }

    int texWidth, texHeight, texChannels;
    std::vector<unsigned char*> imageDataList;
    texWidth = texHeight = texChannels = 0;

    for(const auto& path : filePaths) {
        unsigned char* data = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, 4);
        if(!data) {
            printf("Error: Failed to load texture '%s'\n", path.c_str());
            for(auto& imgData : imageDataList) {
                stbi_image_free(imgData);
            }
            return nullptr;
        }
        imageDataList.push_back(data);
    }

    size_t singleLayerSize = texWidth * texHeight * 4;
    size_t totalSize = singleLayerSize * imageDataList.size();
    unsigned char* arrayData = new unsigned char[totalSize];

    for(size_t i = 0; i < imageDataList.size(); ++i) {
        memcpy(arrayData + i * singleLayerSize, imageDataList[i], singleLayerSize);
    }

    volpe::Texture* arrayTexture = volpe::TextureManager::CreateArrayTexture(
        arrayData, texWidth, texHeight, (unsigned int)imageDataList.size(), volpe::Texture::FMT_8888
    );

    if(!arrayTexture) {
        printf("Error: Failed to create array texture.\n");
        delete[] arrayData;
        for(auto& imgData : imageDataList) {
            stbi_image_free(imgData);
        }
        return nullptr;
    }

    arrayTexture->SetWrapMode(volpe::Texture::WM_Repeat, volpe::Texture::WM_Repeat);
    arrayTexture->SetFilterMode(volpe::Texture::FM_TrilinearMipmap, volpe::Texture::FM_Linear);

    delete[] arrayData;
    for(auto& imgData : imageDataList) {
        stbi_image_free(imgData);
    }
    return arrayTexture;
}
Texture* TextureManager::CreateRenderTexture(unsigned int width, unsigned int height, Texture::Format fmt) 
{
	Texture* pRT = new Texture(width, height, fmt);
	pRT->SetFilterMode(volpe::Texture::FM_Nearest);
	pRT->SetWrapMode(volpe::Texture::WM_Clamp);
	return pRT;
}

//----------------------------------------------------------
// Destroys a texture. Only actually deletes it if the refcount
// is down to 0.
//----------------------------------------------------------
void TextureManager::DestroyTexture(Texture* pTex)
{
    cout<<"TYRING TO DELETE: "<<pTex<<endl;
	std::map<std::string, Entry*>::iterator iter;
	for( iter = m_textures.begin(); iter != m_textures.end(); iter++ )
	{
		if( iter->second->m_pTex == pTex )
		{
			iter->second->m_refCount--;
			if( iter->second->m_refCount == 0 )
			{
				delete iter->second->m_pTex;
				m_textures.erase(iter);
			}
			return;
		}
	}

	// If we got here, it can only be via a from-data texture which wasn't in
	// the list (unless the pointer being passed in is dodgy in which case we're
	// in trouble anyway).
	delete pTex;
}

}


