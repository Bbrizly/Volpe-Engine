//-----------------------------------------------------------------------------
// File:			W_MaterialManager.h
// Original Author:	Gordon Wood
//
// Class to manage materials. Handles creation of them and dealing with duplicate
// requests for the same material. 
//
// TODO: Left as an exercise is to load and populate materials from an XML
// definition of them (see accompanying slides).
//-----------------------------------------------------------------------------
#ifndef W_MATERIAL_MANAGER_H
#define W_MATERIAL_MANAGER_H

#include "W_Types.h"
#include "W_Material.h"
#include <string>
#include <map>

namespace volpe
{
class MaterialManager
{
	public:
		//-------------------------------------------------------------------------
		// PUBLIC INTERFACE
		//-------------------------------------------------------------------------
		static Material* CreateMaterial(const std::string& name);
		static void DestroyMaterial(Material* pMat);

		// TODO: You should really have a method like "Cleanup" that will delete
		// any leftover materials that weren't destroyed by the game, as
		// a safeguard - or at least prints a warning.
		//-------------------------------------------------------------------------

	private:
		//-------------------------------------------------------------------------
		// PRIVATE TYPES
		//-------------------------------------------------------------------------
		struct Entry
		{
			Material*	m_pMat;
			int			m_iRefCount;
			Entry(Material* pMat) : m_pMat(pMat), m_iRefCount(1) {}
		};
		//-------------------------------------------------------------------------

		//-------------------------------------------------------------------------
		// PRIVATE MEMBERS
		//-------------------------------------------------------------------------
		static std::map<std::string, Entry*>	m_materials;
		//-------------------------------------------------------------------------
};

}

#endif



