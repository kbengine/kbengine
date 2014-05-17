//-----------------------------------------------------------------------------
// TmxObjectGroup.h
//
// Copyright (c) 2010-2014, Tamir Atias
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL TAMIR ATIAS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: Tamir Atias
//-----------------------------------------------------------------------------
#ifndef __TMX_OBJECTGROUP_H__
#define __TMX_OBJECTGROUP_H__

#include <string>
#include <vector>

#include "TmxPropertySet.h"

class TiXmlNode;

namespace Tmx 
{
	class Object;
	
	//-------------------------------------------------------------------------
	// A class used for holding a list of objects.
	// This class doesn't have a property set.
	//-------------------------------------------------------------------------
	class ObjectGroup 
	{
	public:
		ObjectGroup();
		~ObjectGroup();

		// Parse an objectgroup node.
		void Parse(const TiXmlNode *objectGroupNode);

		// Get the name of the object group.
		const std::string &GetName() const { return name; }

		// Get the width of the object group, in pixels.
		// Note: do not rely on this due to temporary bug in tiled.
		int GetWidth() const { return width; }

		// Get the height of the object group, in pixels.
		// Note: do not rely on this due to temporary bug in tiled.
		int GetHeight() const { return height; }

		// Get a single object.
		const Tmx::Object *GetObject(int index) const { return objects.at(index); }

		// Get the number of objects in the list.
		int GetNumObjects() const { return objects.size(); }

		// Get whether the object layer is visible.
		int GetVisibility() const { return visible; }

		// Get the property set.
		const Tmx::PropertySet &GetProperties() const { return properties; }

		// Get the whole list of objects.
		const std::vector< Tmx::Object* > &GetObjects() const { return objects; }

		// Get the zorder of the object group.
		int GetZOrder() const { return zOrder; }
		
		// Set the zorder of the object group.
		void SetZOrder( int z ) { zOrder = z; }

	private:
		std::string name;
		
		int width;
		int height;
		int visible;
		int zOrder;

		Tmx::PropertySet properties;

		std::vector< Tmx::Object* > objects;
	};
};

#endif

