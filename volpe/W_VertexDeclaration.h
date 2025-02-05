//-----------------------------------------------------------------------------
// File:			W_VertexDeclaration.h
// Original Author:	Gordon Wood
//
// Class representing a complete vertex declaration, including all the
// attributes that comprise the vertex, and the associated source buffers
//-----------------------------------------------------------------------------
#ifndef W_VERTEXDECLARATION_H
#define W_VERTEXDECLARATION_H

#include <vector>
#include "W_Types.h"

namespace volpe
{
class VertexBuffer;
class IndexBuffer;

class VertexDeclaration
{
	public:
		//-------------------------------------------------------------------------
		// PUBLIC INTERFACE
		//-------------------------------------------------------------------------
		VertexDeclaration();
		~VertexDeclaration();
	
		void Begin();
		void SetVertexBuffer(volpe::VertexBuffer* p_pVB);
		void SetIndexBuffer(volpe::IndexBuffer* p_pIB);
		void AppendAttribute(volpe::Attribute p_attr, int p_iNumComponents, volpe::ComponentType p_type, int p_iOffset = -1);
		void End(int stride = -1 /* override if you don't want it to calculate itself */);

		void Bind();
		//-------------------------------------------------------------------------

	private:
		//-------------------------------------------------------------------------
		// PRIVATE TYPES
		//-------------------------------------------------------------------------
		struct AttributeInfo
		{
			volpe::Attribute		m_attr;
			volpe::ComponentType	m_type;
			int					m_iOffset;
			int					m_iNumComponents;
		};
		//-------------------------------------------------------------------------

		//-------------------------------------------------------------------------
		// PRIVATE MEMBERS
		//-------------------------------------------------------------------------
		std::vector<AttributeInfo>	m_attrs;
		volpe::VertexBuffer*			m_pVB;
		volpe::IndexBuffer*			m_pIB;
		GLuint						m_vao;
		//-------------------------------------------------------------------------
};

}

#endif

