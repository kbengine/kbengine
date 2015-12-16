//-----------------------------------------------------------------------------
// TmxTile.h
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
#ifndef __TMX_TILE_H__
#define __TMX_TILE_H__

#include "TmxPropertySet.h"

namespace Tmx 
{
	//-------------------------------------------------------------------------
	// Class to contain information about every tile in the tileset/tiles 
	// element.
	// It may expand if there are more elements or attributes added into the
	// the tile element.
	// This class also contains a property set.
	//-------------------------------------------------------------------------
	class Tile 
	{
	public:
        Tile(int id);
		Tile();
		~Tile();
	
		// Parse a tile node.
		void Parse(const TiXmlNode *tileNode);
		
		// Get the Id. (relative to the tilset)
		int GetId() const { return id; }

		// Get a set of properties regarding the tile.
		const Tmx::PropertySet &GetProperties() const { return properties; }

	private:
		int id;

		Tmx::PropertySet properties;
	};
};

#endif

