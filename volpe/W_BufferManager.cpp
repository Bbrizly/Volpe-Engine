//-----------------------------------------------------------------------------
// File:			W_BufferManager.cpp
// Original Author:	Gordon Wood
//
// See header for notes
//-----------------------------------------------------------------------------
#include "W_BufferManager.h"

namespace volpe
{

//----------------------------------------------------------
// Creates a new Vertex Buffer
//----------------------------------------------------------
VertexBuffer* BufferManager::CreateVertexBuffer(unsigned int length)
{
	return new VertexBuffer(length);
}

//----------------------------------------------------------
// Creates a new Vertex Buffer
//----------------------------------------------------------
VertexBuffer* BufferManager::CreateVertexBuffer(const void* pData, unsigned int length)
{
	return new VertexBuffer(pData,length);
}

//----------------------------------------------------------
// Creates a new Index Buffer
//----------------------------------------------------------
IndexBuffer* BufferManager::CreateIndexBuffer(unsigned int numIndices)
{
	return new IndexBuffer(numIndices);
}

//----------------------------------------------------------
// Creates a new Index Buffer
//----------------------------------------------------------
IndexBuffer* BufferManager::CreateIndexBuffer(const unsigned short* pData, unsigned int numIndices)
{
	IndexBuffer* pRet = new IndexBuffer(numIndices);
	pRet->Write(pData);
	return pRet;
}

//----------------------------------------------------------
// Updates a vertex buffer with new data
//----------------------------------------------------------
void BufferManager::UpdateVertexBuffer(VertexBuffer* pBuffer, const void* pData, unsigned int length, unsigned int offset)
{
    if (pBuffer)
    {
        pBuffer->Update(pData, offset, length);
    }
}

//----------------------------------------------------------
// Destroys a buffer. 
//----------------------------------------------------------
void BufferManager::DestroyBuffer(Buffer* pBuf)
{
	if(!pBuf)
		return;
	delete pBuf;
}

}

